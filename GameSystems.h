#pragma once

#include "Engine/Primitives.h"
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

//TODO: Fix it.

struct Powerup {
	float timeLasting;
	std::function<void()> mEffect;
	Sprite mIcon;
	Powerup() = default;
	Powerup(std::string sFileName, std::function<void()> fEffect, float fTime) {
		mIcon = Sprite();
		mIcon.SetPosition(Vec2f());
		mIcon.SetTexture(Graphics::LoadTexture(sFileName), .75f);
		mEffect = fEffect;
		timeLasting = fTime;
	}
};

struct Chest {
	std::vector<Powerup> vPowerups;
	Powerup currentPowerup;
	Sprite mSprite;
	Animator* mAnimator;
	bool chestOpened;
	Text text;
	Chest() {
		mAnimator = new Animator(Graphics::LoadFromDir("chestFrames"), 300, true);
		mSprite = Sprite();
		mSprite.SetPosition({ 4000, 2100 });
		text = Text("Press 'E' to open.", { 0, 0 }, 0.02f);
		text.SetPosition(mSprite.position + Vec2f(0, 1200) - ToScreenCoord({text.GetStringSize(), 368}));
	}
	void Update(Vec2f characterPosition, float ticks = 150.f) {
		if (Abs(characterPosition.GetDistance(mSprite.position)) < 1500.f) {
			if (isKeyPressed('E')) OpenChest();
		}
		else
			CloseChest();
	}
	void OpenChest() {
		chestOpened = true;
		while (mAnimator->GetIndex() < mAnimator->GetSize()-1)
			mAnimator->UpdateFrames();
		currentPowerup = *SelectRandomly(vPowerups.begin(), vPowerups.end());
		currentPowerup.mIcon.SetPosition(mSprite.position);
	}
	void CloseChest() {
		chestOpened = false;
		while (mAnimator->GetIndex() > 0)
			mAnimator->UpdateFrames(true);
	}
	void Render(Vec2f characterPosition) {
		mSprite.SetTexture(mAnimator->GetCurrentFrame());
		mSprite.Draw();
		if (Abs(characterPosition.GetDistance(mSprite.position)) < 1500.f) {
			if (!chestOpened)
				text.DrawString();
			else{
				if (currentPowerup.mIcon.position.y < mSprite.position.y + 900)
					currentPowerup.mIcon.SetPosition(currentPowerup.mIcon.position + Vec2f(0, 45));
				currentPowerup.mIcon.Draw();
			}
		}
	}
	void Free() {
		mAnimator->Free();
		mSprite.Free();
		delete mAnimator;
	}
};