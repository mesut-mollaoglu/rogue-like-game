#pragma once
#include "Animator.h"
#include <string>
#include <algorithm>
#include <functional>

class BaseStateMachine {
public:
	static enum class MouseWheel {
		WHEEL_UP,
		WHEEL_DOWN,
		WHEEL_UNKNOWN,
	};
	typedef std::chrono::time_point<std::chrono::high_resolution_clock> timePoint;
	static float GetTimeLapse(timePoint tp1, timePoint tp2) {
		return std::chrono::duration<float>(tp1 - tp2).count();
	}
	static MouseWheel GetMouseWheel(MSG msg = Window::windowMessage) {
		if (msg.message == WM_MOUSEWHEEL) {
			short delta = GET_WHEEL_DELTA_WPARAM(msg.wParam);
			if (delta > 0) return MouseWheel::WHEEL_UP;
			else if (delta < 0) return MouseWheel::WHEEL_DOWN;
			else return MouseWheel::WHEEL_UNKNOWN;
		}
	}
	static bool isKeyPressed(char a) {
		return (GetAsyncKeyState((unsigned short)a) & 0x8000);
	}
	template<class... Args>
	static bool isKeyPressed(char a, Args... args) {
		bool isPressed = BaseStateMachine::isKeyPressed(a);
		isPressed = isPressed && isKeyPressed(args...);
		return isPressed;
	}
	static bool isKeyReleased(char a, MSG msg = Window::windowMessage) {
		return (msg.message == WM_KEYUP && a == (char)msg.wParam);
	}
	template<class... Args>
	static bool isKeyReleased(char a, Args... args) {
		bool isReleased = BaseStateMachine::isKeyReleased(a);
		isReleased &= isKeyReleased(args...);
		return isReleased;
	}
	static bool getKeyPressed() {
		for (int i = 0x01; i < 0xFE; i++) {
			if (isKeyPressed(i)) return true;
		}
		return false;
	}
	static Math::Vec2f GetMousePos() {
		POINT* pt = new POINT;
		GetCursorPos(pt);
		ScreenToClient(GetActiveWindow(), pt);
		return Math::Vec2f((float)pt->x, (float)pt->y);
		delete pt;
	}
	static Math::Vec2f ToScreenCoord(Math::Vec2f pos) {
		Math::Vec2f ret;
		ret.x = Camera::Position.z * Window::width * (2.0f * pos.x / Window::width - 1.0f);
		ret.y = -Camera::Position.z * Window::height * (2.0f * pos.y / Window::height - 1.0f);
		return ret;
	}
	typedef struct Key {
		std::vector<char> keys;
		void Add(char c) {
			keys.push_back(c);
		}
		Key(char c) {
			Add(c);
		}
		template<class... Args>
		Key(char c, Args... args) {
			Add(c);
			Add(args...);
		}
		bool isPressed() {
			bool keyPressed = true;
			std::for_each(keys.begin(), keys.end(), [&, this](char a) {keyPressed &= BaseStateMachine::isKeyPressed(a); });
			return keyPressed;
		}
		bool isReleased() {
			bool keyPressed = true;
			std::for_each(keys.begin(), keys.end(), [&, this](char a) {keyPressed &= BaseStateMachine::isKeyReleased(a); });
			return keyPressed;
		}
	}Key;
	typedef struct StateController {
		std::function<void()> mFunction;
		Animator* mAnimator;
		std::string mStateName;
		std::vector<BaseStateMachine::Key> mKeys;
		float cooldown;
		timePoint tp1;
		bool activated = true;
		void Update() {
			if (mAnimator->isPlayed() && mAnimator->shouldPlayOnce()) {
				activated = false;
				tp1 = Clock::now();
			}
			if (cooldown < BaseStateMachine::GetTimeLapse(Clock::now(), tp1)) {
				activated = true;
				tp1 = Clock::now();
			}
		}
		StateController() = default;
		StateController(std::function<void()> update, Animator* animator, std::string name, std::vector<Key> keys, float cool) {
			mFunction = update;
			mAnimator = animator;
			mStateName = name;
			mKeys = keys;
			cooldown = cool;
		};
	} StateController;
	const char* GetState() const {
		return mCurrentState.mStateName.c_str();
	}
	void SetState(std::string state) {
		Animator* animator = new Animator(*mCurrentState.mAnimator);
		if (animator->shouldPlayOnce() && !animator->isPlayed()) {
			delete animator;
			return;
		}
		if (!GetStateByName(state).activated) {
			return;
		}
		mPreviousState = mCurrentState;
		mPreviousState.mAnimator->ResetPlayBool();
		mCurrentState.mStateName = state;
		if (animator != nullptr) delete animator;
	}
	StateController GetCurrentState() {
		std::for_each(states.begin(), states.end(), [&, this](StateController& state) {
			if (equals(state.mStateName)) {
				mCurrentState = state;
			}
			});
		return mCurrentState;
	}
	StateController GetStateByName(std::string name) {
		StateController returnState;
		std::for_each(states.begin(), states.end(), [&, this](StateController& state) {
			if (strcmp(name.c_str(), state.mStateName.c_str()) == 0) returnState = state;
			});
		return returnState;
	}
	ID3D11ShaderResourceView* RenderState() {
		return GetCurrentState().mAnimator->UpdateFrames();
	}
	bool equals(std::string state) const {
		return strcmp(GetState(), state.c_str()) == 0;
	}
	using Clock = std::chrono::high_resolution_clock;
	StateController mCurrentState;
	StateController mPreviousState;
	std::vector<StateController> states;
	void Clear() {
		for (auto state : states) {
			for (auto frame : state.mAnimator->frames) {
				frame.frame->Release();
				frame.~Frame();
			}
			state.mAnimator->frames.clear();
			delete state.mAnimator;
			for (auto key : state.mKeys) {
				key.keys.clear();
			}
			state.mKeys.clear();
		}
		states.clear();
	}
};

class StateMachine : public BaseStateMachine {
public:
	void AddStateComb(std::function<void()> func, Animator* anim, std::string name, std::vector<Key> keys, float cooldown = 0) {
		StateController* controller = new StateController(func, anim, name, keys, cooldown / 1000);
		states.push_back(*controller);
		delete controller;
		mCurrentState = states[0];
	}
	void AddState(std::function<void()> func, Animator* anim, std::string name, std::vector<char> keys, float cooldown = 0) {
		std::vector<Key> container;
		std::for_each(keys.begin(), keys.end(), [&, this](char& a) {container.push_back(a); });
		AddStateComb(func, anim, name, container, cooldown);
	}
	void UpdateState() {
		std::for_each(states.begin(), states.end(), [&, this](StateController& state) {state.Update(); });
		try {
			mCurrentState.mFunction();
		}
		catch (std::bad_function_call& e) {
			std::cout << e.what() << std::endl;
		}
		std::for_each(states.begin(), states.end(), [&, this](StateController& state) {
			if (!state.mKeys.empty()) {
				for (auto& key : state.mKeys) {
					if (!equals(state.mStateName) && key.isPressed()) {
						SetState(state.mStateName);
					}
				}
			}
			else {
				if (!getKeyPressed()) {
					SetState(state.mStateName);
				}
			}
			});
	}
};

class AIStateMachine : public BaseStateMachine {
public:
	void AddState(std::function<void()> func, Animator* anim, std::string name, float cooldown = 0) {
		StateController* controller = new StateController(func, anim, name, {}, cooldown / 1000);
		states.push_back(*controller);
		delete controller;
		mCurrentState = states[0];
	}
	void UpdateState() {
		std::for_each(states.begin(), states.end(), [&, this](StateController& state) {state.Update(); });
		try {
			mCurrentState.mFunction();
		}
		catch (std::bad_function_call& e) {
			std::cout << e.what() << std::endl;
		}
	}
};

class Level {
public:
	uint32_t ticks = 75;
	enum class ManageLevel {
		CurrentLevel,
		PrevLevel,
		NextLevel,
		GotoLevel
	} nLevel = ManageLevel::CurrentLevel;
	std::string sLevelName;
	std::string nLevelName = "";
	virtual void Render() = 0;
	virtual void FixedUpdate() = 0;
	virtual void Update() = 0;
	virtual void Load() = 0;
	virtual void UnLoad() = 0;
};