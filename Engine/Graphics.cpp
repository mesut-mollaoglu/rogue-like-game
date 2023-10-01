#include "Graphics.h"

ComPtr<ID3D11BlendState> Graphics::blendState;
ComPtr<ID3D11Device1> Graphics::device;
ComPtr<ID3D11DeviceContext1> Graphics::deviceContext;
ComPtr<ID3D11SamplerState> Graphics::samplerState;
ComPtr<IDXGISwapChain1> Graphics::swapChain;
ComPtr<ID3D11RenderTargetView> Graphics::renderTarget;
ComPtr<ID3D11VertexShader> Graphics::vertexShader;
ComPtr<ID3D11PixelShader> Graphics::pixelShader;
ComPtr<ID3D11InputLayout> Graphics::inputLayout;
ComPtr<ID3D11RasterizerState> Graphics::rasterizer;
uint32_t Graphics::stride = sizeof(Structures::Vertex);
uint32_t Graphics::offset = 0;
float Window::width;
float Window::height;
HWND Window::windowHandle;
MSG Window::windowMessage;
WPARAM Window::wParam;
LPARAM Window::lParam;
std::wstring Window::className = L"ClassName";
std::wstring Window::windowName = L"Rogue Like Game";
Vec3f Camera::Position;
XMMATRIX Camera::projMatrix, Camera::viewMatrix, Camera::worldMatrix;
XMVECTOR Camera::eyePos;
XMVECTOR Camera::lookAtPos;
XMVECTOR Camera::upVector;
float Camera::fovDegrees;
float Camera::fovRadians;
float Camera::aspectRatio;
float Camera::nearZ;
float Camera::farZ;
XMVECTOR Camera::defaultUp;
XMVECTOR Camera::defaultForward;
XMMATRIX Camera::rotationDefault;
ID3D11Buffer* Camera::projBuffer;
Graphics::Format Graphics::formatTable[3] = {
        {GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM},
        {GUID_WICPixelFormat24bppBGR, DXGI_FORMAT_B8G8R8A8_UNORM},
        {GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM},
};
Graphics::Topology Graphics::nDrawModes[5] = {
        {D3D11_PRIMITIVE_TOPOLOGY_LINELIST, "LineList"},
        {D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, "LineStrip"},
        {D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, "TriangleList"},
        {D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, "TriangleStrip"},
        {D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, "PointList"},
};

bool Graphics::InitDevices() {
    ID3D11Device* baseDevice;
    ID3D11DeviceContext* baseDeviceContext;
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG_BUILD)
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
        0, creationFlags,
        featureLevels, ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION, &baseDevice,
        0, &baseDeviceContext);
    if (FAILED(hr)) {
        MessageBoxA(0, "D3D11CreateDevice() failed", "Fatal Error", MB_OK);
        return GetLastError();
    }
    hr = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)device.GetAddressOf());
    assert(SUCCEEDED(hr));
    baseDevice->Release();
    hr = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)deviceContext.GetAddressOf());
    assert(SUCCEEDED(hr));
    baseDeviceContext->Release();
#ifdef DEBUG_BUILD
    ID3D11Debug* d3dDebug = nullptr;
    d3d11Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
    if (d3dDebug)
    {
        ID3D11InfoQueue* d3dInfoQueue = nullptr;
        if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
        {
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
            d3dInfoQueue->Release();
        }
        d3dDebug->Release();
    }
#endif
    return SUCCEEDED(hr);
}

bool Graphics::InitSwapChain() {
    IDXGIFactory2* dxgiFactory;
    IDXGIDevice1* dxgiDevice;
    HRESULT hr = device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
    assert(SUCCEEDED(hr));
    IDXGIAdapter* dxgiAdapter;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    assert(SUCCEEDED(hr));
    dxgiDevice->Release();
    DXGI_ADAPTER_DESC adapterDesc;
    dxgiAdapter->GetDesc(&adapterDesc);
    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
    assert(SUCCEEDED(hr));
    dxgiAdapter->Release();
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = 0; 
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    hr = dxgiFactory->CreateSwapChainForHwnd(device.Get(), Window::windowHandle, &swapChainDesc, 0, 0, &swapChain);
    assert(SUCCEEDED(hr));
    dxgiFactory->Release();
    return SUCCEEDED(hr);
}

bool Graphics::InitRenderTarget() {
    ID3D11Texture2D* d3d11FrameBuffer;
    HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer);
    assert(SUCCEEDED(hr));
    hr = device->CreateRenderTargetView(d3d11FrameBuffer, 0, renderTarget.GetAddressOf());
    assert(SUCCEEDED(hr));
    d3d11FrameBuffer->Release();
    return SUCCEEDED(hr);
}

bool Graphics::CreateVertexShader(ComPtr<ID3D11VertexShader>& shader, std::wstring shaderFile, std::string function) {
    ID3DBlob* vsBlob; 
    ID3DBlob* shaderCompileErrorsBlob;
    HRESULT hr = D3DCompileFromFile(shaderFile.c_str(), nullptr, nullptr, function.c_str(), "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorsBlob);
    if (FAILED(hr))
    {
        const char* errorString = NULL;
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            errorString = "Could not compile shader; file not found";
        else if (shaderCompileErrorsBlob) {
            errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
            shaderCompileErrorsBlob->Release();
        }
        MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
        return 1;
    }
    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, shader.GetAddressOf());
    assert(SUCCEEDED(hr));
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    hr = device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), inputLayout.GetAddressOf());
    assert(SUCCEEDED(hr));
    vsBlob->Release();
    return SUCCEEDED(hr);
}

bool Graphics::CreatePixelShader(ComPtr<ID3D11PixelShader>& shader, std::wstring shaderFile, std::string function) {
    ID3DBlob* psBlob;
    ID3DBlob* shaderCompileErrorsBlob;
    HRESULT hr = D3DCompileFromFile(shaderFile.c_str(), nullptr, nullptr, function.c_str(), "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob);
    if (FAILED(hr))
    {
        const char* errorString = NULL;
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            errorString = "Could not compile shader; file not found";
        else if (shaderCompileErrorsBlob) {
            errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
            shaderCompileErrorsBlob->Release();
        }
        MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
        return 1;
    }
    hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, shader.GetAddressOf());
    assert(SUCCEEDED(hr));
    psBlob->Release();
    return SUCCEEDED(hr);
}

bool Graphics::InitSampler() {
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.BorderColor[0] = 1.0f;
    samplerDesc.BorderColor[1] = 1.0f;
    samplerDesc.BorderColor[2] = 1.0f;
    samplerDesc.BorderColor[3] = 0.0f;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    HRESULT hr = device->CreateSamplerState(&samplerDesc, &samplerState);
    return SUCCEEDED(hr);
}

void Graphics::InitCamera(Vec3f fPos) {
    Graphics::CreateConstantBuffer<Structures::Projection>(Camera::projBuffer);
    Camera::defaultUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    Camera::defaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    Camera::rotationDefault = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
    Camera::upVector = XMVector3TransformCoord(Camera::defaultUp, Camera::rotationDefault);
    Camera::worldMatrix = XMMatrixIdentity();
    Camera::fovDegrees = 90.0f;
    Camera::fovRadians = (Camera::fovDegrees / 360.0f) * XM_2PI;
    Camera::aspectRatio = (float)Window::width / (float)Window::height;
    Camera::nearZ = 0.1f;
    Camera::farZ = 100.0f;
    UpdateCamera(fPos);
}

void Graphics::UpdateCamera(Vec3f fPos) {
    Camera::Position = fPos;
    Camera::eyePos = XMVectorSet(0.0f, 0.0f, -Camera::Position.z, 0.0f);
    Camera::lookAtPos = XMVector3TransformCoord(Camera::defaultForward, Camera::rotationDefault);
    Camera::lookAtPos += Camera::eyePos;
    Camera::viewMatrix = XMMatrixLookAtLH(Camera::eyePos, Camera::lookAtPos, Camera::upVector);
    Camera::projMatrix = XMMatrixPerspectiveFovLH(Camera::fovRadians, Camera::aspectRatio, Camera::nearZ,
        Camera::farZ);
    Graphics::MapConstantBuffer<Structures::Projection>(Camera::projBuffer, {Camera::projMatrix, Camera::worldMatrix, Camera::viewMatrix});
}

void Graphics::InitRasterizer() {
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = TRUE;
    device->CreateRasterizerState(&rasterizerDesc, rasterizer.GetAddressOf());
}

void Graphics::InitBlendState() {
    D3D11_BLEND_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_BLEND_DESC));
    desc.RenderTarget[0].BlendEnable = TRUE;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].RenderTargetWriteMask = 0x0f;
    device->CreateBlendState(&desc, &blendState);
}