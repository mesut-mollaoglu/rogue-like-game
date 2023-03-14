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

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class Graphics {
public:
	XMMATRIX projMatrix, viewMatrix, worldMatrix;
	int width, height;
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
	struct Vertex {
		XMFLOAT2 pos;
		XMFLOAT4 color;
		XMFLOAT2 tex;
	};
	struct Constants
	{
		XMFLOAT2 pos;
		XMFLOAT2 paddingUnused;
		XMFLOAT4 horizontalScale;
	};
	struct CollisionConstants
	{
		XMFLOAT2 pos;
		XMFLOAT2 paddingUnused;
		XMFLOAT4 collisionValues;
	};
	struct ProjectionBuffer
	{
		XMMATRIX proj;
		XMMATRIX world;
		XMMATRIX view;
	};
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