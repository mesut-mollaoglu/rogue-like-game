#pragma once

#include "Animation.h"
#include <string>
#include <algorithm>

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
	static MouseWheel GetMouseWheel(MSG msg = Structures::Window::message) {
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
	static bool isKeyReleased(char a, MSG msg = Structures::Window::message) {
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
	static Math::float2 GetMousePos() {
		POINT* pt = new POINT;
		GetCursorPos(pt);
		ScreenToClient(GetActiveWindow(), pt);
		return Math::float2((float)pt->x, (float)pt->y);
		delete pt;
	}
	static Math::float2 ToScreenCoord(Math::float2 pos) {
		Math::float2 ret;
		ret.x = Graphics::GetEyeDistance().z * Structures::Window::GetWidth() * (2.0f * pos.x / Structures::Window::GetWidth() - 1.0f);
		ret.y = -Graphics::GetEyeDistance().z * Structures::Window::GetHeight() * (2.0f * pos.y / Structures::Window::GetHeight() - 1.0f);
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
		std::function<void()> updateFunc;
		Animator* renderFunc;
		std::string stateName;
		std::vector<BaseStateMachine::Key> activationKeys;
		float cooldown;
		timePoint tp1;
		bool activated = true;
		void Update() {
			if (renderFunc->isPlayed() && renderFunc->shouldPlayOnce()) {
				activated = false;
				tp1 = Clock::now();
			}
			if (cooldown < BaseStateMachine::GetTimeLapse(Clock::now(), tp1)) {
				activated = true;
				tp1 = Clock::now();
			}
		}
		StateController() = default;
		StateController(std::function<void()> update, Animator* anim, std::string name, std::vector<Key> keys, float cool) {
			updateFunc = update;
			renderFunc = anim;
			stateName = name;
			activationKeys = keys;
			cooldown = cool;
		};
	} StateController;
	const char* GetState() const {
		return currentState.stateName.c_str();
	}
	void SetState(std::string state) {
		Animator* animator = new Animator(*this->currentState.renderFunc);
		if (animator->shouldPlayOnce() && !animator->isPlayed()) {
			delete animator;
			return;
		}
		if (!GetStateByName(state).activated) {
			return;
		}
		previousState = currentState;
		previousState.renderFunc->ResetPlayBool();
		currentState.stateName = state;
		if (animator != nullptr) delete animator;
	}
	StateController GetCurrentState() {
		std::for_each(states.begin(), states.end(), [&, this](StateController& state) {
			if (this->equals(state.stateName)) {
				this->currentState = state;
			}
			});
		return currentState;
	}
	StateController GetStateByName(std::string name) {
		StateController returnState;
		std::for_each(states.begin(), states.end(), [&, this](StateController& state) {
			if (strcmp(name.c_str(), state.stateName.c_str()) == 0) returnState = state;
			});
		return returnState;
	}
	ID3D11ShaderResourceView* RenderState() {
		return GetCurrentState().renderFunc->UpdateFrames();
	}
	bool equals(std::string state) const {
		return strcmp(this->GetState(), state.c_str()) == 0;
	}
	using Clock = std::chrono::high_resolution_clock;
	StateController currentState;
	StateController previousState;
	std::vector<StateController> states;
};

class StateMachine : public BaseStateMachine {
public:
	void AddStateComb(std::function<void()> func, Animator* anim, std::string name, std::vector<Key> keys, float cooldown = 0) {
		StateController* controller = new StateController(func, anim, name, keys, cooldown / 1000);
		states.push_back(*controller);
		delete controller;
		this->currentState = states[0];
	}
	void AddState(std::function<void()> func, Animator* anim, std::string name, std::vector<char> keys, float cooldown = 0) {
		std::vector<Key> container;
		std::for_each(keys.begin(), keys.end(), [&, this](char& a) {container.push_back(a); });
		AddStateComb(func, anim, name, container, cooldown);
	}
	void UpdateState() {
		std::for_each(states.begin(), states.end(), [&, this](StateController& state) {state.Update(); });
		try {
			this->currentState.updateFunc();
		}
		catch (std::bad_function_call& e) {
			std::cout << e.what() << std::endl;
		}
		std::for_each(states.begin(), states.end(), [&, this](StateController& state) {
			if (!state.activationKeys.empty()) {
				for (auto& key : state.activationKeys) {
					if (!this->equals(state.stateName) && key.isPressed()) {
						SetState(state.stateName);
					}
				}
			}
			else {
				if (!getKeyPressed()) {
					SetState(state.stateName);
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
		this->currentState = states[0];
	}
	void UpdateState() {
		std::for_each(states.begin(), states.end(), [&, this](StateController& state) {state.Update(); });
		try {
			this->currentState.updateFunc();
		}
		catch (std::bad_function_call& e) {
			std::cout << e.what() << std::endl;
		}
	}
};