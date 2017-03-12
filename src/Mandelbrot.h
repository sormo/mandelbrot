#pragma once
#include "oxygine-framework.h"
#include "PixelSprite.h"
#include "Camera.h"
#include <functional>
#include <complex>

#include <thread>
#include <mutex>
#include <atomic>
#include <memory>

#define MAXIMUM_ESCAPE_ITERATIONS 128

class MandelbrotPart
{
public:
	MandelbrotPart(oxygine::Vector2 position, int width, int height, oxygine::Actor * parent);
	~MandelbrotPart();
	void Update(double x, double y, double pixelSize);

	void Apply();

	bool IsRunning();

private:
	bool m_isWorkerRunning = false;
	std::mutex m_isWorkerRunningLock;

	std::unique_ptr<std::thread> m_worker;
	spPixelSprite m_sprite;

	const int m_width;
	const int m_height;
};

DECLARE_SMART(Mandelbrot, spMandelbrot);
class Mandelbrot : public oxygine::Actor
{
public:
	Mandelbrot();

private:
	virtual void doUpdate(const UpdateState& us) override;
	double GetPixelSize();

	struct Part
	{
		Part(oxygine::Vector2 position, int width, int height, oxygine::Actor * parent) : 
			part(position, width, height, parent) {}
		MandelbrotPart part;
		bool apply = false;
	};
	std::vector<std::unique_ptr<Part>> m_parts;

	oxygine::spActor m_parent;
	spCamera m_camera;
	oxygine::VectorD2 m_offset;
	double m_scale = 1.0;
};
