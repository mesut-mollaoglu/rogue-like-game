#pragma once

#include "Engine/Primitives.h"
#include "Enemy.h"
#include <random>
#include <iterator>

template<typename T, typename RandomGenerator>
inline T SelectRandomly(T start, T end, RandomGenerator& g) {
	std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
	std::advance(start, dis(g));
	return start;
}

template<typename T>
inline T SelectRandomly(T start, T end) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	return SelectRandomly(start, end, gen);
}

template<class T, class U> bool IsSameType(U obj) {
	return dynamic_cast<T>(obj) != nullptr;
}

struct Powerup {
	float timeLasting;
	std::function<void()> mEffect;
	std::function<void()> mEnd;
	Sprite mIcon;
	Powerup() = default;
	Powerup(std::string sFileName, std::function<void()> fEffect, std::function<void()> fEnd, float fTime) {
		mIcon = Sprite();
		mIcon.SetPosition(Vec2f());
		mIcon.SetTexture(Graphics::LoadTexture(sFileName), .75f);
		mEffect = fEffect;
		mEnd = fEnd;
		timeLasting = fTime;
	}
	void Free() {
		std::destroy_at(std::addressof(mEffect));
		std::destroy_at(std::addressof(mEnd));
		mIcon.Free();
	}
};

struct Chest {
	std::function<void()> mFunction;
	std::vector<Powerup> vPowerups;
	Powerup currentPowerup;
	Sprite mSprite;
	Animator mAnimator;
	bool bChestOpened, bNoPowerup;
	Text text;
	Chest() {
		bChestOpened = false;
		bNoPowerup = true;
		mAnimator = Animator(Graphics::LoadFromDir("Assets\\Chest\\frames"), 75, true);
		mSprite = Sprite();
		mSprite.SetTexture(mAnimator.GetByIndex(0).frame);
		mSprite.SetPosition({ 3800, 2100 });
		text = Text("Press 'E' to open.", { 0, 0 }, 0.02f);
		text.SetPosition(mSprite.position + Vec2f(0, 800) + ToScreenCoord({ 512 - text.GetStringSize(), 368 }));
	}
	void Update(bool bCanOpen, Vec2f characterPosition, float ticks = 150.f) {
		if (!bNoPowerup && currentPowerup.mEffect && currentPowerup.mEnd) {
			if (currentPowerup.timeLasting > 0)
				currentPowerup.mEffect();
			if (currentPowerup.timeLasting <= 0) {
				currentPowerup.mEnd();
				bNoPowerup = true;
				CloseChest();
			}
			currentPowerup.timeLasting -= ticks;
		}
		if (bCanOpen && Abs(characterPosition.GetDistance(mSprite.position)) < 1500.f) {
			if (isKeyPressed('E') && !bChestOpened) OpenChest();
		}
	}
	void OpenChest() {
		if (mFunction) mFunction();
		bChestOpened = true;
		bNoPowerup = false;
		while (mAnimator.GetIndex() < mAnimator.GetSize() - 1)
			mAnimator.UpdateFrames();
		currentPowerup = *SelectRandomly(vPowerups.begin(), vPowerups.end());
		assert(currentPowerup.mEffect && currentPowerup.mEnd);
		currentPowerup.mIcon.SetPosition(mSprite.position);
	}
	void CloseChest() {
		bChestOpened = false;
		while (mAnimator.GetIndex() > 0)
			mAnimator.UpdateFrames(true);
	}
	void Render(Vec2f characterPosition) {
		mSprite.SetTexture(mAnimator.GetCurrentFrame());
		mSprite.Draw();
		if (Abs(characterPosition.GetDistance(mSprite.position)) < 1500.f) {
			if (!bChestOpened)
				text.DrawString();
		}
		if (bChestOpened) {
			if (currentPowerup.mIcon.position.y < mSprite.position.y + 765)
				currentPowerup.mIcon.SetPosition(currentPowerup.mIcon.position + Vec2f(0, 45));
			currentPowerup.mIcon.Draw();
		}
	}
	void Free() {
		std::destroy_at(std::addressof(mFunction));
		text.Free();
		for (auto& element : vPowerups)
			element.Free();
		mAnimator.Free();
		mSprite.Free();
		if (bNoPowerup || !currentPowerup.mEffect) return;
		currentPowerup.Free();
	}
};

struct WaveSystem {
	std::vector<std::unique_ptr<Entity>> enemies;
	bool bFinished;
	enum class WaveStates {
		Cooldown = 0,
		Spawning = 1,
		Waiting = 2
	} waveStates = WaveStates::Cooldown;
	Text text;
	float waveCooldown, spawnRate;
	int nMaxNumber, nWaveNumber;
	int nCurrentSpawnNumber;
	std::vector<Entity::FactoryFunction> factories;
	WaveSystem() = default;
	WaveSystem(int nMax) {
		factories = { &RangedEnemy::Create, &Enemy::Create };
		bFinished = false;
		nMaxNumber = nMax;
		spawnRate = 20000;
		waveCooldown = 40000;
		nWaveNumber = 0;
		srand(time(0));
		text = Text("Wave " + std::to_string(nWaveNumber), { 0, 0 }, 0.03f);
		text.SetPosition(ToScreenCoord({ 512 - text.GetStringSize(), 6 }));
	}
	void SetWave(int waveNumber, WaveStates state, int nSpawned) {
		text.SetText("Wave " + std::to_string(waveNumber));
		text.SetPosition(ToScreenCoord({ 512 - text.GetStringSize(), 6 }));
		nWaveNumber = waveNumber;
		waveStates = state;
		nCurrentSpawnNumber = nSpawned;
	}
	void Update(float nDeltaTime = 150.f) {
		assert(waveNumber <= maxNumber);
		switch (waveStates) {
		case WaveStates::Cooldown: {
			waveCooldown -= nDeltaTime;
			if (nMaxNumber > nWaveNumber && waveCooldown <= 0) {
				waveCooldown = 40000;
				SetWave(nWaveNumber+1, WaveStates::Spawning, 0);
			}
			else if (nMaxNumber == nWaveNumber && waveCooldown <= 0)
				bFinished = true;
		}
								 break;
		case WaveStates::Spawning: {
			enemies.emplace_back(GetRandomEntity());
			if (IsSameType<RangedEnemy*>(enemies.back().get())) enemies.back()->SetState("Spawn");
			enemies.back()->SetPosition(SelectSpawnPosition(enemies.back().get()));
			nCurrentSpawnNumber++;
			waveStates = WaveStates::Waiting;
		}
								 break;
		case WaveStates::Waiting: {
			spawnRate -= nDeltaTime;
			if (spawnRate <= 0) {
				spawnRate = 20000;
				if (nWaveNumber + 1 >= nCurrentSpawnNumber) waveStates = WaveStates::Spawning;
				else if (enemies.empty()) waveStates = WaveStates::Cooldown;
			}
		}
								break;
		}
	}
	std::unique_ptr<Entity> GetRandomEntity() {
		Entity::FactoryFunction entity = *SelectRandomly(factories.begin(), factories.end());
		return entity();
	}
	template <class T> Vec2f SelectSpawnPosition(T entity) {
		float x = 0, y = 0;
		if (IsSameType<Enemy*>(entity)) {
			x = (rand() % 4) & 1 ? -6000 : 6000;
			y = rand() % 4200 - 2100;
		}
		else if (IsSameType<RangedEnemy*>(entity)) {
			x = rand() % 9000 - 4500;
			y = rand() % 4000 - 2000;
		}
		return { x, y };
	}
	void Render() {
		switch (waveStates) {
		case WaveStates::Spawning: case WaveStates::Waiting: {
			if (text.color != Structures::Color(1.f, 1.f, 1.f, 1.f))
				text.SetColor(Structures::Color(1.f, 1.f, 1.f, 1.f));
		}
								 break;
		case WaveStates::Cooldown: {
			if (text.color != Structures::Color(.5f, .5f, .5f, 1.f))
				text.SetColor(Structures::Color(.5f, .5f, .5f, 1.f));
		}
								 break;
		}
		text.DrawString();
	}
	void Free() {
		factories.clear();
		text.Free();
		if (!enemies.empty())
			for (int i = 0; i < enemies.size(); i++) {
				enemies[i]->Destroy();
				enemies[i].reset();
			}
		enemies.clear();
	}
};

struct Coins {
	int nAmount;
	Text mText;
	Coins() {
		mText = Text("$000", ToScreenCoord({ 875, 6 }), 0.03f);
		SetAmount(0);
	}
	void SetAmount(int amount) {
		if (amount < 0) nAmount = 0;
		else if (amount > 999) nAmount = 999;
		else nAmount = amount;
		std::string sAmount = std::to_string(nAmount);
		std::string sText;
		int size = 3 - sAmount.size();
		for (int i = 0; i < size; i++) sText += '0';
		sText += sAmount;
		mText.SetText("$" + sText);
	}
	void Render() {
		mText.DrawString();
	}
	void Free() {
		mText.Free();
	}
};

struct Item {
	std::vector<float> vValues;
	mutable int nValueIndex;
	Sprite mIcon;
	Text sDescription;
	Text sLevel;
	Text sPrice;
	std::string desc;
	Item() = default;
	Item(std::string nLocation, std::string nDescription, std::vector<float> vList) {
		nValueIndex = 0;
		vValues = vList;
		sLevel = Text("Level 1", { 0, 0 }, 0.03f);
		sLevel.SetPosition(ToScreenCoord({ 512 - sLevel.GetStringSize(), 475 }));
		mIcon = Sprite();
		mIcon.SetTexture(Graphics::LoadTexture(nLocation));
		mIcon.SetPosition(ToScreenCoord({ 512, 250 }));
		desc = nDescription;
		sDescription = Text(nDescription, { 0, 0 }, 0.03f);
		sDescription.SetPosition(ToScreenCoord({ 512 - sDescription.GetStringSize(), 400 }));
		sPrice = Text("$2", { 0, 0 }, 0.03f);
		sPrice.SetPosition(ToScreenCoord({ 512 - sDescription.GetStringSize(), 550 }));
	}
	void SetIndex(int index) {
		assert(nValueIndex != index);
		if (index >= vValues.size()) return;
		nValueIndex = index;
	}
	void SetPrice(int value) {
		std::string price = std::to_string(value);
		sPrice.SetText("$" + price);
		sPrice.SetPosition(ToScreenCoord({ 512 - sPrice.GetStringSize(), 550 }));
	}
	int FindValue(float val) {
		for (int i = 0; i < vValues.size(); i++)
			if (vValues[i] == val)
				return i;
		return -1;
	}
	void SetLevel(int index) {
		SetIndex(index);
		SetPrice(2 << index);
		sLevel.SetText("Level " + std::to_string(nValueIndex + 1));
		sLevel.SetPosition(ToScreenCoord({ 512 - sLevel.GetStringSize(), 475 }));
	}
	void Free() {
		vValues.clear();
		mIcon.Free();
		sDescription.Free();
		sLevel.Free();
		sPrice.Free();
	}
	float GetCurrentValue() {
		return vValues[nValueIndex];
	}
};