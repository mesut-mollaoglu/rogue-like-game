#include "GameController.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#pragma warning(disable:4996)

GameController* controller;
Graphics* graphics;
Sprite* spriteLoader;
HWND windowHandle = nullptr;

StateMachine::MouseWheel StateMachine::mouseWheel;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_MOUSEWHEEL) { StateMachine::mouseWheel = StateMachine::GetMouseWheel(wParam); }
	if (message == WM_DESTROY) { PostQuitMessage(0); controller->Unload(); return 0; }
	DefWindowProc(hwnd, message, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
	printf("Debugging Window:\n");
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"ClassName";
	wc.lpszMenuName = nullptr;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClassEx(&wc);
	RECT rect = { 0, 0, 1920, 1046};
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);
	windowHandle = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"ClassName", L"Rogue Like Game", WS_OVERLAPPEDWINDOW, 100, 100, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);
	graphics = new Graphics();
	controller = new GameController();
	spriteLoader = new Sprite();
	if (!graphics->InitGraphics(windowHandle)) {
		delete graphics;
		return -1;
	}
	controller->Init(graphics);
	spriteLoader->Init(graphics);
	ShowWindow(windowHandle, nCmdShow);
	controller->Load(spriteLoader, windowHandle);
	MSG message = { 0 };
	message.message = WM_NULL;
	while (message.message != WM_QUIT) {
		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		else {
			controller->Render();
			controller->Update(windowHandle, message);
		}
	}
	return 0;
}