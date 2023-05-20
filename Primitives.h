#pragma once
#include "SaveSystem.h"

class PrimitiveShapes {
public:
	enum class FlipHorizontal {
		FlippedHorizontal,
		NormalHorizontal
	};
	enum class FlipVertical {
		FlippedVertical,
		NormalVertical
	};
	typedef struct Primitive {
		ComPtr<ID3D11Buffer> vBuffer;
		ComPtr<ID3D11Buffer> iBuffer;
		ComPtr<ID3D11Buffer> cBuffer;
		Math::float3 fColor;
		std::vector<Graphics::Vertex> vData;
		FlipHorizontal nHorizontal = FlipHorizontal::NormalHorizontal;
		FlipVertical nVertical = FlipVertical::NormalVertical;
		std::vector<DWORD> iData;
		uint32_t nIndex;
		uint32_t nCount = 0;
		const uint32_t nVertexCount = 4;
		const uint32_t nIndexCount = 6;
		Primitive() = default;
		Primitive(uint32_t shapeIndex, Math::float3 color) {
			nIndex = shapeIndex;
			fColor = color;
		}
		void AddPoint(Math::float3 fColor, Math::float2 fPos) {
			vData.push_back(Graphics::Vertex{ XMFLOAT2(fPos.x, fPos.y), XMFLOAT4(fColor.x, fColor.y, fColor.z, 1.0f), XMFLOAT2(1, 1) });
			nCount++;
			if (nCount == nIndex) {
				CreateIndex();
				Graphics::CreateIndexBuffer(iBuffer, iData.data(), sizeof(DWORD) * nIndex * nIndexCount);
				Graphics::CreateVertexBuffer(vBuffer, vData.data(), sizeof(Graphics::Vertex) * vData.size());
				Graphics::CreateConstantBuffer<Graphics::Constants>(cBuffer);
			}
		}
		void CreateIndex() {
			iData.reserve(nIndex * nVertexCount);
			for (int i = 0; i < nIndex * nVertexCount; i += nVertexCount) {
				DWORD j = static_cast<DWORD>(i);
				iData.push_back(j);
				iData.push_back(j + 1);
				iData.push_back(j + 2);
				iData.push_back(j);
				iData.push_back(j + 2);
				iData.push_back(j + 3);
			}
		}
		void Draw(Math::float2 fPosition) {
			Graphics::SetConstantValues<Graphics::Constants>(this->cBuffer.Get(), { XMFLOAT2{fPosition.x / Structures::Window::GetWidth(),
				fPosition.y / Structures::Window::GetHeight()}, XMFLOAT2{(nHorizontal == FlipHorizontal::NormalHorizontal) ? 1.0f : -1.0f,
				(nVertical == FlipVertical::NormalVertical) ? 1.0f : -1.0f}, XMFLOAT4{fColor.x, fColor.y, fColor.z, 0} });
			Graphics::d3dDeviceContext->VSSetConstantBuffers(0, 1, cBuffer.GetAddressOf());
			Graphics::d3dDeviceContext->PSSetConstantBuffers(0, 1, cBuffer.GetAddressOf());
			Graphics::d3dDeviceContext->IASetIndexBuffer(iBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			Graphics::d3dDeviceContext->IASetVertexBuffers(0, 1, vBuffer.GetAddressOf(), &Graphics::stride,
				&Graphics::offset);
			Graphics::d3dDeviceContext->DrawIndexed(iData.size(), 0, 0);
		}
	}Primitive;
	typedef struct Rect {
		Primitive* pShape;
		Rect() = default;
		Rect(Math::float2 fSize, Math::float3 color) {
			pShape = new Primitive(4, color);
			int width = Structures::Window::GetWidth();
			int height = Structures::Window::GetHeight();
			float nHypo = Math::sqrt(width * width + height * height);
			float nAspect = Graphics::GetEyeDistance().z / nHypo;
			pShape->AddPoint(color, Math::float2(-fSize.x * nAspect, -fSize.y * nAspect));
			pShape->AddPoint(color, Math::float2(-fSize.x * nAspect, fSize.y * nAspect));
			pShape->AddPoint(color, Math::float2(fSize.x * nAspect, fSize.y * nAspect));
			pShape->AddPoint(color, Math::float2(fSize.x * nAspect, -fSize.y * nAspect));
		}
		void Draw(Math::float2 fPos) {
			Graphics::d3dDeviceContext->PSSetShaderResources(0, 1, &Sprite::sNullShaderResource);
			pShape->Draw(fPos);
		}
	}Rect;
	typedef struct TexturedRect {
		Primitive* pShape;
		TexturedRect() {
			pShape = new Primitive(4, Math::float3());
			pShape->AddPoint(Math::float3(0.0f, 0.0f, 0.0f), Math::float2(-1, -1));
			pShape->AddPoint(Math::float3(0.0f, 0.0f, 0.0f), Math::float2(-1, 1));
			pShape->AddPoint(Math::float3(0.0f, 0.0f, 0.0f), Math::float2(1, 1));
			pShape->AddPoint(Math::float3(0.0f, 0.0f, 0.0f), Math::float2(1, -1));
		}
		void Draw(ID3D11ShaderResourceView* nImage, Math::float2 fPos, FlipHorizontal hFlip = FlipHorizontal::NormalHorizontal, FlipVertical vFlip = FlipVertical::NormalVertical) {
			pShape->nHorizontal = hFlip;
			pShape->nVertical = vFlip;
			Graphics::d3dDeviceContext->PSSetShaderResources(0, 1, &nImage);
			pShape->Draw(fPos);
		}
	};
};