#include "PixelSprite.h"

PixelSprite::PixelSprite(int width, int height)
{
	m_texture = oxygine::IVideoDriver::instance->createTexture();
	m_texture->init(width, height, oxygine::TF_R8G8B8A8);

	oxygine::AnimationFrame textureFrame(m_texture);
	setAnimFrame(textureFrame);

	m_pixelBuffer.resize(width * height * getBytesPerPixel(m_texture->getFormat()), 0);
}

int PixelSprite::GetWidth()
{
	return m_texture->getWidth();
}

int PixelSprite::GetHeight()
{
	return m_texture->getHeight();
}

void PixelSprite::SetPixel(int x, int y, const oxygine::Color & color)
{
	oxygine::Pixel p;
	p.r = color.r;
	p.g = color.g;
	p.b = color.b;
	p.a = color.a;

	oxygine::ImageData d(m_texture->getWidth(), m_texture->getHeight(),
		m_texture->getWidth() * getBytesPerPixel(m_texture->getFormat()), m_texture->getFormat(), m_pixelBuffer.data());

	oxygine::PixelR8G8B8A8().setPixel(d.getPixelPtr(x, y), p);
}

oxygine::Color PixelSprite::GetPixel(int x, int y)
{
	oxygine::Pixel p;
	oxygine::PixelR8G8B8A8().getPixel(m_pixelBuffer.data(), p);

	return oxygine::Color(p.r, p.g, p.b, p.a);
}

void PixelSprite::ApplyPixelBuffer()
{
	oxygine::ImageData data(m_texture->getWidth(), m_texture->getHeight(), 
		getBytesPerPixel(oxygine::TF_R8G8B8A8) * m_texture->getWidth(), 
		oxygine::TF_R8G8B8A8, m_pixelBuffer.data());

	m_texture->updateRegion(0, 0, data);
}
