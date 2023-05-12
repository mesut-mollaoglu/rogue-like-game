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
#include <initializer_list>
#include <memory>

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
	static ID2D1RenderTarget* GetRenderTarget() {
		return Graphics::renderTarget.Get();
	}
	static ID2D1Factory1* GetFactory() {
		return Graphics::factory.Get();
	}
	static IDWriteFactory5* getWriteFactory() {
		return Graphics::dWriteFactory.Get();
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
	static void Clear(float r, float g, float b, float a);
	static void Begin();
	static void End();
	static HRESULT InitWritingFactory();
	static UINT stride, offset;
	HWND windowHandle;
	static ComPtr<ID3D11InputLayout> inputLayout;
	static ComPtr<ID3D11Buffer> projectionBuffer;
	static ComPtr<ID3D11SamplerState> samplerState;
	static ComPtr<ID3D11PixelShader> mainPixelShader;
	static ComPtr<ID3D11PixelShader> pixelShader;
	static ComPtr<ID3D11VertexShader> vertexShader;
	static ComPtr<ID2D1SolidColorBrush> blackColor;
	static ComPtr<ID2D1SolidColorBrush> whiteColor;
	static ComPtr<ID2D1SolidColorBrush> snowColor;
	static void DrawTextF(std::wstring text, float x, float y, float width, float height, ID2D1Brush* color);
	static ComPtr<ID3D11Device> d3dDevice;
	static ComPtr<ID3D11DeviceContext> d3dDeviceContext;
	static ComPtr<ID3D11DepthStencilState> depthStencilState;
	static ComPtr<IDXGISwapChain> swapChain;
	static ComPtr<ID3D11RenderTargetView> renderTargetView;
	static ComPtr<ID3D11RasterizerState> rasterizerState;
	float renderTargetWidth, renderTargetHeight;
	static HRESULT CreateVertexBuffer(ComPtr<ID3D11Buffer>& vertexBuffer, Graphics::Vertex* vertex, UINT numVertices);
	static HRESULT CreateIndexBuffer(ComPtr<ID3D11Buffer>& indexBuffer, DWORD* indices = 0, UINT numIndices = 0);
	static HRESULT CreatePixelShader(std::wstring filename, ComPtr<ID3D11PixelShader>& shader);
	static HRESULT CreateVertexShader(std::wstring filename, ComPtr<ID3D11VertexShader>& shader, ComPtr<ID3D11InputLayout>& inputLayout, D3D11_INPUT_ELEMENT_DESC* desc, UINT arraySize);
	template <class T> static HRESULT CreateConstantBuffer(ComPtr<ID3D11Buffer>& buffer) {
		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.ByteWidth = sizeof(T) + 0xf & 0xfffffff0;
		constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		HRESULT hr = d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, buffer.GetAddressOf());
		return hr;
	}
	template <class T>
	static void SetConstantValues(ComPtr<ID3D11Buffer> buffer, T data) {
		D3D11_MAPPED_SUBRESOURCE projectionSubresource;
		Graphics::d3dDeviceContext->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &projectionSubresource);
		CopyMemory(projectionSubresource.pData, &data, sizeof(T));
		Graphics::d3dDeviceContext->Unmap(buffer.Get(), 0);
	}
private:
	ComPtr<IDXGIFactory1> dxgiFactory;
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIDevice> dxgiDevice;
	static ComPtr<ID2D1Factory1> factory;
	static ComPtr<IDWriteFactory5> dWriteFactory;
	static ComPtr<IDWriteTextFormat> textFormat;
	static ComPtr<IDWriteTextLayout> textLayout;
	ComPtr<ID2D1RenderTarget> dxgiRenderTarget;
	static ComPtr<ID2D1HwndRenderTarget> renderTarget;
	ComPtr<ID3D11Texture2D> renderTargetTexture;
	ComPtr<ID3D11Texture2D> backBuffer;
};