#pragma once
#include "Graphics.h"
#include <vector>
#include <wrl/client.h>
#include <unordered_map>
#include <functional>
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

class Enemy;
class Character;

class Sprite {
public:
	static ID3D11ShaderResourceView* sNullShaderResource;
	static ID3D11ShaderResourceView* LoadTexture(
		PCWSTR uri,
		UINT destinationWidth,
		UINT destinationHeight);
	static ID2D1Bitmap* LoadSprite(
		PCWSTR uri,
		UINT destinationWidth,
		UINT destinationHeight,
		ID2D1Bitmap* ppBitmap);
	static std::vector<ID3D11ShaderResourceView*> LoadFromDir(std::string pathName,
		UINT destinationWidth,
		UINT destinationHeight);
	static IWICImagingFactory* factory;
	static uint8_t CheckCollision(Character* character, Enemy* enemy);
};