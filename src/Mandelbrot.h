#include "oxygine-framework.h"
#pragma once
#include "PixelSprite.h"
#include "Camera.h"
#include <functional>
#include <complex>

#define MAXIMUM_ESCAPE_ITERATIONS 128

DECLARE_SMART(Mandelbrot, spMandelbrot);
class Mandelbrot : public oxygine::Actor
{
public:
	Mandelbrot();

private:

	std::vector<oxygine::Color> m_colors;
	spPixelSprite m_pixelSprite;
	spCamera m_camera;

	// camera scale and offset
	oxygine::VectorD2 m_offset;
	double m_scale = 1.0;

	double GetPixelSize();
	const oxygine::Color & GetColor(size_t escape);
	size_t CountEscapeIterations(std::complex<double> c);
	void InitializeColors();
	void ResetPixelSprite();
};
