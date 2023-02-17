#include "Graphics.h"

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

HRESULT Graphics::InitWritingFactory() {
	HRESULT hr;
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory5),
		reinterpret_cast<IUnknown**>(dWriteFactory.GetAddressOf()));
	if (SUCCEEDED(hr)) {
		hr = this->dWriteFactory->CreateTextFormat(L"scientifica", NULL, DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 60.0f, L"en-us", &textFormat);
	}
	if (SUCCEEDED(hr)) { hr = this->textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER); }
	if (SUCCEEDED(hr)) { this->textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER); }
	return hr;
}

void Graphics::DrawTextF(std::wstring text, float x, float y, float width, float height, ID2D1Brush* color) {
	HRESULT hr;
	if (SUCCEEDED(this->InitWritingFactory())) {
		hr = this->dWriteFactory->CreateTextLayout(text.c_str(), (UINT)wcslen(text.c_str()), this->textFormat.Get(),
			width, height, textLayout.GetAddressOf());
	}
	if (SUCCEEDED(hr)) {
		this->renderTarget->DrawTextLayout(D2D1::Point2F(x, y), textLayout.Get(), color, D2D1_DRAW_TEXT_OPTIONS_NONE);
	}
	this->textLayout.Get()->Release();
}

bool Graphics::InitGraphics(HWND hwnd) {
	this->windowHandle = hwnd;
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
	std::cout << width << " " << height << std::endl;
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
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
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
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
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
	this->renderTarget->SetDpi(ceil(this->renderTarget->GetSize().width * dpi / (float)this->width),
		this->renderTarget->GetSize().height * dpi / (float)this->height);
	renderTargetWidth = this->renderTarget->GetSize().width;
	renderTargetHeight = this->renderTarget->GetSize().height;
	if (hr != S_OK) return false;
	renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), blackColor.GetAddressOf());
	renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), whiteColor.GetAddressOf());
	renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 0.3f), snowColor.GetAddressOf());
	if (hr != S_OK) return false;
	}
	//Input layout, pixel shader and vertex shader creation.
	{
	ID3DBlob* shaderBlob;
	HRESULT hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &shaderBlob, nullptr);
	if (FAILED(hr)) {
		hr = D3DCompileFromFile(L"PixelShader.cso", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &shaderBlob, nullptr); 
	}
	assert(SUCCEEDED(hr));
	hr = this->d3dDevice.Get()->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr,
		pixelShader.GetAddressOf());
	if (FAILED(hr)) { MessageBox(hwnd, L"Error: Can't create shader.", L"Error", MB_OK); }
	shaderBlob->Release();
	ID3DBlob* vertexShaderBlob;
	hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &vertexShaderBlob, nullptr);
	if (FAILED(hr)) { 
		hr = D3DCompileFromFile(L"VertexShader.cso", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &vertexShaderBlob, nullptr);
	}
	assert(SUCCEEDED(hr));
	hr = this->d3dDevice.Get()->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr,
		vertexShader.GetAddressOf());
	if (FAILED(hr)) { MessageBox(hwnd, L"Error: Can't create shader.", L"Error", MB_OK); }
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	this->d3dDevice->CreateInputLayout(ied, ARRAYSIZE(ied), vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize(), inputLayout.GetAddressOf());
	vertexShaderBlob->Release();
	stride = sizeof(Vertex);
	offset = 0;
	}
	//Constant buffer to manage the position of the character
	{
		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.ByteWidth = sizeof(Constants) + 0xf & 0xfffffff0;
		constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
		HRESULT hResult = d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, constantBuffer.GetAddressOf());
		assert(SUCCEEDED(hResult));
	}
	//Constant buffer to control the projection
	{
		D3D11_BUFFER_DESC projectionBufferDesc = {};
		projectionBufferDesc.ByteWidth = sizeof(ProjectionBuffer);
		projectionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		projectionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		projectionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
		HRESULT hResult = d3dDevice->CreateBuffer(&projectionBufferDesc, nullptr, projectionBuffer.GetAddressOf());
		assert(SUCCEEDED(hResult));
	}
	//Creating a constant buffer for the enemy.
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(Constants);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = d3dDevice->CreateBuffer(&bufferDesc, nullptr, enemyConstantBuffer.GetAddressOf());
		assert(SUCCEEDED(hResult));
	}
	worldMatrix = XMMatrixIdentity();
	static XMVECTOR eyePos = XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f);
	static XMVECTOR lookAtPos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	static XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	viewMatrix = XMMatrixLookAtLH(eyePos, lookAtPos, upVector);
	float fovDegrees = 90.0f;
	float fovRadians = (fovDegrees / 360.0f) * XM_2PI;
	float aspectRatio = (float)this->width / (float)this->height;
	float nearZ = 0.1f;
	float farZ = 100.0f;
	projMatrix = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
	return true;
}