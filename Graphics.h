#pragma once
#include <Windows.h>
#include <d3d11_1.h>
#include <dxgi1_4.h>
#include <d2d1_1.h>
#include <dwrite_3.h>
#include <d2d1helper.h>
#include <wincodec.h>
#include <d3dcompiler.h>
#include <iostream>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <fstream>
#include <vector>
#include <utility>
#include "Math.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Structures {
	typedef struct Window {
		static HWND windowHandle;
		static int GetWidth() {
			RECT rc;
			GetClientRect(GetActiveWindow(), &rc);
			return static_cast<int>(rc.right - rc.left);
		}
		static int GetHeight() {
			RECT rc;
			GetClientRect(GetActiveWindow(), &rc);
			return static_cast<int>(rc.bottom - rc.top);
		}
	}Window;
	typedef struct Camera {
		static Math::float3 Position;
		static XMMATRIX projMatrix, viewMatrix, worldMatrix;
		static XMVECTOR eyePos;
		static XMVECTOR lookAtPos;
		static XMVECTOR upVector;
	}Camera;
};

class Graphics {
public:
	static inline Math::float3 GetEyeDistance() {
		return Structures::Camera::Position;
	}
	static void SetEyePosition(Math::float3 vec) {
		Structures::Camera::Position = vec;
		Structures::Camera::worldMatrix = XMMatrixIdentity();
		Structures::Camera::eyePos = XMVectorSet(Structures::Camera::Position.x, Structures::Camera::Position.y,
			-Structures::Camera::Position.z, 0.0f);
		Structures::Camera::lookAtPos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		Structures::Camera::upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		Structures::Camera::viewMatrix = XMMatrixLookAtLH(Structures::Camera::eyePos, 
			Structures::Camera::lookAtPos, Structures::Camera::upVector);
		float fovDegrees = 90.0f;
		float fovRadians = (fovDegrees / 360.0f) * XM_2PI;
		float aspectRatio = (float)Structures::Window::GetWidth() / (float)Structures::Window::GetHeight();
		float nearZ = 0.1f;
		float farZ = 100.0f;
		Structures::Camera::projMatrix = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
	}
	bool InitGraphics(HWND hwnd);
	Graphics();
	~Graphics();
	ID2D1RenderTarget* GetRenderTarget() {
		return renderTarget.Get();
	}
	ID2D1Factory1* GetFactory() {
		return factory.Get();
	}
	IDWriteFactory5* getWriteFactory() {
		return dWriteFactory.Get();
	}
	typedef struct Vertex {
		XMFLOAT2 pos;
		XMFLOAT4 color;
		XMFLOAT2 tex;
	}Vertex;
	typedef struct Constants
	{
		XMFLOAT2 pos;
		XMFLOAT2 paddingUnused;
		XMFLOAT4 horizontalScale;
	}Constants;
	typedef struct CollisionConstants
	{
		XMFLOAT2 pos;
		XMFLOAT2 paddingUnused;
		XMFLOAT4 collisionValues;
	}CollisionConstants;
	typedef struct ProjectionBuffer
	{
		XMMATRIX proj;
		XMMATRIX world;
		XMMATRIX view;
	}ProjBuffer;
	void Clear(float r, float g, float b, float a);
	void Begin();
	void End();
	HRESULT InitWritingFactory();
	UINT stride, offset;
	HWND windowHandle;
	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11Buffer> projectionBuffer;	
	ComPtr<ID3D11Buffer> enemyConstantBuffer;
	ComPtr<ID3D11Buffer> constantBuffer;
	ComPtr<ID3D11SamplerState> samplerState;
	ComPtr<ID3D11PixelShader> mainPixelShader;
	ComPtr<ID3D11PixelShader> pixelShader;
	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID2D1SolidColorBrush> blackColor;
	ComPtr<ID2D1SolidColorBrush> whiteColor;
	ComPtr<ID2D1SolidColorBrush> snowColor;
	void DrawTextF(std::wstring text, float x, float y, float width, float height, ID2D1Brush* color);
	ComPtr<ID3D11Device> d3dDevice;
	ComPtr<ID3D11DeviceContext> d3dDeviceContext;
	ComPtr<ID3D11DepthStencilState> depthStencilState;
	ComPtr<IDXGISwapChain> swapChain;
	ComPtr<ID3D11RenderTargetView> renderTargetView;
	ComPtr<ID3D11RasterizerState> rasterizerState;
	float renderTargetWidth, renderTargetHeight;
protected:
	ComPtr<IDXGIFactory1> dxgiFactory;
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIDevice> dxgiDevice;
	ComPtr<ID2D1Factory1> factory;
	ComPtr<IDWriteFactory5> dWriteFactory;
	ComPtr<IDWriteTextFormat> textFormat;
	ComPtr<IDWriteTextLayout> textLayout;
	ComPtr<ID2D1RenderTarget> dxgiRenderTarget;
	ComPtr<ID2D1HwndRenderTarget> renderTarget;
	ComPtr<ID3D11Texture2D> renderTargetTexture;
	ComPtr<ID3D11Texture2D> backBuffer;
};