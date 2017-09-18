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
#include <tuple>
#include <condition_variable>

// TODO divide into separate classes

class MandelbrotPart
{
public:
	MandelbrotPart(oxygine::Vector2 position, int width, int height, oxygine::Actor * parent);
	~MandelbrotPart();
	void Update(double x, double y, double pixelSize, size_t maximumIterations);

	bool Apply();
	void Clear();
	bool IsRunning();

private:
	void StopWorker();
	bool m_isWorkerRunning = false;
	std::mutex m_isWorkerRunningLock;

	std::unique_ptr<std::thread> m_worker;
	spPixelSprite m_sprite;

	const int m_width;
	const int m_height;

	size_t m_maximumIterations = 1024;

	// 
	std::vector<std::vector<size_t>> m_iterations;
	size_t m_minIterations;
	size_t m_maxIterations;
};

DECLARE_SMART(Mandelbrot, spMandelbrot);
class Mandelbrot : public oxygine::Actor
{
public:
	Mandelbrot(std::function<void()> onCameraChange);

	void Reset();
	void Update(oxygine::VectorD2 offset, double scale);
	std::tuple<oxygine::VectorD2, double> GetCameraData();

private:
	virtual void doUpdate(const UpdateState& us) override;
	double GetPixelSize();
	size_t GetMaximumIterations();

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

DECLARE_SMART(MandelbrotParent, spMandelbrotParent);
class MandelbrotParent : public oxygine::Actor
{
public:
	MandelbrotParent()
	{
		auto onCameraChange = [this]()
		{
			oxygine::log::messageln("onCameraChange");

			auto cameraData = m_foreground->GetCameraData();

			m_background->Reset();
			m_background->Update(std::get<0>(cameraData), std::get<1>(cameraData));

			SwapMandelbrots();
			ApplyMandelbrots();
		};

		m_mandelbrotA = new Mandelbrot(onCameraChange);
		m_mandelbrotB = new Mandelbrot(onCameraChange);

		m_mandelbrotA->attachTo(this);
		m_mandelbrotB->attachTo(this);

		m_mandelbrotA->Update({0.0, 0.0}, 1.0);
		
		m_foreground = m_mandelbrotA.get();
		m_background = m_mandelbrotB.get();

		ApplyMandelbrots();
	}

private:
	void SwapMandelbrots()
	{
		Mandelbrot * tmp = m_foreground;
		m_foreground = m_background;
		m_background = tmp;
	}
	void ApplyMandelbrots()
	{
		m_foreground->setPriority(2);
		m_background->setPriority(1);

		m_foreground->setTouchEnabled(true);
		m_foreground->setTouchChildrenEnabled(true);
		m_background->setTouchEnabled(false);
		m_background->setTouchChildrenEnabled(false);
	}

	spMandelbrot m_mandelbrotA;
	spMandelbrot m_mandelbrotB;

	Mandelbrot * m_foreground;
	Mandelbrot * m_background;
};
