#pragma once
#include "Sprite.h"
#include <chrono>

class Animator {
public:
	typedef std::chrono::time_point<std::chrono::steady_clock> timePoint;
	typedef struct Frame {
		ID3D11ShaderResourceView* frame;
		float duration;
		Frame() = default;
		Frame(ID3D11ShaderResourceView* image, float dur) {
			frame = image;
			duration = dur;
		};
		~Frame() {}
	} Frame;
	Animator(std::vector<ID3D11ShaderResourceView*> images, float duration, bool play = false) {
		[&, this](std::vector<ID3D11ShaderResourceView*> vec) {
			for (int i = 0; i < vec.size(); i++) {
				Frame frame = Frame(vec[i], duration);
				frames.push_back(frame);
			}
		}(images);
		SetCurrentFrame(0);
		notPlayed = true;
		this->playOnce = play;
		start = std::chrono::high_resolution_clock::now();
	}
	Animator(Animator& animator) {
		this->frames = animator.frames;
		this->playOnce = animator.playOnce;
		this->notPlayed = animator.notPlayed;
	}
	ID3D11ShaderResourceView* UpdateFrames() {
		if (!bInit)
		{
			start = now = Clock::now();
			bInit = true;
		}
		now = Clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
		if (duration.count() > currentFrame.duration) {
			start = Clock::now();
			this->index++;
			if (notPlayed && index == frames.size()) notPlayed = false;
			index = index % frames.size();
			SetCurrentFrame(this->index);
		}
		return GetCurrentFrame();
	}
	ID3D11ShaderResourceView* GetCurrentFrame() {
		return this->currentFrame.frame; 
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
		for (size_t i = 0; i < frames.size(); i++) {
			if (i == ind % this->GetSize()) {
				return frames[i];
			}
		}
	}
	void SetCurrentFrame(int ind) {
		Frame frame = GetByIndex(ind);
		this->currentFrame = frame;
	} 
	int GetSize() {
		return this->frames.size();
	}
	Frame GetCurrentFrameRaw() {
		return this->currentFrame;
	}
	int GetIndex() const {
		return this->index;
	}
	void SetIndex(int ind) noexcept {
		this->index = ind;
	}
	friend std::ostream& operator<<(std::ostream& out, Animator& anim) {
		out << (int)anim.GetSize() << std::endl;
		return out;
	}
protected:
	Frame currentFrame;
	int index = 0;
	bool notPlayed, playOnce;
	using Clock = std::chrono::steady_clock;
	timePoint start;
	timePoint now;
	std::chrono::milliseconds duration;
	bool bInit = false;
	std::vector<Frame> frames;
};