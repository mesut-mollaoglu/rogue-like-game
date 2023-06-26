#pragma once
#include "StateMachine.h"
#include "Math.h"

class Primitives {
public:
	static inline std::vector<Structures::Vertex> GetRotatedVertex(float width, float height, float depth, float fAngle, float nAspect) {
		std::vector<Structures::Vertex> vertexData;
		vertexData.push_back(Structures::Vertex{ (-width * cos(fAngle) - height * sin(fAngle)) * nAspect, (-width * sin(fAngle) + height * cos(fAngle)) * nAspect, depth, 0, 0});
		vertexData.push_back(Structures::Vertex{ (width * cos(fAngle) + height * sin(fAngle)) * nAspect, (width * sin(fAngle) - height * cos(fAngle)) * nAspect, depth, 1, 1});
		vertexData.push_back(Structures::Vertex{ (-width * cos(fAngle) + height * sin(fAngle)) * nAspect, (-width * sin(fAngle) - height * cos(fAngle)) * nAspect, depth, 0, 1});
		vertexData.push_back(Structures::Vertex{ (width * cos(fAngle) - height * sin(fAngle)) * nAspect, (width * sin(fAngle) + height * cos(fAngle)) * nAspect, depth, 1, 0});
		return vertexData;
	}
	enum class FlipHorizontal {
		FlippedHorizontal,
		NormalHorizontal,
	};
	enum class FlipVertical {
		FlippedVertical,
		NormalVertical,
	};
	typedef struct Rect {
		ID3D11Buffer* constantBuffer;
		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* indexBuffer;
		Structures::Color color;
		Math::Vec2f fPosition;
		Math::Vec2i nSize;
		float fRotation;
		float nAspect;
		Rect() = default;
		Rect(Math::Vec2i size, float fAngle = 0.0f) {
			fRotation = fAngle;
			nSize = size;
			nAspect = Camera::Position.z/Math::sqrt(Window::width * Window::width + Window::height * Window::height);
			std::vector<DWORD> indices = { 0, 1, 2, 0, 3, 1 };
			Graphics::CreateVertexBuffer(vertexBuffer, GetRotatedVertex(nSize.x, nSize.y, 0.0f, fRotation, nAspect));
			Graphics::CreateIndexBuffer(indexBuffer, indices);
			Graphics::CreateConstantBuffer<Structures::Constants>(constantBuffer);
		}
		void Rotate(float angle) {
			fRotation += angle;
			std::vector<Structures::Vertex> vertexData = GetRotatedVertex(nSize.x, nSize.y, 0.0f, fRotation, nAspect);
			Graphics::MapVertexBuffer(vertexBuffer, vertexData);
		}
		void SetRotation(float fAngle) {
			float rotation = fAngle - fRotation;
			if (rotation != 0)
				Rotate(rotation);
		}
		void SetPosition(Math::Vec2f fPos) {
			Graphics::MapConstantBuffer<Structures::Constants>(constantBuffer, { fPos, {0, 0}, color });
		}
		void SetColor(Structures::Color fColor) {
			Graphics::MapConstantBuffer<Structures::Constants>(constantBuffer, { fPosition, {0, 0}, fColor });
		}
		void Draw() {
			Graphics::deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
			Graphics::deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);
			Graphics::deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &Graphics::stride, &Graphics::offset);
			Graphics::deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			Graphics::deviceContext->DrawIndexed(6, 0, 0);
		}
		virtual ~Rect() {}
	}Rect;
	typedef struct Sprite {
		ID3D11Buffer* constantBuffer;
		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* indexBuffer;
		Math::Vec2f fPosition;
		float fAngle;
		float fDepth;
		FlipHorizontal mHorizontal = FlipHorizontal::NormalHorizontal;
		FlipVertical mVertical = FlipVertical::NormalVertical;
		Sprite(float fDepth = 0, float fAngle = 0) {
			fPosition = Math::Vec2f();
			std::vector<DWORD> indices = { 0, 1, 2, 0, 3, 1 };
			Graphics::CreateIndexBuffer(indexBuffer, indices);
			Graphics::CreateVertexBuffer(vertexBuffer, GetRotatedVertex(1.0f, 1.0f, fDepth, 0.0f, 1.0f));
			Graphics::CreateConstantBuffer<Structures::Constants>(constantBuffer);
			SetPosition(fPosition);
		}
		void SetPosition(Math::Vec2f fPosition) {
			this->fPosition = fPosition;
			float HorizontalFlip = (mHorizontal == FlipHorizontal::NormalHorizontal) ? 1.0f : -1.0f;
			float VerticalFlip = (mVertical == FlipVertical::NormalVertical) ? 1.0f : -1.0f;
			Graphics::MapConstantBuffer<Structures::Constants>(constantBuffer, { {fPosition.x / Window::width, fPosition.y / Window::height}, {HorizontalFlip, VerticalFlip}, {0, 0, 0, 0} });
		}
		void Draw(ID3D11ShaderResourceView* mTexture, FlipHorizontal mHorizontalFlip = FlipHorizontal::NormalHorizontal, FlipVertical mVerticalFlip = FlipVertical::NormalVertical) {
			mHorizontal = mHorizontalFlip;
			mVertical = mVerticalFlip;
			Graphics::MapConstantBuffer<Structures::Constants>(constantBuffer, { {fPosition.x / Window::width, fPosition.y / Window::height}, {(mHorizontal == FlipHorizontal::NormalHorizontal) ? 1.0f : -1.0f, (mVertical == FlipVertical::NormalVertical) ? 1.0f : -1.0f}, {0, 0, 0, 0} });
			Graphics::deviceContext->PSSetShaderResources(0, 1, &mTexture);
			Graphics::deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
			Graphics::deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);
			Graphics::deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			Graphics::deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &Graphics::stride, &Graphics::offset);
			Graphics::deviceContext->DrawIndexed(6, 0, 0);
		}
		void Free() {
			constantBuffer->Release();
			vertexBuffer->Release();
			indexBuffer->Release();
		}
	};
};