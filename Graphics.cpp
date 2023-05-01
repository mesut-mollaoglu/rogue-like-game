#include "Graphics.h"

UINT Graphics::stride = sizeof(Graphics::Vertex);
UINT Graphics::offset = 0;
ComPtr<ID3D11Device> Graphics::d3dDevice = nullptr;
ComPtr<ID3D11DeviceContext> Graphics::d3dDeviceContext = nullptr;
Math::float3 Structures::Camera::Position;
XMVECTOR Structures::Camera::eyePos;
XMVECTOR Structures::Camera::lookAtPos;
XMVECTOR Structures::Camera::upVector;
XMMATRIX Structures::Camera::projMatrix;
XMMATRIX Structures::Camera::viewMatrix;
XMMATRIX Structures::Camera::worldMatrix;
float Structures::Camera::fovDegrees;
float Structures::Camera::fovRadians;
float Structures::Camera::aspectRatio;
float Structures::Camera::nearZ;
float Structures::Camera::farZ;
HWND Structures::Window::windowHandle = GetActiveWindow();
XMMATRIX Structures::Camera::rotationDefault;
XMVECTOR Structures::Camera::defaultForward;
XMVECTOR Structures::Camera::defaultUp;
ComPtr<ID3D11DepthStencilState> Graphics::depthStencilState;
ComPtr<IDXGISwapChain> Graphics::swapChain;
ComPtr<ID3D11RenderTargetView> Graphics::renderTargetView;
ComPtr<ID3D11RasterizerState> Graphics::rasterizerState;
ComPtr<ID3D11InputLayout> Graphics::inputLayout;
ComPtr<ID3D11Buffer> Graphics::projectionBuffer;
ComPtr<ID3D11SamplerState> Graphics::samplerState;
ComPtr<ID3D11PixelShader> Graphics::mainPixelShader;
ComPtr<ID3D11PixelShader> Graphics::pixelShader;
ComPtr<ID3D11VertexShader> Graphics::vertexShader;
ComPtr<ID2D1Factory1> Graphics::factory;
ComPtr<IDWriteFactory5> Graphics::dWriteFactory;
ComPtr<IDWriteTextFormat> Graphics::textFormat;
ComPtr<IDWriteTextLayout> Graphics::textLayout;
ComPtr<ID2D1HwndRenderTarget> Graphics::renderTarget;
ComPtr<ID2D1SolidColorBrush> Graphics::blackColor;
ComPtr<ID2D1SolidColorBrush> Graphics::whiteColor;
ComPtr<ID2D1SolidColorBrush> Graphics::snowColor;

Graphics::Graphics() {

}

Graphics::~Graphics() {
	swapChain->SetFullscreenState(FALSE, NULL);
	if (this->factory) this->factory->Release();
	if (this->renderTarget) this->renderTarget->Release();
	if (this->dxgiRenderTarget) this->dxgiRenderTarget->Release();
	if (this->dWriteFactory) this->dWriteFactory->Release();
	if (this->textFormat) this->textFormat->Release();
}

void Graphics::Clear(float r, float g, float b, float a)
{
	const float color[4] = { r, g, b, a };
 	Graphics::d3dDeviceContext->ClearRenderTargetView(Graphics::renderTargetView.Get(), color);
}

void Graphics::Begin()
{
	Graphics::d3dDeviceContext->OMSetDepthStencilState(Graphics::depthStencilState.Get(), 1);
	Graphics::d3dDeviceContext->RSSetState(Graphics::rasterizerState.Get());
	Graphics::d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Graphics::d3dDeviceContext->VSSetShader(Graphics::vertexShader.Get(), nullptr, 0);
	Graphics::d3dDeviceContext->PSSetShader(Graphics::mainPixelShader.Get(), nullptr, 0);
	Graphics::d3dDeviceContext->IASetInputLayout(Graphics::inputLayout.Get());
	Graphics::d3dDeviceContext->PSSetSamplers(0, 1, Graphics::samplerState.GetAddressOf());
	Graphics::d3dDeviceContext->VSSetConstantBuffers(1, 1, Graphics::projectionBuffer.GetAddressOf());
}

void Graphics::End()
{
	Graphics::swapChain->Present(1, NULL);
}

HRESULT Graphics::CreateVertexBuffer(ComPtr<ID3D11Buffer>& vertexBuffer, Graphics::Vertex* vertex, UINT numVertices) {
	D3D11_BUFFER_DESC bd = { 0 };
	bd.ByteWidth = sizeof(Graphics::Vertex) * numVertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA srd = { vertex, 0, 0 };
	HRESULT hr = Graphics::d3dDevice->CreateBuffer(&bd, &srd, vertexBuffer.GetAddressOf());
	return hr;
}

HRESULT Graphics::CreateIndexBuffer(ComPtr<ID3D11Buffer>& indexBuffer, DWORD* indices, UINT numIndices) {
	DWORD defaultIndices[] = {
		0, 1, 2,
		0, 2, 3
	};
	if (indices == 0 || numIndices == 0) {
		indices = defaultIndices;
		numIndices = ARRAYSIZE(defaultIndices);
	}
	D3D11_BUFFER_DESC indexBufferDesc = { 0 };
	indexBufferDesc.ByteWidth = sizeof(indices) * numIndices;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	D3D11_SUBRESOURCE_DATA indexSubData = { indices, 0, 0 };
	HRESULT hr = Graphics::d3dDevice.Get()->CreateBuffer(&indexBufferDesc, &indexSubData, indexBuffer.GetAddressOf());
	return hr;
	delete defaultIndices;
}

HRESULT Graphics::CreatePixelShader(std::wstring filename, ComPtr<ID3D11PixelShader>& shader) {
	shader = nullptr;
	ID3DBlob* pixelShaderBlob;
	HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &pixelShaderBlob, nullptr);
	if (FAILED(hr)) {
		MessageBox(Structures::Window::windowHandle, L"Error: Can't create shader.", L"Error", MB_OK);
	}
	assert(SUCCEEDED(hr));
	hr = Graphics::d3dDevice.Get()->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr,
		shader.GetAddressOf());
	if (FAILED(hr)) { MessageBox(Structures::Window::windowHandle, L"Error: Can't create shader.", L"Error", MB_OK); }
	pixelShaderBlob->Release();
	return hr;
}

HRESULT Graphics::CreateVertexShader(std::wstring filename, ComPtr<ID3D11VertexShader>& shader, ComPtr<ID3D11InputLayout>& inputLayout, D3D11_INPUT_ELEMENT_DESC* desc, UINT arraySize) {
	shader = nullptr;
	ID3DBlob* vertexShaderBlob;
	HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &vertexShaderBlob, nullptr);
	if (FAILED(hr)) {
		MessageBox(Structures::Window::windowHandle, L"Error: Can't create shader.", L"Error", MB_OK);
	}
	assert(SUCCEEDED(hr));
	hr = Graphics::d3dDevice.Get()->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr,
		shader.GetAddressOf());
	if (FAILED(hr)) { MessageBox(Structures::Window::windowHandle, L"Error: Can't create shader.", L"Error", MB_OK); }
	Graphics::d3dDevice->CreateInputLayout(desc, arraySize, vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize(), inputLayout.GetAddressOf());
	vertexShaderBlob->Release();
	return hr;
}

HRESULT Graphics::InitWritingFactory() {
	HRESULT hr;
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory5),
		reinterpret_cast<IUnknown**>(dWriteFactory.GetAddressOf()));
	if (SUCCEEDED(hr)) {
		hr = dWriteFactory->CreateTextFormat(L"scientifica", NULL, DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 60.0f, L"en-us", &textFormat);
	}
	if (SUCCEEDED(hr)) { hr = textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER); }
	if (SUCCEEDED(hr)) { textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER); }
	return hr;
}

void Graphics::DrawTextF(std::wstring text, float x, float y, float width, float height, ID2D1Brush* color) {
	HRESULT hr;
	if (SUCCEEDED(InitWritingFactory())) {
		hr = dWriteFactory->CreateTextLayout(text.c_str(), (UINT)wcslen(text.c_str()), textFormat.Get(),
			width, height, textLayout.GetAddressOf());
	}
	if (SUCCEEDED(hr)) {
		renderTarget->DrawTextLayout(D2D1::Point2F(x, y), textLayout.Get(), color, D2D1_DRAW_TEXT_OPTIONS_NONE);
	}
	textLayout.Get()->Release();
}

bool Graphics::InitGraphics(HWND hwnd) {
	Structures::Window::windowHandle = hwnd;
	int width = Structures::Window::GetWidth();
	int height = Structures::Window::GetHeight();
	//Direct3D initialization
	{
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory.GetAddressOf());
	if (hr != S_OK) return false;
	CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
	dxgiFactory->EnumAdapters1(0, adapter.GetAddressOf());
	hr = D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, 0, D3D11_SDK_VERSION,
		&d3dDevice, &featureLevel, &d3dDeviceContext);
	if (hr != S_OK) return false;
	hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)dxgiDevice.GetAddressOf());
	if (hr != S_OK) return false;
	RECT rect;
	GetClientRect(hwnd, &rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	dxgiFactory->CreateSwapChain(this->d3dDevice.Get(), &swapChainDesc, swapChain.GetAddressOf());
	hr = this->swapChain.Get()->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
	Microsoft::WRL::ComPtr<IDXGISurface> surfaceBuffer;
	hr = this->backBuffer.Get()->QueryInterface(IID_PPV_ARGS(surfaceBuffer.GetAddressOf()));
	if (hr != S_OK) return false;
	backBuffer->GetDesc(&textureDesc);
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	hr = this->d3dDevice->CreateRenderTargetView(this->backBuffer.Get(), &renderTargetViewDesc, renderTargetView.GetAddressOf());
	if (hr != S_OK) return false;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	hr = this->d3dDevice->CreateTexture2D(&textureDesc, NULL, depthStencilBuffer.GetAddressOf());
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	hr = this->d3dDevice->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf());
	if (SUCCEEDED(hr)) this->d3dDeviceContext->OMSetDepthStencilState(this->depthStencilState.Get(), 1);
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	hr = this->d3dDevice->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());
	if (SUCCEEDED(hr)) this->d3dDeviceContext->RSSetState(this->rasterizerState.Get());
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Format = textureDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	ID3D11DepthStencilView* depthStencilView;
	hr = this->d3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc,
		&depthStencilView);
	D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
	backBuffer->GetDesc(&backBufferDesc);
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(backBufferDesc.Width);
	viewport.Height = static_cast<float>(backBufferDesc.Height);
	viewport.MinDepth = D3D11_MIN_DEPTH;
	viewport.MaxDepth = D3D11_MAX_DEPTH;
	d3dDeviceContext->RSSetViewports(1, &viewport);
	this->d3dDeviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView);
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	this->d3dDevice.Get()->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
	D3D11_BUFFER_DESC constantBufferDesc;
	}
	//Direct2D initialization
	{
	float dpi = GetDpiForWindow(hwnd);
	HRESULT hr = factory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(width, height)),
		renderTarget.GetAddressOf());
	this->renderTarget->SetDpi(ceil(this->renderTarget->GetSize().width * dpi / (float)width),
		this->renderTarget->GetSize().height * dpi / (float)height);
	renderTargetWidth = this->renderTarget->GetSize().width;
	renderTargetHeight = this->renderTarget->GetSize().height;
	if (hr != S_OK) return false;
	renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), blackColor.GetAddressOf());
	renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), whiteColor.GetAddressOf());
	renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 0.3f), snowColor.GetAddressOf());
	if (hr != S_OK) return false;
	}
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	Graphics::CreatePixelShader(L"MainPixelShader.hlsl", this->mainPixelShader);
	Graphics::CreatePixelShader(L"PixelShader.hlsl", this->pixelShader);
	Graphics::CreateVertexShader(L"VertexShader.hlsl", this->vertexShader, this->inputLayout, ied, ARRAYSIZE(ied));
	Graphics::CreateConstantBuffer<Graphics::ProjectionBuffer>(projectionBuffer);
	Graphics::SetEyePosition(Math::float3(0.0f, 0.0f, 5.0f));
	return true;
}