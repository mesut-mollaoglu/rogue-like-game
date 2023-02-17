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
	struct ProjectionBuffer
	{
		XMMATRIX proj;
		XMMATRIX world;
		XMMATRIX view;
	};
	HRESULT InitWritingFactory();
	UINT stride, offset;
	HWND windowHandle;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> projectionBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> enemyConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackColor;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> whiteColor;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> snowColor;
	void DrawTextF(std::wstring text, float x, float y, float width, float height, ID2D1Brush* color);
	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	float renderTargetWidth, renderTargetHeight;
protected:
	Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	Microsoft::WRL::ComPtr<ID2D1Factory1> factory;
	Microsoft::WRL::ComPtr<IDWriteFactory5> dWriteFactory;
	Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
	Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout;
	Microsoft::WRL::ComPtr<ID2D1RenderTarget> dxgiRenderTarget;
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> renderTarget;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTargetTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
};