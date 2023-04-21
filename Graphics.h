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
		static MSG message;
	}Window;
	typedef struct Camera {
		static Math::float3 Position;
		static XMMATRIX projMatrix, viewMatrix, worldMatrix;
		static XMVECTOR eyePos;
		static XMVECTOR lookAtPos;
		static XMVECTOR upVector;
		static float fovDegrees;
		static float fovRadians;
		static float aspectRatio;
		static float nearZ;
		static float farZ;
		static XMVECTOR defaultUp;
		static XMVECTOR defaultForward;
		static XMMATRIX rotationDefault;
	}Camera;
};

class Graphics {
public:
	static inline Math::float3 GetEyeDistance() {
		return Structures::Camera::Position;
	}
	static void SetEyePosition(Math::float3 vec) {
		using Structures::Camera;
		Camera::defaultUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		Camera::defaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		Camera::rotationDefault = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
		Camera::Position = vec;
		Camera::worldMatrix = XMMatrixIdentity();
		Camera::eyePos = XMVectorSet(0.0f, 0.0f, -Camera::Position.z, 0.0f);
		Camera::lookAtPos = XMVector3TransformCoord(Camera::defaultForward, Camera::rotationDefault);
		Camera::lookAtPos += Camera::eyePos;
		Camera::upVector = XMVector3TransformCoord(Camera::defaultUp, Camera::rotationDefault);
		Camera::viewMatrix = XMMatrixLookAtLH(Camera::eyePos, Camera::lookAtPos, Camera::upVector);
		Camera::fovDegrees = 90.0f;
		Camera::fovRadians = (Camera::fovDegrees / 360.0f) * XM_2PI;
		Camera::aspectRatio = (float)Structures::Window::GetWidth() / (float)Structures::Window::GetHeight();
		Camera::nearZ = 0.1f;
		Camera::farZ = 100.0f; 
		Camera::projMatrix = XMMatrixPerspectiveFovLH(Camera::fovRadians, Camera::aspectRatio, Camera::nearZ,
			Camera::farZ);
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
		XMFLOAT2 cameraPos;
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