#pragma once
#include "Animator.h"
#include <string>
#include <algorithm>
#include <functional>

enum class MouseWheel {
	WHEEL_UP,
	WHEEL_DOWN,
	WHEEL_UNKNOWN,
};

enum class Mouse {
	LeftMouseButton = 0,
	MiddleMouseButton = 1,
	RightMouseButton = 2,
	Unknown = 3
};

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timePoint;
using Clock = std::chrono::high_resolution_clock;

inline float GetTimeLapse(timePoint tp1, timePoint tp2) {
	return std::chrono::duration<float>(tp1 - tp2).count();
}

inline MouseWheel GetMouseWheel(MSG msg = Window::windowMessage) {
	if (msg.message == WM_MOUSEWHEEL) {
		short delta = GET_WHEEL_DELTA_WPARAM(msg.wParam);
		if (delta > 0) return MouseWheel::WHEEL_UP;
		else if (delta < 0) return MouseWheel::WHEEL_DOWN;
		else return MouseWheel::WHEEL_UNKNOWN;
	}
}

inline Mouse GetMouseRelease(MSG msg = Window::windowMessage) {
	if (msg.message == WM_LBUTTONUP) return Mouse::LeftMouseButton;
	if (msg.message == WM_MBUTTONUP) return Mouse::MiddleMouseButton;
	if (msg.message == WM_RBUTTONUP) return Mouse::RightMouseButton;
	return Mouse::Unknown;
}

inline Mouse GetMouseClick(MSG msg = Window::windowMessage) {
	if (msg.message == WM_LBUTTONDOWN) return Mouse::LeftMouseButton;
	if (msg.message == WM_MBUTTONDOWN) return Mouse::MiddleMouseButton;
	if (msg.message == WM_RBUTTONDOWN) return Mouse::RightMouseButton;
	return Mouse::Unknown;
}

inline bool isKeyPressed(char a) {
	return (GetAsyncKeyState((unsigned short)a) & 0x8000);
}
template<class... Args>
inline bool isKeyPressed(char a, Args... args) {
	bool isPressed = isKeyPressed(a);
	isPressed = isPressed && isKeyPressed(args...);
	return isPressed;
}
inline bool isKeyReleased(char a, MSG msg = Window::windowMessage) {
	return (msg.message == WM_KEYUP && a == (char)msg.wParam);
}
template<class... Args>
inline bool isKeyReleased(char a, Args... args) {
	bool isReleased = isKeyReleased(a);
	isReleased &= isKeyReleased(args...);
	return isReleased;
}
inline bool getKeyPressed() {
	for (int i = 0x01; i < 0xFE; i++) {
		if (isKeyPressed(i)) return true;
	}
	return false;
}
inline Vec2f GetMousePos() {
	POINT* pt = new POINT;
	GetCursorPos(pt);
	ScreenToClient(GetActiveWindow(), pt);
	return Vec2f((float)pt->x, (float)pt->y);
	delete pt;
}
inline Vec2f ToScreenCoord(Vec2f pos) {
	Vec2f ret;
	ret.x = Camera::Position.z * Window::width * (2.0f * pos.x / Window::width - 1.0f);
	ret.y = -Camera::Position.z * Window::height * (2.0f * pos.y / Window::height - 1.0f);
	return ret;
}

class BaseStateMachine {
public:
	typedef struct StateController {
		std::function<void()> mFunction;
		Animator* mAnimator;
		std::string mStateName;
		std::vector<char> mKeys;
		float cooldown;
		timePoint tp1;
		bool activated = true;
		void Update() {
			if (mAnimator->isPlayed() && mAnimator->shouldPlayOnce()) {
				activated = false;
				tp1 = Clock::now();
			}
			if (cooldown < GetTimeLapse(Clock::now(), tp1)) {
				activated = true;
				tp1 = Clock::now();
			}
		}
		StateController() = default;
		StateController(std::function<void()> update, Animator* animator, std::string name, std::vector<char> keys, float cool) {
			mFunction = update;
			mAnimator = animator;
			mStateName = name;
			mKeys = keys;
			cooldown = cool;
		};
	} StateController;
	const char* GetState() const {
		return states[nCurrentIndex].mStateName.c_str();
	}
	void SetState(std::string state) {
		if ((states[nCurrentIndex].mAnimator->shouldPlayOnce() && !states[nCurrentIndex].mAnimator->isPlayed()) || !states[GetStateIndex(state)].activated) {
			return;
		}
		int prevIndex = nCurrentIndex;
		states[prevIndex].mAnimator->ResetPlayBool();
		nCurrentIndex = GetStateIndex(state);
	}
	StateController GetCurrentState() {
		return states[nCurrentIndex];
	}
	int GetStateIndex(std::string name) {
		for (int i = 0; i < states.size(); i++)
			if (states[i].mStateName == name)
				return i;
		return 0;
	}
	Structures::Texture RenderState() {
		return states[nCurrentIndex].mAnimator->UpdateFrames();
	}
	bool IsCurrentState(std::string state) const {
		return strcmp(GetState(), state.c_str()) == 0;
	}
	int nCurrentIndex;
	std::vector<StateController> states;
	void Clear() {
		for (auto state : states) {
			std::destroy_at(std::addressof(state.mFunction));
			state.mAnimator->Free();
			delete state.mAnimator;
			state.mKeys.clear();
		}
		states.clear();
	}
};

class StateMachine : public BaseStateMachine {
public:
	void AddState(std::function<void()> func, Animator* anim, std::string name, std::vector<char> keys, float cooldown = 0) {
		states.push_back(StateController(func, anim, name, keys, cooldown / 1000.f));
		nCurrentIndex = states.size() - 1;
	}
	void UpdateState() {
		std::for_each(states.begin(), states.end(), [&, this](StateController& state) {state.Update(); });
		try {
			states[nCurrentIndex].mFunction();
		}
		catch (std::bad_function_call& e) {
			;
		}
		std::for_each(states.begin(), states.end(), [&, this](StateController& state) {
			if (!state.mKeys.empty()) {
				for (auto& key : state.mKeys) {
					if (!IsCurrentState(state.mStateName) && isKeyPressed(key)) {
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
		states.push_back(StateController(func, anim, name, {}, cooldown / 1000.f));
		nCurrentIndex = states.size() - 1;
	}
	void UpdateState() {
		std::for_each(states.begin(), states.end(), [&, this](StateController& state) {state.Update(); });
		try {
			states[nCurrentIndex].mFunction();
		}
		catch (std::bad_function_call& e) {
			;
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
	int nIndex = 0;
	virtual void Render() = 0;
	virtual void FixedUpdate() = 0;
	virtual void Update() = 0;
	virtual void Load() = 0;
	virtual void UnLoad() = 0;
};