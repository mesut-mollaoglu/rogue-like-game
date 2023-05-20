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
		Structures::Color fColor;
		std::vector<Graphics::Vertex> vData;
		FlipHorizontal nHorizontal = FlipHorizontal::NormalHorizontal;
		FlipVertical nVertical = FlipVertical::NormalVertical;
		std::vector<DWORD> iData;
		uint32_t nIndex;
		const uint32_t nVertexCount = 4;
		const uint32_t nIndexCount = 6;
		Primitive() = default;
		Primitive(uint32_t shapeIndex, Structures::Color color) {
			nIndex = shapeIndex;
			fColor = color.Normalize();
		}
		void AddPoint(Structures::Color fColor, Math::float2 fPos) {
			vData.push_back(Graphics::Vertex{ XMFLOAT2(fPos.x, fPos.y), XMFLOAT4(fColor.r, fColor.g, fColor.a, fColor.a), XMFLOAT2(1, 1) });
			if (vData.size() == nIndex) {
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
			Graphics::SetConstantValues<Graphics::Constants>(this->cBuffer.Get(), { XMFLOAT2{(fPosition.x - Graphics::GetEyeDistance().x) / Structures::Window::GetWidth(),
				(fPosition.y - Graphics::GetEyeDistance().y) / Structures::Window::GetHeight()}, XMFLOAT2{(nHorizontal == FlipHorizontal::NormalHorizontal) ? 1.0f : -1.0f,
				(nVertical == FlipVertical::NormalVertical) ? 1.0f : -1.0f}, XMFLOAT4{fColor.r, fColor.g, fColor.b, fColor.a} });
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
		int32_t width;
		int32_t height;
		float nAspect;
		float fAngle;
		Rect() = default;
		Rect(Math::int2 fSize, Structures::Color color, float fAngle = 0) {
			pShape = new Primitive(4, color);
			width = static_cast<int32_t>(fSize.x);
			height = static_cast<int32_t>(fSize.y);
			int nWidth = Structures::Window::GetWidth();
			int nHeight = Structures::Window::GetHeight();
			float nHypo = Math::sqrt(nWidth * nWidth + nHeight * nHeight);
			this->fAngle = fAngle;
			nAspect = Graphics::GetEyeDistance().z / nHypo;
			pShape->AddPoint(color.Normalize(), Math::float2((-width * cos(fAngle) + height * sin(fAngle)) * nAspect, (-width * sin(fAngle) - height * cos(fAngle)) * nAspect));
			pShape->AddPoint(color.Normalize(), Math::float2((-width * cos(fAngle) - height * sin(fAngle)) * nAspect, (-width * sin(fAngle) + height * cos(fAngle)) * nAspect));
			pShape->AddPoint(color.Normalize(), Math::float2((width * cos(fAngle) - height * sin(fAngle)) * nAspect, (width * sin(fAngle) + height * cos(fAngle)) * nAspect));
			pShape->AddPoint(color.Normalize(), Math::float2((width * cos(fAngle) + height * sin(fAngle)) * nAspect, (width * sin(fAngle) - height * cos(fAngle)) * nAspect));
		}
		void Rotate(float angle) {
			this->fAngle += angle;
			std::vector<Graphics::Vertex> vertexData;
			vertexData.push_back(Graphics::Vertex{ XMFLOAT2((-width * cos(fAngle) + height * sin(fAngle)) * nAspect, (-width * sin(fAngle) - height * cos(fAngle)) * nAspect), XMFLOAT4(pShape->fColor.r, pShape->fColor.g, pShape->fColor.b, pShape->fColor.a), XMFLOAT2(1, 1) });
			vertexData.push_back(Graphics::Vertex{ XMFLOAT2((-width * cos(fAngle) - height * sin(fAngle)) * nAspect, (-width * sin(fAngle) + height * cos(fAngle)) * nAspect), XMFLOAT4(pShape->fColor.r, pShape->fColor.g, pShape->fColor.b, pShape->fColor.a), XMFLOAT2(1, 1) });
			vertexData.push_back(Graphics::Vertex{ XMFLOAT2((width * cos(fAngle) - height * sin(fAngle)) * nAspect, (width * sin(fAngle) + height * cos(fAngle)) * nAspect), XMFLOAT4(pShape->fColor.r, pShape->fColor.g, pShape->fColor.b, pShape->fColor.a), XMFLOAT2(1, 1) });
			vertexData.push_back(Graphics::Vertex{ XMFLOAT2((width * cos(fAngle) + height * sin(fAngle)) * nAspect, (width * sin(fAngle) - height * cos(fAngle)) * nAspect), XMFLOAT4(pShape->fColor.r, pShape->fColor.g, pShape->fColor.b, pShape->fColor.a), XMFLOAT2(1, 1) });
			Graphics::SetVertexValues(pShape->vBuffer, vertexData);
		}
		void SetRotation(float angle) {
			float fRotation = angle - fAngle;
			if(fRotation != 0)
				Rotate(fRotation);
		}
		void Draw(Math::float2 fPos) {
			Graphics::d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			Graphics::d3dDeviceContext->PSSetShaderResources(0, 1, &Sprite::sNullShaderResource);
			pShape->Draw(fPos);
		}
	}Rect;
	typedef struct Line {
		ComPtr<ID3D11Buffer> vBuffer;
		ComPtr<ID3D11Buffer> cBuffer;
		Structures::Color fColor;
		std::vector<Graphics::Vertex> vData;
		uint32_t nIndex;
		uint32_t nCount = 0;
		Line() = default;
		Line(Math::float2 fStart, Math::float2 fEnd, Structures::Color color) {
			int width = Structures::Window::GetWidth();
			int height = Structures::Window::GetHeight();
			float nHypo = Math::sqrt(width * width + height * height);
			float nAspect = Graphics::GetEyeDistance().z / nHypo;
			fColor = color.Normalize();
			vData.push_back(Graphics::Vertex{ XMFLOAT2(fStart.x * nAspect, fStart.y * nAspect), XMFLOAT4(fColor.r, fColor.g, fColor.a, fColor.a), XMFLOAT2(1, 1) });
			vData.push_back(Graphics::Vertex{ XMFLOAT2(fEnd.x * nAspect, fEnd.y * nAspect), XMFLOAT4(fColor.r, fColor.g, fColor.a, fColor.a), XMFLOAT2(1, 1) });
			Graphics::CreateVertexBuffer(vBuffer, vData.data(), sizeof(Graphics::Vertex) * vData.size());
			Graphics::CreateConstantBuffer<Graphics::Constants>(cBuffer);
		}
		void Draw(Math::float2 fPosition) {
			Graphics::d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			Graphics::d3dDeviceContext->PSSetShaderResources(0, 1, &Sprite::sNullShaderResource);
			Graphics::SetConstantValues<Graphics::Constants>(this->cBuffer.Get(), { XMFLOAT2{(fPosition.x - Graphics::GetEyeDistance().x) / Structures::Window::GetWidth(),
				(fPosition.y - Graphics::GetEyeDistance().y) / Structures::Window::GetHeight()}, XMFLOAT2{1, 1}, XMFLOAT4{fColor.r, fColor.g, fColor.b, fColor.a} });
			Graphics::d3dDeviceContext->VSSetConstantBuffers(0, 1, cBuffer.GetAddressOf());
			Graphics::d3dDeviceContext->PSSetConstantBuffers(0, 1, cBuffer.GetAddressOf());
			Graphics::d3dDeviceContext->IASetVertexBuffers(0, 1, vBuffer.GetAddressOf(), &Graphics::stride,
				&Graphics::offset);
			Graphics::d3dDeviceContext->Draw(2, 0);
		}
	}Line;
	typedef struct TexturedRect {
		Primitive* pShape;
		TexturedRect() {
			pShape = new Primitive(4, Structures::Color());
			pShape->AddPoint(Structures::Color(), Math::float2(-1, -1));
			pShape->AddPoint(Structures::Color(), Math::float2(-1, 1));
			pShape->AddPoint(Structures::Color(), Math::float2(1, 1));
			pShape->AddPoint(Structures::Color(), Math::float2(1, -1));
		}
		void Draw(ID3D11ShaderResourceView* nImage, Math::float2 fPos, FlipHorizontal hFlip = FlipHorizontal::NormalHorizontal, FlipVertical vFlip = FlipVertical::NormalVertical) {
			pShape->nHorizontal = hFlip;
			pShape->nVertical = vFlip;
			Graphics::d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			Graphics::d3dDeviceContext->PSSetShaderResources(0, 1, &nImage);
			pShape->Draw(fPos);
		}
	};
};