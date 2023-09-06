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
		if(mIcon.mTexture.texture != nullptr) mIcon.Free();
	}
};

struct Chest {
	std::vector<Powerup> vPowerups;
	Powerup currentPowerup;
	Sprite mSprite;
	Animator* mAnimator;
	bool bChestOpened, bNoPowerup;
	Text text;
	Chest() {
		bChestOpened = false;
		bNoPowerup = true;
		mAnimator = new Animator(Graphics::LoadFromDir("chestFrames"), 75, true);
		mSprite = Sprite();
		mSprite.SetTexture(mAnimator->GetByIndex(0).frame);
		mSprite.SetPosition({ 4000, 2100 });
		text = Text("Press 'E' to open.", { 0, 0 }, 0.02f);
		text.SetPosition(mSprite.position + Vec2f(0, 1000) + ToScreenCoord({512 - text.GetStringSize(), 368}));
	}
	void Update(Vec2f characterPosition, float ticks = 150.f) {
		if(!bNoPowerup && currentPowerup.mEffect && currentPowerup.mEnd) {
			if (currentPowerup.timeLasting > 0)
				currentPowerup.mEffect();
			if (currentPowerup.timeLasting <= 0) {
				currentPowerup.mEnd();
				bNoPowerup = true;
				CloseChest();
			}
			currentPowerup.timeLasting -= ticks;
		}
		if (Abs(characterPosition.GetDistance(mSprite.position)) < 1500.f) {
			if (isKeyPressed('E') && !bChestOpened) OpenChest();
		}
	}
	void OpenChest() {
		bChestOpened = true;
		bNoPowerup = false;
		while (mAnimator->GetIndex() < mAnimator->GetSize() - 1)
			mAnimator->UpdateFrames();
		currentPowerup = *SelectRandomly(vPowerups.begin(), vPowerups.end());
		assert(currentPowerup.mEffect && currentPowerup.mEnd);
		currentPowerup.mIcon.SetPosition(mSprite.position);
	}
	void CloseChest() {
		bChestOpened = false;
		while (mAnimator->GetIndex() > 0)
			mAnimator->UpdateFrames(true);
	}
	void Render(Vec2f characterPosition) {
		mSprite.SetTexture(mAnimator->GetCurrentFrame());
		mSprite.Draw();
		if (Abs(characterPosition.GetDistance(mSprite.position)) < 1500.f) {
			if (!bChestOpened)
				text.DrawString();
		}
		if (bChestOpened) {
			if (currentPowerup.mIcon.position.y < mSprite.position.y + 900)
				currentPowerup.mIcon.SetPosition(currentPowerup.mIcon.position + Vec2f(0, 45));
			currentPowerup.mIcon.Draw();
		}
	}
	void Free() {
		text.Free();
		for (auto& element : vPowerups)
			element.Free();
		mAnimator->Free();
		mSprite.Free();
		delete mAnimator;
		if (bNoPowerup || !currentPowerup.mEffect) return;
		currentPowerup.Free();
	}
};

struct WaveSystem {
	std::vector<std::unique_ptr<Enemy>> enemies;
	enum class WaveStates {
		Cooldown,
		Spawning,
		Waiting
	} waveStates = WaveStates::Cooldown;
	Text text;
	float waveCooldown, spawnRate;
	int nMaxNumber, nWaveNumber;
	int nCurrentSpawnNumber;
	WaveSystem() = default;
	WaveSystem(int nMax) {
		nMaxNumber = nMax;
		spawnRate = 30000;
		waveCooldown = 60000;
		nWaveNumber = 0;
		srand(time(0));
		text = Text("Wave " + std::to_string(nWaveNumber), { 0, 0 }, 0.03f);
		text.SetPosition(ToScreenCoord({430, 15})); 
	}
	void Update(Vec2f characterPosition, float nDeltaTime = 150.f) {
		assert(waveNumber <= maxNumber);
		switch (waveStates) {
		case WaveStates::Cooldown:{
			if (text.color != Structures::Color(.5f, .5f, .5f, 1.f))
				text.SetColor(Structures::Color(.5f, .5f, .5f, 1.f));
			waveCooldown -= nDeltaTime;
			if (nMaxNumber > nWaveNumber && waveCooldown <= 0) {
				nWaveNumber++;
				nCurrentSpawnNumber = 0;
				text.SetText("Wave " + std::to_string(nWaveNumber));
				waveCooldown = 60000;
				waveStates = WaveStates::Spawning;
			}
		}
								break;
		case WaveStates::Spawning: {
			if (text.color != Structures::Color(1.f, 1.f, 1.f, 1.f))
				text.SetColor(Structures::Color(1.f, 1.f, 1.f, 1.f));
			enemies.push_back(std::make_unique<Enemy>("eAttack", "eMove", "eIdle", "eDead"));
			enemies.back()->SetPosition(SelectSpawnPosition(characterPosition));
			nCurrentSpawnNumber++;
			waveStates = WaveStates::Waiting;
		}
								 break;
		case WaveStates::Waiting: {
			if (text.color != Structures::Color(1.f, 1.f, 1.f, 1.f))
				text.SetColor(Structures::Color(1.f, 1.f, 1.f, 1.f));
			spawnRate -= nDeltaTime;
			if (spawnRate <= 0) {
				spawnRate = 30000;
				if (nWaveNumber >= nCurrentSpawnNumber) waveStates = WaveStates::Spawning;
				else 
					if(enemies.empty())
						waveStates = WaveStates::Cooldown;
			}
		}
								break;
		}
	}
	Vec2f SelectSpawnPosition(Vec2f characterPosition) {
		float x = (rand() % 4) & 1 ? -6000 : 6000;
		float y = rand() % 4200 - 2100;
		return { x, y };
	}
	void Render() {
		text.DrawString();
	}
};

struct Coins {
	int nAmount;
	Text mText;
	Coins() {
		mText = Text("$000", ToScreenCoord({ 885, 15}), 0.03f);
		SetAmount(0);
	}
	void SetAmount(int amount) {
		assert(amount != nAmount && amount < 1000);
		nAmount = amount;
		std::string sAmount = std::to_string(amount);
		std::string sText;
		int size = 3 - sAmount.size();
		for (int i = 0; i < size; i++) sText += '0';
		sText += sAmount;
		mText.SetText("$" + sText);
	}
	void Render() {
		mText.DrawString();
	}
};

struct Item {
	std::vector<float> vValues;
	mutable int nValueIndex;
	Sprite mIcon;
	Text sDescription;
	Text sLevel;
	std::string desc;
	timePoint tp;
	Item() = default;
	Item(std::string nLocation, std::string nDescription, std::vector<float> vList) {
		nValueIndex = 0;
		vValues = vList;
		sLevel = Text("Level 1", { 0, 0 }, 0.03f);
		sLevel.SetPosition(ToScreenCoord({512 - sLevel.GetStringSize(), 475}));
		mIcon = Sprite();
		mIcon.SetTexture(Graphics::LoadTexture(nLocation));
		mIcon.SetPosition(ToScreenCoord({ 512, 250 }));
		desc = nDescription;
		sDescription = Text(nDescription, {0, 0}, 0.03f);
		sDescription.SetPosition(ToScreenCoord({ 512 - sDescription.GetStringSize(), 400 }));
	}
	void SetIndex(int index) {
		assert(nValueIndex != index);
		if (index >= vValues.size()) return;
		nValueIndex = index;
	}
	int FindValue(float val) {
		for (int i = 0; i < vValues.size(); i++)
			if (vValues[i] == val)
				return i;
		return -1;
	}
	void SetLevel(int index) {
		if (GetTimeLapse(Clock::now(), tp) < .5f)
			return;
		SetIndex(index);
		sLevel.SetText("Level " + std::to_string(nValueIndex+1));
		sLevel.SetPosition(ToScreenCoord({512-sLevel.GetStringSize(), 475}));
		tp = Clock::now();
	}
	void Free() {
		vValues.clear();
		mIcon.Free();
		sDescription.Free();
		sLevel.Free();
	}
	float GetCurrentValue() {
		return vValues[nValueIndex];
	}
};