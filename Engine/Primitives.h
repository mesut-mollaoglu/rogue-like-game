#pragma once
#include "StateMachine.h"
#include "Math.h"
#include <fstream>

constexpr float fontHeight = 32.f;
constexpr float spaceSize = 2.f;

struct FontData {
	float left, right;
	int size;
};

class Data {public: static FontData* fontData; };

inline void LoadFontData() {
	Data::fontData = new FontData[95];
	std::ifstream fin;
	int i;
	char temp;
	fin.open("Engine\\FontFiles\\font01.txt");
	for (i = 0; i < 95; i++)
	{
		fin.get(temp);
		while (temp != ' ')
		{
			fin.get(temp);
		}
		fin.get(temp);
		while (temp != ' ')
		{
			fin.get(temp);
		}
		fin >> Data::fontData[i].left;
		fin >> Data::fontData[i].right;
		fin >> Data::fontData[i].size;
	}
	fin.close();
}

inline void FreeFontData() {
	delete[] Data::fontData;
}

inline std::vector<Structures::Vertex> GetRotatedVertex(float width, float height, float depth, float fAngle, float nAspect) {
	std::vector<Structures::Vertex> vertexData;
	vertexData.push_back(Structures::Vertex{ (-width * cos(fAngle) - height * sin(fAngle)) * nAspect, (-width * sin(fAngle) + height * cos(fAngle)) * nAspect, depth, 0, 0 });
	vertexData.push_back(Structures::Vertex{ (width * cos(fAngle) + height * sin(fAngle)) * nAspect, (width * sin(fAngle) - height * cos(fAngle)) * nAspect, depth, 1, 1 });
	vertexData.push_back(Structures::Vertex{ (-width * cos(fAngle) + height * sin(fAngle)) * nAspect, (-width * sin(fAngle) - height * cos(fAngle)) * nAspect, depth, 0, 1 });
	vertexData.push_back(Structures::Vertex{ (width * cos(fAngle) - height * sin(fAngle)) * nAspect, (width * sin(fAngle) + height * cos(fAngle)) * nAspect, depth, 1, 0 });
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
	Vec2f fPosition;
	Vec2i nSize;
	float fRotation;
	float nAspect;
	Rect() = default;
	Rect(Vec2i size, float fAngle = 0.0f) {
		fRotation = fAngle;
		nSize = size;
		nAspect = Camera::Position.z / Sqrt(Window::width * Window::width + Window::height * Window::height);
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
	void SetPosition(Vec2f fPos) {
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
	Vec2f position;
	Structures::Texture mTexture;
	Sprite() {
		position = Vec2f();
		std::vector<DWORD> indices = { 0, 1, 2, 0, 3, 1 };
		Graphics::CreateIndexBuffer(indexBuffer, indices);
		Graphics::CreateVertexBuffer(vertexBuffer, GetRotatedVertex(1.f, 1.f, 0.0f, 0.0f, 1.0f));
		Graphics::CreateConstantBuffer<Structures::Constants>(constantBuffer);
		SetPosition(position);
	}
	void SetPosition(Vec2f pos) {
		position = pos;
		Graphics::MapConstantBuffer<Structures::Constants>(constantBuffer, { {position.x / Window::width, position.y / Window::height}, {1, 1}, {0, 0, 0, 0} });
	}
	void SetTexture(Structures::Texture tex, float sizeMultiplier = 1.0f) {
		if (mTexture.texture == nullptr || mTexture.width != tex.width || mTexture.height != tex.height)
			Graphics::MapVertexBuffer(vertexBuffer, GetRotatedVertex(sizeMultiplier * tex.width / tex.height, sizeMultiplier, 0.0f, 0.0f, 1.0f));
		mTexture = tex;
	}
	void Draw(FlipHorizontal mHorizontalFlip = FlipHorizontal::NormalHorizontal, FlipVertical mVerticalFlip = FlipVertical::NormalVertical, ComPtr<ID3D11PixelShader> pixelShader = Graphics::pixelShader) {
		Graphics::MapConstantBuffer<Structures::Constants>(constantBuffer, { {position.x / Window::width, position.y / Window::height}, {(mHorizontalFlip == FlipHorizontal::NormalHorizontal) ? 1.0f : -1.0f, (mVerticalFlip == FlipVertical::NormalVertical) ? 1.0f : -1.0f}, {0, 0, 0, 0} });
		Graphics::deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);
		Graphics::deviceContext->PSSetShaderResources(0, 1, &mTexture.texture);
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
		mTexture.Free();
	}
};

typedef struct HealthBar {
	ComPtr<ID3D11PixelShader> shader;
	ID3D11Buffer* healthBuffer;
	Sprite sprite;
	float fMax, fMin, fCurrent;
	HealthBar(float nCurrent = 100.f, float nMax = 100.f, float nMin = 0.f, Vec2f position = { 0, 0 }) {
		Graphics::CreatePixelShader(shader, L"Shaders\\shaders.hlsl", "healthbar");
		fCurrent = nCurrent;
		fMax = nMax;
		fMin = nMin;
		sprite = Sprite();
		Graphics::CreateConstantBuffer<Structures::Health>(healthBuffer);
		sprite.SetPosition(position);
	}
	void SetTexture(Structures::Texture tex, float sizeMultiplier = 1.f) {
		sprite.SetTexture(tex, sizeMultiplier);
	}
	void SetPosition(Vec2f fPosition) {
		sprite.SetPosition(fPosition);
	}
	void SetHealth(float fHealth) {
		fCurrent = Clamp(fHealth, fMin, fMax);
		Graphics::MapConstantBuffer<Structures::Health>(healthBuffer, { fMax, fMin, fCurrent, 1.0f });
	}
	void Draw() {
		Graphics::deviceContext->PSSetConstantBuffers(2, 1, &healthBuffer);
		sprite.Draw(FlipHorizontal::NormalHorizontal, FlipVertical::NormalVertical, shader);
	}
	void Free() {
		shader.Reset();
		sprite.Free();
		healthBuffer->Release();
	}
};

typedef struct Text {
	ComPtr<ID3D11PixelShader> shader;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* constantBuffer;
	ID3D11ShaderResourceView* mFontTex;
	int indexSize;
	Structures::Color color;
	Vec2f position;
	std::string mText;
	float multiplier;
	Text() = default;
	Text(std::string sText, Vec2f pos = { 0, 0 }, float sizeMultiplier = 0.125f, Structures::Color fColor = { 1, 1, 1, 1 }) {
		position = pos;
		mText = sText;
		multiplier = sizeMultiplier;
		if (Data::fontData == NULL) LoadFontData();
		mFontTex = Graphics::LoadTexture("Engine\\FontFiles\\font01.tga").texture;
		Graphics::CreateVertexBuffer(vertexBuffer, BuildVertex(sText));
		Graphics::CreateIndexBuffer(indexBuffer, BuildIndex(sText));
		Graphics::CreateConstantBuffer<Structures::Constants>(constantBuffer);
		SetPosition(pos);
		SetColor(fColor);
		Graphics::CreatePixelShader(shader, L"Shaders\\shaders.hlsl", "font_main");
	}
	void SetPosition(Vec2f pos) {
		position = pos;
		Graphics::MapConstantBuffer<Structures::Constants>(constantBuffer, { {pos.x / Window::width, pos.y / Window::height}, {1, 1 }, color });
	}
	void SetColor(Structures::Color fColor) {
		color = fColor;
		Graphics::MapConstantBuffer<Structures::Constants>(constantBuffer, { {position.x / Window::width, position.y / Window::height}, {1, 1 }, fColor });
	}
	void SetText(std::string text) {
		mText = text;
		Graphics::MapVertexBuffer(vertexBuffer, BuildVertex(text));
		Graphics::MapIndexBuffer(indexBuffer, BuildIndex(text));
	}
	std::vector<Structures::Vertex> BuildVertex(std::string sText) {
		float xPos = 0, yPos = 0;
		std::vector<Structures::Vertex> vertices;
		int numLetters = sText.size();
		int letter;
		for (int i = 0; i < numLetters; i++)
		{
			letter = ((int)sText[i]) - 32;
			if (letter == 0) {
				xPos += spaceSize;
			}
			else {
				vertices.push_back(Structures::Vertex{ xPos * multiplier, yPos * multiplier, 0.f, Data::fontData[letter].left, 0.0f });
				vertices.push_back(Structures::Vertex{ (xPos + Data::fontData[letter].size) * multiplier, (yPos - fontHeight) * multiplier, 0.f, Data::fontData[letter].right, 1.f });
				vertices.push_back(Structures::Vertex{ xPos * multiplier, (yPos - fontHeight) * multiplier, 0.f, Data::fontData[letter].left, 1.f });
				vertices.push_back(Structures::Vertex{ xPos * multiplier, yPos * multiplier, 0.f, Data::fontData[letter].left, 0.f });
				vertices.push_back(Structures::Vertex{ (xPos + Data::fontData[letter].size) * multiplier, yPos * multiplier, 0.f, Data::fontData[letter].right, 0.f });
				vertices.push_back(Structures::Vertex{ (xPos + Data::fontData[letter].size) * multiplier, (yPos - fontHeight) * multiplier, 0.f, Data::fontData[letter].right, 1.f });
				xPos += Data::fontData[letter].size;
			}
		}
		return vertices;
	}
	std::vector<DWORD> BuildIndex(std::string sText) {
		std::vector<DWORD> indices;
		indexSize = 6 * sText.size();
		for (int i = 0; i < indexSize; i++) indices.push_back(static_cast<DWORD>(i));
		return indices;
	}
	void DrawString() {
		Graphics::deviceContext->PSSetShader(shader.Get(), nullptr, 0);
		Graphics::deviceContext->PSSetShaderResources(0, 1, &mFontTex);
		Graphics::deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
		Graphics::deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);
		Graphics::deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		Graphics::deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &Graphics::stride, &Graphics::offset);
		Graphics::deviceContext->DrawIndexed(indexSize, 0, 0);
	}
	float GetStringSize() {
		float sum = 0.f;
		for (int i = 0; i < mText.size(); i++) {
			int letter = ((int)mText[i]) - 32;
			sum += (letter == 0) ? spaceSize : Data::fontData[letter].size;
		}
		return sum * 42.f * multiplier;
	}
	void Free() {
		shader.Reset();
		constantBuffer->Release();
		vertexBuffer->Release();
		indexBuffer->Release();
		mFontTex->Release();
	}
};

typedef struct Button {
	Vec2i nSize;
	Sprite mSprite;
	std::function<void()> mFunction;
	float sizeMultiplier;
	Button() = default;
	Button(std::function<void()> function, std::string mSource = "", Vec2f pos = { 0, 0 }, float multiplier = 1.0f) {
		mFunction = function;
		mSprite = Sprite();
		mSprite.SetTexture(Graphics::LoadTexture(mSource), multiplier * Window::height / Window::width);
		sizeMultiplier = multiplier;
		nSize.x = 84 * multiplier * (mSprite.mTexture.width / mSprite.mTexture.height) * (Window::height / Window::width);
		nSize.y = 63 * multiplier;
		SetPosition(pos);
	}
	bool Hover() {
		Vec2f mousePos = GetMousePos();
		float xPos = (mSprite.position.x / (Camera::Position.z * Window::width) + 1.f) * Window::width / 2.f;
		float yPos = Window::height - (mSprite.position.y / (Camera::Position.z * Window::height) + 1.f) * Window::height / 2.f;
		return (mousePos.x < xPos + nSize.x && mousePos.x > xPos - nSize.x && mousePos.y < yPos + nSize.y && mousePos.y > yPos - nSize.y);
	}
	void Update() {
		if (Hover()) {
			float multiply = sizeMultiplier * Window::height / Window::width;
			Graphics::MapVertexBuffer(mSprite.vertexBuffer, GetRotatedVertex(mSprite.mTexture.width /
				mSprite.mTexture.height * multiply * 1.1f, multiply * 1.1f, 0.f, 0.f, 1.f));
			if (isKeyPressed(VK_LBUTTON)) mFunction();
		}
		else {
			float multiply = sizeMultiplier * Window::height / Window::width;
			Graphics::MapVertexBuffer(mSprite.vertexBuffer, GetRotatedVertex(mSprite.mTexture.width /
				mSprite.mTexture.height * multiply, multiply, 0.f, 0.f, 1.f));
		}
	}
	void SetTexture(Structures::Texture image) {
		mSprite.SetTexture(image, sizeMultiplier * Window::height / Window::width);
	}
	void SetPosition(Vec2f pos) {
		mSprite.SetPosition(pos);
	}
	void Draw() {
		mSprite.Draw();
	}
	void Free() {
		std::destroy_at(std::addressof(mFunction));
		mSprite.Free();
	}
};