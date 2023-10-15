#pragma once
#include "Graphics.h"
#include <chrono>

class Animator {
public:
	typedef std::chrono::time_point<std::chrono::steady_clock> timePoint;
	typedef struct Frame {
		Structures::Texture frame;
		float duration;
		Frame() = default;
		Frame(Structures::Texture image, float dur) {
			frame = image;
			duration = dur;
		};
		~Frame() {}
	} Frame;
	Animator() = default;
	Animator(std::vector<Structures::Texture> images, float duration, bool play = false) {
		for (int i = 0; i < images.size(); i++) 
			frames.emplace_back(Frame(images[i], duration));
		index = 0;
		notPlayed = true;
		playOnce = play;
		now = std::chrono::high_resolution_clock::now();
	}
	Structures::Texture UpdateFrames(bool bReverse = false) {
		if (Abs(std::chrono::duration<float>(Clock::now() - now).count()) > frames[index].duration / 1000.f) {
			index += bReverse ? -1 : 1;
			index %= frames.size();
			if (notPlayed && index == frames.size() - 1) notPlayed = false;
			now = Clock::now();
		}
		return frames[index].frame;
	}
	Structures::Texture GetCurrentFrame() {
		return frames[index].frame;
	}
	bool isPlayed() {
		return !notPlayed;
	}
	void ResetPlayBool() {
		notPlayed = true;
	}
	bool shouldPlayOnce() {
		return playOnce;
	}
	Frame GetByIndex(int ind) {
		return frames[ind % frames.size()];
	}
	void SetCurrentFrame(int ind) {
		index = ind % frames.size();
	}
	int GetSize() {
		return frames.size();
	}
	Frame GetCurrentFrameRaw() {
		return frames[index];
	}
	int GetIndex() const {
		return index;
	}
	void SetIndex(int ind) noexcept {
		index = ind;
	}
	void Free() {
		if (!frames.empty())
			for (auto frame : frames) {
				frame.frame.Free();
				frame.~Frame();
			}
		frames.clear();
	}
protected:
	std::vector<Frame> frames;
	int index = 0;
	bool notPlayed, playOnce;
	using Clock = std::chrono::steady_clock;
	timePoint now;
};