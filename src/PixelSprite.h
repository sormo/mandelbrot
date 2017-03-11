#pragma once

#include "oxygine-framework.h"

class PixelSprite : public oxygine::Sprite
{
public:
	PixelSprite(int width, int height);

	void SetPixel(int x, int y, const oxygine::Color & color);
	oxygine::Color GetPixel(int x, int y);

	void ApplyPixelBuffer();

	int GetWidth();
	int GetHeight();

private:
	oxygine::spNativeTexture m_texture;

	std::vector<unsigned char> m_pixelBuffer;
};

DECLARE_SMART(PixelSprite, spPixelSprite);
