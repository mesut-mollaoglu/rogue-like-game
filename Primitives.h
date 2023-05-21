#pragma once
#include "SaveSystem.h"

class PrimitiveShapes {
public:
	static inline void SetDrawMode(std::string drawMode) {
		for (Graphics::Topology t : Graphics::nDrawModes)
			if (strcmp(drawMode.c_str(), t.sDrawMode.c_str()) == 0)
				Graphics::d3dDeviceContext->IASetPrimitiveTopology(t.tDrawMode);
	}
	static void SetSprite(ID3D11ShaderResourceView* nImage = nullptr) {
		ID3D11ShaderResourceView* nFrame = (nImage == nullptr) ? Sprite::sNullShaderResource : nImage;
		Graphics::d3dDeviceContext->PSSetShaderResources(0, 1, &nFrame);
	}
	static std::vector<Graphics::Vertex> GetRotatedVertex(float width, float height, float zAxis, float fAngle, float nAspect) {
		std::vector<Graphics::Vertex> vertexData;
		vertexData.push_back(Graphics::Vertex{ XMFLOAT3((-width * cos(fAngle) + height * sin(fAngle)) * nAspect, (-width * sin(fAngle) - height * cos(fAngle)) * nAspect, zAxis), XMFLOAT2(0, 1) });
		vertexData.push_back(Graphics::Vertex{ XMFLOAT3((-width * cos(fAngle) - height * sin(fAngle)) * nAspect, (-width * sin(fAngle) + height * cos(fAngle)) * nAspect, zAxis), XMFLOAT2(0, 0) });
		vertexData.push_back(Graphics::Vertex{ XMFLOAT3((width * cos(fAngle) - height * sin(fAngle)) * nAspect, (width * sin(fAngle) + height * cos(fAngle)) * nAspect, zAxis), XMFLOAT2(1, 0)});
		vertexData.push_back(Graphics::Vertex{ XMFLOAT3((width * cos(fAngle) + height * sin(fAngle)) * nAspect, (width * sin(fAngle) - height * cos(fAngle)) * nAspect, zAxis), XMFLOAT2(1, 1)});
		return vertexData;
	}
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
		void SetConstant(Math::float2 fPosition) {
			Graphics::SetConstantValues<Graphics::Constants>(this->cBuffer.Get(), { XMFLOAT2{(fPosition.x - Graphics::GetEyeDistance().x) / Structures::Window::GetWidth(),
				(fPosition.y - Graphics::GetEyeDistance().y) / Structures::Window::GetHeight()}, XMFLOAT2{(nHorizontal == FlipHorizontal::NormalHorizontal) ? 1.0f : -1.0f,
				(nVertical == FlipVertical::NormalVertical) ? 1.0f : -1.0f}, XMFLOAT4{fColor.r, fColor.g, fColor.b, fColor.a} });
		}
		void Draw() {
			Graphics::d3dDeviceContext->VSSetConstantBuffers(0, 1, cBuffer.GetAddressOf());
			Graphics::d3dDeviceContext->PSSetConstantBuffers(0, 1, cBuffer.GetAddressOf());
			Graphics::d3dDeviceContext->IASetIndexBuffer(iBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			Graphics::d3dDeviceContext->IASetVertexBuffers(0, 1, vBuffer.GetAddressOf(), &Graphics::stride, &Graphics::offset);
			Graphics::d3dDeviceContext->DrawIndexed(iData.size(), 0, 0);
		}
	}Primitive;
	typedef struct Rect {
		Primitive* pShape;
		int32_t width;
		int32_t height;
		float zAxis;
		float nAspect;
		float fAngle;
		Rect() = default;
		Rect(Math::int2 fSize, Structures::Color color, float zAxis = 0, float fAngle = 0) {
			pShape = new Primitive(4, color);
			this->zAxis = zAxis;
			width = static_cast<int32_t>(fSize.x);
			height = static_cast<int32_t>(fSize.y);
			int nWidth = Structures::Window::GetWidth();
			int nHeight = Structures::Window::GetHeight();
			float nHypo = Math::sqrt(nWidth * nWidth + nHeight * nHeight);
			this->fAngle = fAngle;
			nAspect = Graphics::GetEyeDistance().z / nHypo;
			pShape->vData = PrimitiveShapes::GetRotatedVertex(width, height, this->zAxis, fAngle, nAspect);
			pShape->CreateIndex();
			Graphics::CreateConstantBuffer<Graphics::Constants>(pShape->cBuffer);
			Graphics::CreateVertexBuffer(pShape->vBuffer, pShape->vData);
			Graphics::CreateIndexBuffer(pShape->iBuffer, pShape->iData);
			SetPosition(Math::float2());
		}
		void Rotate(float angle) {
			this->fAngle += angle;
			std::vector<Graphics::Vertex> vertexData = PrimitiveShapes::GetRotatedVertex(width, height, this->zAxis, fAngle, nAspect);
			Graphics::SetVertexValues(pShape->vBuffer, vertexData);
		}
		void SetRotation(float angle) {
			float fRotation = angle - fAngle;
			if(fRotation != 0)
				Rotate(fRotation);
		}
		void SetPosition(Math::float2 fPosition) {
			pShape->SetConstant(fPosition);
		}
		void Draw() {
			PrimitiveShapes::SetDrawMode("TriangleList");
			PrimitiveShapes::SetSprite();
			pShape->Draw();
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
		Line(Math::float3 fStart, Math::float3 fEnd, Structures::Color color) {
			int width = Structures::Window::GetWidth();
			int height = Structures::Window::GetHeight();
			float nHypo = Math::sqrt(width * width + height * height);
			float nAspect = Graphics::GetEyeDistance().z / nHypo;
			fColor = color.Normalize();
			vData.push_back(Graphics::Vertex{ XMFLOAT3(fStart.x * nAspect, fStart.y * nAspect, fEnd.z), XMFLOAT2(1, 1) });
			vData.push_back(Graphics::Vertex{ XMFLOAT3(fEnd.x * nAspect, fEnd.y * nAspect, fEnd.z), XMFLOAT2(1, 1) });
			Graphics::CreateVertexBuffer(vBuffer, vData);
			Graphics::CreateConstantBuffer<Graphics::Constants>(cBuffer);
			vData.clear();
		}
		void Draw(Math::float2 fPosition) {
			PrimitiveShapes::SetDrawMode("LineList");
			PrimitiveShapes::SetSprite();
			Graphics::SetConstantValues<Graphics::Constants>(this->cBuffer.Get(), { XMFLOAT2{(fPosition.x - Graphics::GetEyeDistance().x) / Structures::Window::GetWidth(),
				(fPosition.y - Graphics::GetEyeDistance().y) / Structures::Window::GetHeight()}, XMFLOAT2{1, 1}, XMFLOAT4{fColor.r, fColor.g, fColor.b, fColor.a} });
			Graphics::d3dDeviceContext->VSSetConstantBuffers(0, 1, cBuffer.GetAddressOf());
			Graphics::d3dDeviceContext->PSSetConstantBuffers(0, 1, cBuffer.GetAddressOf());
			Graphics::d3dDeviceContext->IASetVertexBuffers(0, 1, vBuffer.GetAddressOf(), &Graphics::stride, &Graphics::offset);
			Graphics::d3dDeviceContext->Draw(2, 0);
		}
	}Line;
	typedef struct TexturedRect {
		Primitive* pShape;
		Math::float2 fPosition;
		float fAngle;
		float fDepth;
		TexturedRect(float fDepth = 0, float fAngle = 0) {
			fPosition = Math::float2();
			this->fAngle = fAngle;
			this->fDepth = fDepth;
			pShape = new Primitive(4, Structures::Color());
			pShape->vData = PrimitiveShapes::GetRotatedVertex(1, 1, fDepth, fAngle, 1);
			pShape->CreateIndex();
			Graphics::CreateIndexBuffer(pShape->iBuffer, pShape->iData);
			Graphics::CreateVertexBuffer(pShape->vBuffer, pShape->vData);
			Graphics::CreateConstantBuffer<Graphics::Constants>(pShape->cBuffer);
			SetAttributes(Math::float2(), fAngle);
		}
		void Rotate(float angle) {
			this->fAngle += angle;
			std::vector<Graphics::Vertex> vertexData = PrimitiveShapes::GetRotatedVertex(1, 1, fDepth, fAngle, 1);
			Graphics::SetVertexValues(pShape->vBuffer, vertexData);
		}
		void SetRotation(float angle) {
			float fRotation = angle - fAngle;
			if (fRotation != 0)
				Rotate(fRotation);
		}
		void SetAttributes(Math::float2 fPosition, float fAngle = 0) {
			this->fPosition = fPosition;
			SetRotation(fAngle);
			Graphics::SetConstantValues<Graphics::Constants>(pShape->cBuffer.Get(), { XMFLOAT2{(fPosition.x - Graphics::GetEyeDistance().x) / Structures::Window::GetWidth(),
				(fPosition.y - Graphics::GetEyeDistance().y) / Structures::Window::GetHeight()}, XMFLOAT2{(pShape->nHorizontal == FlipHorizontal::NormalHorizontal) ? 1.0f : -1.0f,
				(pShape->nVertical == FlipVertical::NormalVertical) ? 1.0f : -1.0f}, XMFLOAT4{fAngle, 0, 0, 0} });
		}
		void Draw(ID3D11ShaderResourceView* nImage, FlipHorizontal hFlip = FlipHorizontal::NormalHorizontal, FlipVertical vFlip = FlipVertical::NormalVertical) {
			pShape->nHorizontal = hFlip;
			pShape->nVertical = vFlip;
			PrimitiveShapes::SetDrawMode("TriangleList");
			PrimitiveShapes::SetSprite(nImage);
			pShape->Draw();
		}
	};
};