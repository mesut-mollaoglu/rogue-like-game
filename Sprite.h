#pragma once
#include "Graphics.h"
#include <vector>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <wrl/client.h>
#include <unordered_map>
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

class Enemy;
class Character;

class Sprite {
public:
	std::unique_ptr<SpriteBatch> spriteBatch;
	std::unique_ptr<SpriteFont> spriteFont;
	void Init(Graphics* graphics) {
		gfx = graphics;
		spriteBatch = std::make_unique<DirectX::SpriteBatch>(this->gfx->d3dDeviceContext.Get());
	}
	ID3D11ShaderResourceView* LoadTexture(
		PCWSTR uri,
		UINT destinationWidth,
		UINT destinationHeight);
	ID2D1Bitmap* LoadSprite(
		PCWSTR uri,
		UINT destinationWidth,
		UINT destinationHeight,
		ID2D1Bitmap* ppBitmap);
	void Draw(ID2D1Bitmap* bitmap, int index, float x, float y, int spriteWidth, int spriteHeight, bool flip);
	std::vector<ID3D11ShaderResourceView*> LoadFromDir(std::string pathName,
		UINT destinationWidth,
		UINT destinationHeight);
	void DrawTexture(ID3D11ShaderResourceView* image, float x, float y, bool flip);
	IWICImagingFactory* factory;
	uint8_t CheckCollision(Character* character, Enemy* enemy);
protected:
	Graphics* gfx;
};