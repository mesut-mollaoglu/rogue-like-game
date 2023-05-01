#pragma once

#include "Animation.h"
#include <string>
#include <algorithm>

class StateMachine {
public:
	typedef struct Key {
		std::vector<char> keys;
		Key() = default;
		Key(char c) {
			Add(c);
		}
		void Add(char c) {
			keys.push_back(c);
		}
		template<class... Args>
		void KeyComb(char c, Args... args) {
			Add(c);
			Add(args...);
		}
		bool isPressed() {
			bool keyPressed = true;
			std::for_each(keys.begin(), keys.end(), [&, this](char a) {keyPressed &= StateMachine::isKeyPressed(a); });
			return keyPressed;
		}
		bool isReleased() {
			bool keyPressed = true;
			std::for_each(keys.begin(), keys.end(), [&, this](char a) {keyPressed &= StateMachine::isKeyReleased(a); });
			return keyPressed;
		}
	}Key;
	static enum class MouseWheel {
		WHEEL_UP,
		WHEEL_DOWN,
		WHEEL_UNKNOWN,
	};
	typedef std::chrono::time_point<std::chrono::high_resolution_clock> timePoint;
	typedef struct StateController {
		std::function<void()> updateFunc;
		Animator* renderFunc;
		std::string stateName;
		std::vector<StateMachine::Key> activationKeys;
		float cooldown;
		timePoint tp1;
		bool activated = true;
		void Update() {
			if (renderFunc->isPlayed() && renderFunc->shouldPlayOnce()) {
				activated = false;
				tp1 = Clock::now();
			}
			if (cooldown < StateMachine::GetTimeLapse(Clock::now(), tp1)) {
				activated = true;
				tp1 = Clock::now();
			}
		}
		StateController() = default;
		StateController(std::function<void()> update, Animator* anim, std::string name, std::vector<Key> keys, float cool){
			updateFunc = update;
			renderFunc = anim;
			stateName = name;
			activationKeys = keys;
			cooldown = cool;
		};
		~StateController() {}
	} StateController;
	StateMachine(){

	}
	~StateMachine() {

	}
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
		if(animator != nullptr) delete animator;
	}
	virtual void AddStateComb(std::function<void()> func, Animator* anim, std::string name, std::vector<Key> keys, float cooldown = 0) {
		StateController* controller = new StateController(func, anim, name, keys, cooldown / 1000);
		states.push_back(*controller);
		delete controller;
		this->currentState = states[0];
	}
	virtual void AddState(std::function<void()> func, Animator* anim, std::string name, std::vector<char> keys, float cooldown = 0) {
		std::vector<Key> keyVec;
		for (const char& c : keys){
			keyVec.push_back(StateMachine::Key(c));
		}
		AddStateComb(func, anim, name, keyVec, cooldown);
	}
	static float GetTimeLapse(timePoint tp1, timePoint tp2) {
		return std::chrono::duration<float>(tp1 - tp2).count();
	}
	static MouseWheel GetMouseWheel(MSG msg = Structures::Window::message) {
		if (msg.message == WM_MOUSEWHEEL){
			short delta = GET_WHEEL_DELTA_WPARAM(msg.wParam);
			if (delta > 0) return MouseWheel::WHEEL_UP;
			else if (delta < 0) return MouseWheel::WHEEL_DOWN;
			else return MouseWheel::WHEEL_UNKNOWN;
		}
	}
	StateController GetCurrentState() {
		for (int i = 0; i < states.size(); i++) {
			if (this->equals(states[i].stateName)) {
				currentState = states[i];
			}
		}
		return currentState;
	}
	StateController GetStateByName(std::string name) {
		for (int i = 0; i < states.size(); i++) {
			if (strcmp(name.c_str(), states[i].stateName.c_str()) == 0) {
				return states[i];
			}
		}
	}

	void UpdateState() {
		for (StateController& state : states) {
			state.Update();
		}
		try {
			this->currentState.updateFunc();
		}
		catch(std::bad_function_call &e){
			std::cout << e.what() << std::endl;
		}
		for (StateController state : states) {
			if (!state.activationKeys.empty()) {
				for (auto &key : state.activationKeys) {
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
		}
	}
	ID3D11ShaderResourceView* RenderState() {
		return GetCurrentState().renderFunc->UpdateFrames();
	}
	static bool isKeyPressed(char a) {
		return (GetAsyncKeyState((unsigned short)a) & 0x8000);
	}
	template<class... Args>
	static bool isKeyPressed(char a, Args... args) {
		bool isPressed = StateMachine::isKeyPressed(a);
		isPressed = isPressed && isKeyPressed(args...);
		return isPressed;
	}
	static bool isKeyReleased(char a, MSG msg = Structures::Window::message) {
		if (msg.message == WM_KEYUP && a == (char)msg.wParam) return true;
	}
	template<class... Args>
	static bool isKeyReleased(char a, Args... args) {
		bool isReleased = StateMachine::isKeyReleased(a);
		isReleased = isReleased && isKeyReleased(args...);
		return isReleased;
	}
	static bool getKeyPressed() {
		for (int i = 0x01; i < 0xFE; i++) {
			if (isKeyPressed(i)) return true;
		}
		return false;
	}
	virtual bool equals(std::string state) {
		return strcmp(this->GetState(), state.c_str()) == 0;
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
private:
	using Clock = std::chrono::high_resolution_clock;
	StateController currentState;
	StateController previousState;
	std::vector<StateController> states;
};
