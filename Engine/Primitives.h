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

class Data { public: static FontData* fontData; };

inline void LoadFontData(float width = 1024) {
   Data::fontData = new FontData[95];
   std::ifstream fin;
   int i;
   char temp;
   fin.open("Assets\\Font\\font01.txt");
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
	   Data::fontData[i].right /= width;
	   Data::fontData[i].left /= width;
   }
   fin.close();
}

inline void FreeFontData() {
   delete[] Data::fontData;
}

inline std::vector<Structures::Vertex> GetRotatedVertex(float halfWidth, float halfHeight, float depth, float fAngle, float nAspect) {
	std::vector<Structures::Vertex> vertexData;
	vertexData.emplace_back(Structures::Vertex{ (-halfWidth * cos(fAngle) - halfHeight * sin(fAngle)) * nAspect, (-halfWidth * sin(fAngle) + halfHeight * cos(fAngle)), depth, 0, 0 });
	vertexData.emplace_back(Structures::Vertex{ (halfWidth * cos(fAngle) + halfHeight * sin(fAngle)) * nAspect, (halfWidth * sin(fAngle) - halfHeight * cos(fAngle)), depth, 1, 1 });
	vertexData.emplace_back(Structures::Vertex{ (-halfWidth * cos(fAngle) + halfHeight * sin(fAngle)) * nAspect, (-halfWidth * sin(fAngle) - halfHeight * cos(fAngle)), depth, 0, 1 });
	vertexData.emplace_back(Structures::Vertex{ (halfWidth * cos(fAngle) - halfHeight * sin(fAngle)) * nAspect, (halfWidth * sin(fAngle) + halfHeight * cos(fAngle)), depth, 1, 0 });
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
   Vec2f position;
   Vec2i nSize;
   float rotation;
   Rect() = default;
   Rect(Vec2i size, float fAngle = 0.0f) {
	   std::vector<DWORD> indices = { 0, 1, 2, 0, 3, 1 };
	   nSize = size;
	   rotation = fAngle;
	   Graphics::CreateVertexBuffer(vertexBuffer, GetRotatedVertex(nSize.x, nSize.y, 0.0f, rotation, Window::height/Window::width));
	   Graphics::CreateIndexBuffer(indexBuffer, indices);
	   Graphics::CreateConstantBuffer<Structures::Constants>(constantBuffer);
   }
   void Rotate(float angle) {
	   rotation += angle;
	   Graphics::MapVertexBuffer(vertexBuffer, GetRotatedVertex(nSize.x, nSize.y, 0.0f, rotation, Window::height / Window::width));
   }
   void SetRotation(float fAngle) {
	   float rotation = fAngle - rotation;
	   if (rotation != 0)
		   Rotate(rotation);
   }
   void SetPosition(Vec2f fPos) {
	   position = fPos;
	   Graphics::MapConstantBuffer<Structures::Constants>(constantBuffer, { position, {0, 0}, color });
   }
   void SetColor(Structures::Color fColor) {
	   color = fColor;
	   Graphics::MapConstantBuffer<Structures::Constants>(constantBuffer, { position, {0, 0}, color });
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
   Structures::Texture texture;
   float sizeMultiplier;
   Sprite() {
	   sizeMultiplier = 1.f;
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
   void SetTexture(Structures::Texture tex, float multiplier = 1.0f) {
	   if (texture.width != tex.width || texture.height != tex.height || multiplier != sizeMultiplier) {
		   Graphics::MapVertexBuffer(vertexBuffer, GetRotatedVertex(multiplier * tex.width / tex.height, multiplier, 0.0f, 0.0f, 1.0f));
		   sizeMultiplier = multiplier;
	   }
	   texture = tex;
   }
   void Prepare(FlipHorizontal mHorizontalFlip = FlipHorizontal::NormalHorizontal, FlipVertical mVerticalFlip = FlipVertical::NormalVertical, ComPtr<ID3D11PixelShader> pixelShader = Graphics::pixelShader) {
	   Graphics::MapConstantBuffer<Structures::Constants>(constantBuffer, { {position.x / Window::width, position.y / Window::height}, {(mHorizontalFlip == FlipHorizontal::NormalHorizontal) ? 1.0f : -1.0f, (mVerticalFlip == FlipVertical::NormalVertical) ? 1.0f : -1.0f}, {0, 0, 0, 0} });
	   Graphics::deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);
	   Graphics::deviceContext->PSSetShaderResources(0, 1, &texture.texture);
	   Graphics::deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	   Graphics::deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);
	   Graphics::deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	   Graphics::deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &Graphics::stride, &Graphics::offset);
   }
   void Draw(FlipHorizontal mHorizontalFlip = FlipHorizontal::NormalHorizontal, FlipVertical mVerticalFlip = FlipVertical::NormalVertical, ComPtr<ID3D11PixelShader> pixelShader = Graphics::pixelShader) {
	   Prepare(mHorizontalFlip, mVerticalFlip, pixelShader);
	   Graphics::deviceContext->DrawIndexed(6, 0, 0);
   }
   void Free() {
	   SafeRelease(&constantBuffer);
	   SafeRelease(&vertexBuffer);
	   SafeRelease(&indexBuffer);
	   texture.Free();
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
	   SafeRelease(shader.ReleaseAndGetAddressOf());
	   shader.Reset();
	   sprite.Free();
	   SafeRelease(&healthBuffer);
   }
};

typedef struct Text {
   ComPtr<ID3D11PixelShader> shader;
   ID3D11Buffer* vertexBuffer;
   ID3D11Buffer* indexBuffer;
   ID3D11Buffer* constantBuffer;
   Structures::Texture mFontTex;
   int indexSize;
   bool bRender;
   Structures::Color color;
   Vec2f position;
   std::string mText;
   float multiplier;
   Text() = default;
   Text(std::string sText, Vec2f pos = { 0, 0 }, float sizeMultiplier = 0.125f, Structures::Color fColor = { 1, 1, 1, 1 }) {
	   bRender = true;
	   position = pos;
	   mText = sText;
	   multiplier = sizeMultiplier;
	   mFontTex = Graphics::LoadTexture("Assets\\Font\\font01.tga");
	   if (Data::fontData == NULL) LoadFontData(mFontTex.width);
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
	   if (countLetters(text) > countLetters(mText)) {
		   bRender = false;
		   mText = text;
		   SafeRelease(&vertexBuffer);
		   SafeRelease(&indexBuffer);
		   Graphics::CreateVertexBuffer(vertexBuffer, BuildVertex(text));
		   Graphics::CreateIndexBuffer(indexBuffer, BuildIndex(text));
		   bRender = true;
	   }
	   else {
		   mText = text;
		   Graphics::MapVertexBuffer(vertexBuffer, BuildVertex(text));
		   Graphics::MapIndexBuffer(indexBuffer, BuildIndex(text));
	   }
   }
   std::size_t countLetters(std::string text) {
	   std::size_t count = 0;
	   for (char c : text)
		   if (c != ' ')
			   count++;
	   return count;
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
			   vertices.emplace_back(Structures::Vertex{ xPos * multiplier, yPos * multiplier, 0.f, Data::fontData[letter].left, 0.0f });
			   vertices.emplace_back(Structures::Vertex{ (xPos + Data::fontData[letter].size) * multiplier, (yPos - fontHeight) * multiplier, 0.f, Data::fontData[letter].right, 1.f });
			   vertices.emplace_back(Structures::Vertex{ xPos * multiplier, (yPos - fontHeight) * multiplier, 0.f, Data::fontData[letter].left, 1.f });
			   vertices.emplace_back(Structures::Vertex{ xPos * multiplier, yPos * multiplier, 0.f, Data::fontData[letter].left, 0.f });
			   vertices.emplace_back(Structures::Vertex{ (xPos + Data::fontData[letter].size) * multiplier, yPos * multiplier, 0.f, Data::fontData[letter].right, 0.f });
			   vertices.emplace_back(Structures::Vertex{ (xPos + Data::fontData[letter].size) * multiplier, (yPos - fontHeight) * multiplier, 0.f, Data::fontData[letter].right, 1.f });
			   xPos += Data::fontData[letter].size + 0.5f;
		   }
	   }
	   return vertices;
   }
   std::vector<DWORD> BuildIndex(std::string sText) {
	   std::vector<DWORD> indices;
	   indexSize = 6 * sText.size();
	   for (int i = 0; i < indexSize; i++) indices.emplace_back(static_cast<DWORD>(i));
	   return indices;
   }
   void DrawString() {
	   if (!bRender) return;
	   Graphics::deviceContext->PSSetShader(shader.Get(), nullptr, 0);
	   Graphics::deviceContext->PSSetShaderResources(0, 1, &mFontTex.texture);
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
		   sum += (letter == 0) ? spaceSize : (Data::fontData[letter].size + 0.5f);
	   }
	   return sum * 40.5f * multiplier;
   }
   void Free() {
	   SafeRelease(shader.ReleaseAndGetAddressOf());
	   shader.Reset();
	   SafeRelease(&constantBuffer);
	   SafeRelease(&vertexBuffer);
	   SafeRelease(&indexBuffer);
	   mFontTex.Free();
   }
};

inline bool PointInSprite(Sprite sprite, Vec2f point) {
	Vec2f nHalfSize;
	nHalfSize.x = (sprite.texture.width / sprite.texture.height)* sprite.sizeMultiplier* (Window::height / Window::width);
	nHalfSize.y = sprite.sizeMultiplier;
	Vec2f nPosition;
	nPosition.x = sprite.position.x / Window::width;
	nPosition.y = sprite.position.y / Window::height;
	Vec2f mousePosition;
	mousePosition.x = point.x / Window::width;
	mousePosition.y = point.y / Window::height;
	return mousePosition.x < nPosition.x + nHalfSize.x && mousePosition.x > nPosition.x - nHalfSize.x &&
		mousePosition.y < nPosition.y + nHalfSize.y && mousePosition.y > nPosition.y - nHalfSize.y;
}

typedef struct Button {
   Sprite sprite;
   std::function<void()> mFunction;
   Button() = default;
   float sizeMultiplier;
   Button(std::function<void()> function, std::string mSource = "", Vec2f pos = { 0, 0 }, float multiplier = 1.0f) {
		mFunction = function;
		sprite = Sprite();
		sizeMultiplier = multiplier * 0.75;
		sprite.SetTexture(Graphics::LoadTexture(mSource), sizeMultiplier);
		SetPosition(pos);
   }
   void Update() {
	   if (PointInSprite(sprite, ToScreenCoord(GetMousePos()))) {
		   sprite.SetTexture(sprite.texture, sizeMultiplier * 1.1f);
		   if (GetMouseClick() == Mouse::LeftMouseButton) mFunction();
	   }
	   else {
		   sprite.SetTexture(sprite.texture, sizeMultiplier);
	   }
   }
   void SetTexture(Structures::Texture image) {
	   sprite.SetTexture(image, sizeMultiplier);
   }
   void SetPosition(Vec2f pos) {
	   sprite.SetPosition(pos);
   }
   void Draw() {
	   sprite.Draw();
   }
   void Free() {
	   std::destroy_at(std::addressof(mFunction));
	   sprite.Free();
   }
};

struct Particle {
	Vec2f position;
	Vec2f velocity;
	Vec2f acceleration;
	bool bStopMovement;
	float distanceCovered;
	float mass;
	Vec2f gravityDirection;
	Particle() = default;
	void Update(float maxDistance) {
		if (bStopMovement) return;
		position += velocity;
		distanceCovered += velocity.GetLength();
		float angle = GetAngle(Vec2f(), gravityDirection.Normalize());
		acceleration = toVector(angle) * 9.81f;
		acceleration.y *= -1.f;
		velocity += acceleration;
		if (!bStopMovement && distanceCovered >= maxDistance) bStopMovement = true;
	}
	void AddForce(Vec2f force) {
		acceleration.x += force.x / mass;
		acceleration.y += force.y / mass;
	}
	virtual ~Particle(){}
};

typedef struct ParticleSystem {
	ID3D11Buffer* instanceBuffer;
	Sprite sprite;
	uint32_t stride;
	std::vector<Particle> particles;
	float nMaxDistance;
	ParticleSystem() = default;
	ParticleSystem(std::string particleFile, int particleCount = 5, float maxDistance = 5000.f) {
		nMaxDistance = maxDistance;
		stride = sizeof(Structures::Instance);
		sprite = Sprite();
		sprite.SetTexture(Graphics::LoadTexture(particleFile));
		sprite.SetPosition({ 0, 0 });
		particles = PopulateVector(particleCount);
		Graphics::CreateVertexBuffer(instanceBuffer, projection(particles));
	}
	void Update() {
		for (auto& particle : particles) {
			particle.Update(nMaxDistance);
			if (particle.bStopMovement) particle = initParticle();
		}
		Graphics::MapVertexBuffer(instanceBuffer, projection(particles));
	}
	std::vector<Particle> PopulateVector(int nParticleCount) {
		std::vector<Particle> vec;
		for (int i = 0; i < nParticleCount; i++)
			vec.emplace_back(initParticle());
		return vec;
	}
	Particle initParticle() {
		Particle particle = Particle();
		particle.position = sprite.position;
		particle.velocity = { float(rand() % 300 - 150), float(rand() % 300 - 150) };
		particle.gravityDirection = {2.f, -1.f};
		particle.bStopMovement = false;
		particle.mass = 2.f;
		return particle;
	}
	std::vector<Structures::Instance> projection(std::vector<Particle> particlesVec) {
		std::vector<Structures::Instance> vec;
		for (auto& particle : particlesVec)
			vec.push_back({ particle.position.x / Window::width, particle.position.y / Window::width, 0.f });
		return vec;
	}
	void Render() {
		sprite.Prepare();
		Graphics::deviceContext->IASetVertexBuffers(1, 1, &instanceBuffer, &stride, &Graphics::offset);
		Graphics::deviceContext->DrawIndexedInstanced(6, particles.size(), 0, 0, 0);
	}
	void Free() {
		particles.clear();
		SafeRelease(&instanceBuffer);
		sprite.Free();
	}
};