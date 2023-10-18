#include <windows.h>
#include <d3d11_1.h>
#pragma comment(lib,"d3d11.lib")
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include <wrl/client.h>
#include <DirectXMath.h>
#include <wincodec.h>
#include <memory>

#include "Math.h"
#include <assert.h>
#include <iostream>
#include <string>
#include <vector>

#include "stb_image.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

template <class T> inline void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

struct Window {
	static float width;
	static float height;
	static HWND windowHandle;
	static MSG windowMessage;
	static WPARAM wParam;
	static LPARAM lParam;
	static std::wstring className;
	static std::wstring windowName;
};

struct Camera {
	static Vec3f Position;
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
	static ID3D11Buffer* projBuffer;
};

namespace Structures {
	typedef struct Color {
		float r, g, b, a;
		bool operator!=(Structures::Color col) { return (r != col.r || g != col.g || b != col.b || a != col.a); }
		bool operator==(Structures::Color col) { return (r == col.r && g == col.g && b == col.b && a == col.a); }
	};
	typedef struct Vertex {
		float x, y, z, u, v;
	}Vertex;
	typedef struct Instance {
		float x, y, z;
	};
	typedef struct Projection {
		XMMATRIX projMatrix;
		XMMATRIX worldMatrix;
		XMMATRIX viewMatrix;
	}Projection;
	typedef struct Constants
	{
		Vec2f pos;
		Vec2f flipScale;
		Structures::Color color;
	}Constants;
	typedef struct Health {
		float fMax;
		float fMin;
		float fCurrent;
		float fPadding;
	};
	typedef struct Texture {
		ID3D11ShaderResourceView* texture;
		float width;
		float height;
		void Free() {
			SafeRelease(&texture);
		}
		~Texture() {}
	};
};

class Graphics {
public:
	typedef struct Topology {
		D3D11_PRIMITIVE_TOPOLOGY tDrawMode;
		std::string sDrawMode;
	}Topology;
	typedef struct Format {
		GUID wicFormat;
		DXGI_FORMAT dxgiFormat;
	};
	static inline DXGI_FORMAT FindFormat(GUID guidFormat) {
		for (Graphics::Format formats : Graphics::formatTable)
			if (formats.wicFormat == guidFormat)
				return formats.dxgiFormat;
	}
	static inline void SetDrawMode(std::string drawMode) {
		for (Graphics::Topology t : Graphics::nDrawModes)
			if (strcmp(drawMode.c_str(), t.sDrawMode.c_str()) == 0)
				Graphics::deviceContext->IASetPrimitiveTopology(t.tDrawMode);
	}
	static Graphics::Format formatTable[3];
	static Graphics::Topology nDrawModes[5];
	static uint32_t stride;
	static uint32_t offset;
	static ComPtr<ID3D11BlendState> blendState;
	static ComPtr<ID3D11SamplerState> samplerState;
    static ComPtr<ID3D11Device1> device;
    static ComPtr<ID3D11DeviceContext1> deviceContext;
	static ComPtr<IDXGISwapChain1> swapChain;
	static ComPtr<ID3D11RenderTargetView> renderTarget;
	static ComPtr<ID3D11InputLayout> inputLayout;
	static ComPtr<ID3D11RasterizerState> rasterizer;
	static ComPtr<ID3D11PixelShader> pixelShader;
	static ComPtr<ID3D11VertexShader> vertexShader;
	static bool InitDevices();
	static bool InitSwapChain();
	static bool InitRenderTarget();
	static bool InitSampler();
	static void InitRasterizer();
	static void InitBlendState();
	static bool CreateVertexShader(ComPtr<ID3D11VertexShader>& shader, std::wstring shaderFile, std::string function = "main");
	static bool CreatePixelShader(ComPtr<ID3D11PixelShader>& shader, std::wstring shaderFile, std::string function = "main");
	static void InitCamera(Vec3f fPos);
	static void UpdateCamera(Vec3f fPos);
	template <class T> static inline void CreateVertexBuffer(ID3D11Buffer* &vertexBuffer, std::vector<T> data) {
		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.ByteWidth = sizeof(T) * data.size();
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;
		HRESULT hr = Graphics::device->CreateBuffer(&vertexBufferDesc, nullptr, &vertexBuffer);
		MapVertexBuffer(vertexBuffer, data);
		assert(SUCCEEDED(hr));
	}
	static inline void CreateIndexBuffer(ID3D11Buffer* &indexBuffer, std::vector<DWORD> indices) {
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = sizeof(DWORD) * indices.size();
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		HRESULT hr = Graphics::device->CreateBuffer(&bufferDesc, nullptr, &indexBuffer);
		MapIndexBuffer(indexBuffer, indices);
		assert(SUCCEEDED(hr));
	}
	static inline void MapIndexBuffer(ID3D11Buffer*& indexBuffer, std::vector<DWORD> indices) {
		D3D11_MAPPED_SUBRESOURCE resource;
		Graphics::deviceContext->Map(indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		DWORD* data = (DWORD*)resource.pData;
		for (DWORD &index : indices) {
			*data = index;
			data++;
		}
		Graphics::deviceContext->Unmap(indexBuffer, 0);
	}
	template <class T> static inline void CreateConstantBuffer(ID3D11Buffer* &constantBuffer) {
		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.ByteWidth = sizeof(T) + 0xf & 0xfffffff0;
		constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		HRESULT hResult = Graphics::device->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);
		assert(SUCCEEDED(hResult));
	}
	template <class T> static inline void MapVertexBuffer(ID3D11Buffer* &vertexBuffer, std::vector<T> data) {
		D3D11_MAPPED_SUBRESOURCE resource;
		Graphics::deviceContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		T* vertexData = (T*)resource.pData;
		for (T vertex : data) {
			memcpy(vertexData, &vertex, sizeof(vertex));
			vertexData++;
		}
		Graphics::deviceContext->Unmap(vertexBuffer, 0);
	}
	template <class T> static inline void MapConstantBuffer(ID3D11Buffer* &constantBuffer, T data) {
		D3D11_MAPPED_SUBRESOURCE resource;
		Graphics::deviceContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		CopyMemory(resource.pData, &data, sizeof(data));
		Graphics::deviceContext->Unmap(constantBuffer, 0);
	}
	static inline Structures::Texture LoadTexture(std::string path) {
		Structures::Texture tex;
		int texNumChannels, texWidth, texHeight;
		int texForceNumChannels = 4;
		unsigned char* testTextureBytes = stbi_load(path.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		tex.width = static_cast<float>(texWidth);
		tex.height = static_cast<float>(texHeight);
		assert(testTextureBytes);
		int texBytesPerRow = 4 * tex.width;
		ID3D11ShaderResourceView* textureView;
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = tex.width;
		textureDesc.Height = tex.height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
		textureSubresourceData.pSysMem = testTextureBytes;
		textureSubresourceData.SysMemPitch = texBytesPerRow;
		ID3D11Texture2D* texture;
		device->CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);
		device->CreateShaderResourceView(texture, nullptr, &tex.texture);
		SafeRelease(&texture);
		free(testTextureBytes);
		return tex;
	}
	static inline std::vector<Structures::Texture> LoadFromDir(std::string pathName){
		std::vector<Structures::Texture> images;
		std::string searchStr = pathName + "\\*.*";
		std::wstring search_path = std::wstring(searchStr.begin(), searchStr.end());
		WIN32_FIND_DATA fd;
		HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					std::wstring string(fd.cFileName);
					images.emplace_back(LoadTexture(pathName + "\\" + std::string(string.begin(), string.end())));
				}
			} while (::FindNextFile(hFind, &fd));
			::FindClose(hFind);
		}
		return images;
	}
	static inline void SetStates(){
		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		Graphics::CreatePixelShader(pixelShader, L"Shaders\\shaders.hlsl", "ps_main");
		Graphics::CreateVertexShader(vertexShader, L"Shaders\\shaders.hlsl", "vs_main");
		deviceContext->RSSetState(rasterizer.Get());
		deviceContext->OMSetRenderTargets(1, renderTarget.GetAddressOf(), nullptr);
		deviceContext->OMSetBlendState(blendState.Get(), blendFactor, 0xffffffff);
		Graphics::SetDrawMode("TriangleList");
		deviceContext->IASetInputLayout(inputLayout.Get());
		deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
		deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);
		deviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());
		deviceContext->VSSetConstantBuffers(1, 1, &Camera::projBuffer);
	}
	static inline void ClearAndBegin(Structures::Color color) {
		Graphics::MapConstantBuffer<Structures::Projection>(Camera::projBuffer, { Camera::projMatrix, Camera::worldMatrix, Camera::viewMatrix });
		FLOAT backgroundColor[4] = { color.r, color.g, color.b, color.a };
		Graphics::deviceContext->ClearRenderTargetView(Graphics::renderTarget.Get(), backgroundColor);
		D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)Window::width, (FLOAT)Window::height, 0.0f, 1.0f };
		Graphics::deviceContext->RSSetViewports(1, &viewport);
	}
	static inline void End() {
		Graphics::swapChain->Present(1, 0);
	}
};