#pragma warning(disable:4996)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#define STB_IMAGE_IMPLEMENTATION
#include "Levels.h"
#include "Engine/GameController.h"

GameController controller;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    WNDCLASSEXW winClass = {};
    winClass.cbSize = sizeof(WNDCLASSEXW);
    winClass.style = CS_HREDRAW | CS_VREDRAW;
    winClass.lpfnWndProc = WndProc;
    winClass.hInstance = hInstance;
    winClass.hIcon = LoadIconW(0, IDI_APPLICATION);
    winClass.hCursor = LoadCursorW(0, IDC_ARROW);
    winClass.lpszClassName = Window::className.c_str();
    winClass.hIconSm = LoadIconW(0, IDI_APPLICATION);
    if (!RegisterClassExW(&winClass)) {
        MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
        return GetLastError();
    }
    RECT initialRect = { 0, 0, 1024, 768 };
    AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
    Window::width = initialRect.right - initialRect.left;
    Window::height = initialRect.bottom - initialRect.top;
    Window::windowHandle = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
        Window::className.c_str(),
        Window::windowName.c_str(),
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        Window::width,
        Window::height,
        0, 0, hInstance, 0);
    if (!Window::windowHandle) {
        MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
        return GetLastError();
    }
    LoadFontData();
    Graphics::InitDevices();
    Graphics::InitSwapChain();
    Graphics::InitRenderTarget();
    Graphics::InitSampler();
    Graphics::InitRasterizer();
    Graphics::InitBlendState();
    Graphics::InitCamera({ 0, 0, 5 });
    controller.vLevels.emplace_back(std::make_unique<MainMenu>());
    controller.vLevels.emplace_back(std::make_unique<Main>());
    controller.vLevels.emplace_back(std::make_unique<Dead>());
    controller.vLevels.emplace_back(std::make_unique<Credits>());
    controller.vLevels.emplace_back(std::make_unique<Marketplace>());
    controller.vLevels.emplace_back(std::make_unique<Win>());
    controller.Load();
    Window::windowMessage.message = WM_NULL;
    while (Window::windowMessage.message != WM_QUIT) {
        if (PeekMessage(&Window::windowMessage, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&Window::windowMessage);
            DispatchMessage(&Window::windowMessage);
        }
        else {
            controller.Update();
            controller.Render();
        }
    }
    controller.Unload();
    FreeFontData();
    controller.vLevels.clear();
    Graphics::swapChain->SetFullscreenState(FALSE, NULL);
    return Window::windowMessage.wParam;
}

int main() {
    return 0;
}