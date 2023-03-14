#include "Sprite.h"
#include "Character.h"
#include "Enemy.h"

ID3D11ShaderResourceView* Sprite::LoadTexture(
	PCWSTR uri,
	UINT destinationWidth,
	UINT destinationHeight)
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	this->factory = NULL;
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICStream* pStream = NULL;
	IWICFormatConverter* pConverter = NULL;
	IWICBitmapScaler* pScaler = NULL;

	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		_uuidof(IWICImagingFactory),
		(LPVOID*)&factory);

	if (SUCCEEDED(hr)) {
		hr = this->factory->CreateDecoderFromFilename(
			uri,
			NULL,
			GENERIC_READ,
			WICDecodeMetadataCacheOnLoad,
			&pDecoder
		);
	}
	if (SUCCEEDED(hr)) {
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr)) {
		hr = this->factory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr)) {
		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.0f,
			WICBitmapPaletteTypeMedianCut
		);
	}
	if (destinationWidth && destinationHeight) {
		if (SUCCEEDED(hr)) {
			hr = this->factory->CreateBitmapScaler(&pScaler);
		}

		if (SUCCEEDED(hr)) {
			hr = pScaler->Initialize(
				pConverter,
				destinationWidth,
				destinationHeight,
				WICBitmapInterpolationModeCubic
			);
		}
	}
	WICPixelFormatGUID pixelFormat;
	if (SUCCEEDED(hr)) {
		hr = pScaler->GetPixelFormat(&pixelFormat);
	}
	DXGI_FORMAT dxgiFormat;
	if (pixelFormat == GUID_WICPixelFormat32bppBGRA)
		dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	else if (pixelFormat == GUID_WICPixelFormat24bppBGR)
		dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	else if (pixelFormat == GUID_WICPixelFormat8bppGray)
		dxgiFormat = DXGI_FORMAT_R8_UNORM;
	else
		dxgiFormat = DXGI_FORMAT_UNKNOWN;
	UINT bufferSize = destinationWidth * destinationHeight * 4;
	std::vector<BYTE> buffer(bufferSize);
	pScaler->CopyPixels(nullptr, destinationWidth * 4, bufferSize, buffer.data());
	D3D11_TEXTURE2D_DESC textureDesc = { 0 };
	textureDesc.Width = destinationWidth;
	textureDesc.Height = destinationHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = dxgiFormat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = buffer.data();
	data.SysMemPitch = destinationWidth * 4;
	ID3D11Texture2D* texture;
	hr = this->gfx->d3dDevice->CreateTexture2D(&textureDesc, &data, &texture);
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	ID3D11ShaderResourceView* srv;
	this->gfx->d3dDevice->CreateShaderResourceView(texture, &srvDesc, &srv);
	return srv;
	srv->Release();
	texture->Release();
	pSource->Release();
	pDecoder->Release();
	factory->Release();
	CoUninitialize();
}

ID2D1Bitmap* Sprite::LoadSprite(
	PCWSTR uri,
	UINT destinationWidth,
	UINT destinationHeight,
	ID2D1Bitmap* ppBitmap)
{
	this->factory = NULL;
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICStream* pStream = NULL;
	IWICFormatConverter* pConverter = NULL;
	IWICBitmapScaler* pScaler = NULL;

	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		_uuidof(IWICImagingFactory),
		(LPVOID*)&factory);

	if (SUCCEEDED(hr)) {
		hr = this->factory->CreateDecoderFromFilename(
			uri,
			NULL,
			GENERIC_READ,
			WICDecodeMetadataCacheOnLoad,
			&pDecoder
		);
	}

	if (SUCCEEDED(hr)) {
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr)) {
		hr = this->factory->CreateFormatConverter(&pConverter);
	}

	if (SUCCEEDED(hr)) {
		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.0f,
			WICBitmapPaletteTypeMedianCut
		);
	}
	if (destinationWidth && destinationHeight) {
		if (SUCCEEDED(hr)) {
			hr = this->factory->CreateBitmapScaler(&pScaler);
		}

		if (SUCCEEDED(hr)) {
			hr = pScaler->Initialize(
				pConverter,
				destinationWidth,
				destinationHeight,
				WICBitmapInterpolationModeCubic
			);
		}

		if (SUCCEEDED(hr)) {
			hr = gfx->GetRenderTarget()->CreateBitmapFromWicBitmap(pScaler, NULL, &ppBitmap);
		}
	}
	else {
		if (SUCCEEDED(hr)) {
			hr = gfx->GetRenderTarget()->CreateBitmapFromWicBitmap(pConverter, NULL, &ppBitmap);
		}
	}
	if (pDecoder) pDecoder->Release();
	if (pSource) pSource->Release();
	if (pStream) pStream->Release();
	if (pConverter) pConverter->Release();
	if (pScaler) pScaler->Release();
	return ppBitmap;
	ppBitmap = NULL;
}

void Sprite::Draw(ID2D1Bitmap* bitmap, int index, float x, float y, int spriteWidth, int spriteHeight, bool flip) {
	int spritesAcross = (bitmap->GetSize().width / spriteWidth);
	D2D1_RECT_F src = D2D1::RectF(
		(float)((index % spritesAcross) * spriteWidth),
		(float)((index / spritesAcross) * spriteHeight),
		(float)(((index % spritesAcross) * spriteWidth) + spriteWidth),
		(float)(((index / spritesAcross) * spriteHeight) + spriteHeight)
	);
	D2D1_RECT_F dest = D2D1::RectF(
		x, y,
		x + spriteWidth,
		y + spriteHeight
	);
	if (flip) {
		D2D1::Matrix3x2F flip = D2D1::Matrix3x2F::Scale(-1.0f, 1.0f, D2D1::Point2F(x + spriteWidth / 2, y + spriteHeight / 2));
		dest.left = x + spriteWidth;
		dest.right = x;
		D2D1_LAYER_PARAMETERS params;
		params.contentBounds = dest;
		params.geometricMask = NULL;
		params.maskAntialiasMode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
		params.maskTransform = flip;
		params.opacity = 1.0f;
		params.opacityBrush = NULL;
		params.layerOptions = D2D1_LAYER_OPTIONS_NONE;
		this->gfx->GetRenderTarget()->PushLayer(params, NULL);
		this->gfx->GetRenderTarget()->SetTransform(flip);
	}
	this->gfx->GetRenderTarget()->DrawBitmap(
		bitmap,
		dest,
		1.0f,
		D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
		src
	);
	if (flip) {
		this->gfx->GetRenderTarget()->PopLayer();
		this->gfx->GetRenderTarget()->SetTransform(D2D1::IdentityMatrix());
	}
}

std::vector<ID3D11ShaderResourceView*> Sprite::LoadFromDir(std::string pathName,
	UINT destinationWidth,
	UINT destinationHeight)
{
	std::vector<ID3D11ShaderResourceView*> images;
	std::string searchStr = pathName + "\\*.*";
	std::wstring search_path = std::wstring(searchStr.begin(), searchStr.end());
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				ID3D11ShaderResourceView* image;
				image = this->LoadTexture((std::wstring(pathName.begin(), pathName.end()) + L"\\" + fd.cFileName).c_str(),
					destinationWidth, destinationHeight);
				images.push_back(image);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return images;
}

void Sprite::DrawTexture(ID3D11ShaderResourceView* image, float x, float y, bool flip) {
	ID3D11Texture2D* pTextureInterface = 0;
	ID3D11Resource* res;
	image->GetResource(&res);
	res->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
	D3D11_TEXTURE2D_DESC desc;
	pTextureInterface->GetDesc(&desc);
	int spriteWidth = desc.Width;
	int spriteHeight = desc.Height;
	pTextureInterface->Release();
	res->Release();
	D3D11_RECT dest = {
	x, y,
	x + spriteWidth,
	y + spriteHeight
	};
	D3D11_RECT src = {
		0, 0,
		spriteWidth,
		spriteHeight
	};
	float depth = float((600 - y) / 600);
	if (!flip)
		this->spriteBatch->Draw(image, dest, &src, DirectX::Colors::White, 0.0f,
			DirectX::XMFLOAT2(spriteWidth / 2, spriteHeight / 2),
			DirectX::DX11::SpriteEffects::SpriteEffects_None, depth);
	else
		this->spriteBatch->Draw(image, dest, &src, DirectX::Colors::White, 0.0f,
			DirectX::XMFLOAT2(spriteWidth / 2, spriteHeight / 2),
			DirectX::DX11::SpriteEffects::SpriteEffects_FlipHorizontally, depth);
}

uint8_t Sprite::CheckCollision(Character* character, Enemy* enemy) {
	float dx = character->x - enemy->x;
	float dy = character->y - enemy->y;
	float radius = (character->width + enemy->width) * 3;
	if ((radius * radius) > (dx * dx + dy * dy)) return 1;
	return 0;
}