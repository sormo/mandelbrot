#include "Mandelbrot.h"

Mandelbrot::Mandelbrot()
{
	InitializeColors();

	m_pixelSprite = new PixelSprite(getStage()->getWidth(), getStage()->getHeight());
	m_pixelSprite->attachTo(this);

	ResetPixelSprite();
	m_pixelSprite->ApplyPixelBuffer();

	m_camera = new Camera;
	m_camera->attachTo(this);
	m_camera->setSize(m_pixelSprite->getSize());
	m_camera->setContent(m_pixelSprite);

	m_camera->_onCameraChange = [this]()
	{
		m_scale *= m_camera->getCameraScale();

		double pixelSize = GetPixelSize();

		oxygine::Vector2 offset = m_camera->getCameraOffset();
		oxygine::VectorD2 offsetIncrement = oxygine::VectorD2(offset.x, offset.y) * pixelSize;

		m_offset += offsetIncrement;

		ResetPixelSprite();
		m_pixelSprite->ApplyPixelBuffer();

		m_camera->reset();
	};
}

double Mandelbrot::GetPixelSize()
{
	return (4.0 / (double)m_pixelSprite->getWidth()) / m_scale;
}

const oxygine::Color & Mandelbrot::GetColor(size_t escape)
{
	return m_colors[escape];
}

size_t Mandelbrot::CountEscapeIterations(std::complex<double> c)
{
	size_t ret = 0;
	std::complex<double> z(0.0, 0.0);

	while (norm(z) < 4.0 && ret < MAXIMUM_ESCAPE_ITERATIONS - 1)
	{
		ret++;
		z = z * z + c;
	}

	return ret;
}

void Mandelbrot::InitializeColors()
{
	m_colors.resize(MAXIMUM_ESCAPE_ITERATIONS);
	for (size_t i = 0; i < MAXIMUM_ESCAPE_ITERATIONS; ++i)
	{
		unsigned char value = (unsigned char)((float(i) / (float)MAXIMUM_ESCAPE_ITERATIONS) * 255.0);
		m_colors[i].r = value;
		m_colors[i].g = value;
		m_colors[i].b = value;
	}
	m_colors[MAXIMUM_ESCAPE_ITERATIONS - 1] = oxygine::Color::Black;
}

void Mandelbrot::ResetPixelSprite()
{
	double pixelSize = GetPixelSize();

	double x = -2.5 - m_offset.x;
	double y = -1.5 - m_offset.y;

	for (int i = 0; i < m_pixelSprite->getWidth(); ++i)
	{
		for (int j = 0; j < m_pixelSprite->getHeight(); ++j)
		{
			std::complex<double> c(
				((double)i) * pixelSize + x,
				((double)j) * pixelSize + y);

			m_pixelSprite->SetPixel(i, j, GetColor(CountEscapeIterations(c)));
		}
	}
}
