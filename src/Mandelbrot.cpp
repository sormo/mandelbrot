#include "Mandelbrot.h"

// ---

#define MAXIMUM_ESCAPE_ITERATIONS 1024
std::vector<oxygine::Color> g_palette;

const oxygine::Color & GetColor(size_t escape)
{
	return g_palette[escape];
}

void InitializePalette()
{
	static const size_t PALETTE_SIZE = 255;

	g_palette.resize(PALETTE_SIZE);
	for (size_t i = 0; i < PALETTE_SIZE; ++i)
	{
		unsigned char value = (unsigned char)((float(i) / (float)PALETTE_SIZE) * 255.0);
		g_palette[i].r = value;
		g_palette[i].g = value;
		g_palette[i].b = value;
	}
	g_palette[PALETTE_SIZE - 1] = oxygine::Color::Black;
}

size_t CountEscapeIterations(std::complex<double> c, size_t maximumIterations)
{
	size_t ret = 0;
	std::complex<double> z(0.0, 0.0);

	while (norm(z) < 4.0 && ret <= maximumIterations - 1)
	{
		ret++;
		z = z * z + c;
	}

	return ret;
}

struct PaletteHandle
{
	PaletteHandle(size_t minIterations, size_t maxIterations) :
		m_minIterations(minIterations), m_conversionFactor(1.0) 
	{
		if (maxIterations != minIterations)
			m_conversionFactor = ((double)g_palette.size() - 1.0) / (double)maxIterations;
	}
	const oxygine::Color & GetColor(size_t iterations)
	{
		assert(iterations >= m_minIterations);
		size_t index = m_conversionFactor * iterations;
		assert(index < g_palette.size());
		return g_palette[index];
	}
private:
	size_t m_minIterations;
	double m_conversionFactor;
};

// ---

MandelbrotPart::MandelbrotPart(oxygine::Vector2 position, int width, int height, oxygine::Actor * parent)
	: m_width(width), m_height(height), m_iterations(m_width, std::vector<size_t>(m_height, 0))
{
	m_sprite = new PixelSprite(width, height);
	m_sprite->setPosition(position);
	m_sprite->attachTo(parent);
}

MandelbrotPart::~MandelbrotPart()
{
	StopWorker();
}

void MandelbrotPart::Update(double x, double y, double pixelSize, size_t maximumIterations)
{
	StopWorker();

	m_isWorkerRunningLock.lock();
	m_isWorkerRunning = true;
	m_isWorkerRunningLock.unlock();

	m_maximumIterations = maximumIterations;

	m_worker.reset(new std::thread([this, x, y, pixelSize]() {

		oxygine::VectorD2 offset(m_sprite->getPosition().x*pixelSize - 2.5, m_sprite->getPosition().y*pixelSize - 1.5);

		size_t minIterations = std::numeric_limits<size_t>::max();
		size_t maxIterations = std::numeric_limits<size_t>::min();
		for (int i = 0; i < m_width; ++i)
		{
			for (int j = 0; j < m_height; ++j)
			{
				std::complex<double> c(
					((double)i) * pixelSize + x + offset.x,
					((double)j) * pixelSize + y + offset.y);

				m_iterations[i][j] = CountEscapeIterations(c, m_maximumIterations);
				minIterations = std::min(minIterations, m_iterations[i][j]);
				maxIterations = std::max(maxIterations, m_iterations[i][j]);

				if (!m_isWorkerRunning)
					break;
			}
		}

		//PaletteHandle palette(minIterations, maxIterations);
		PaletteHandle palette(0, MAXIMUM_ESCAPE_ITERATIONS);
		for (int i = 0; i < m_width; ++i)
			for (int j = 0; j < m_height; ++j)
				m_sprite->SetPixel(i, j, palette.GetColor(m_iterations[i][j]));

		m_isWorkerRunningLock.lock();
		m_isWorkerRunning = false;
		m_isWorkerRunningLock.unlock();

	}));
}

bool MandelbrotPart::Apply()
{
	m_isWorkerRunningLock.lock();
	// return true if worker is not running and buffer was applied
	bool ret = !m_isWorkerRunning;

	if (!m_isWorkerRunning)
		m_sprite->ApplyPixelBuffer();

	m_isWorkerRunningLock.unlock();

	return ret;
}

void MandelbrotPart::Clear()
{
	StopWorker();
	
	m_sprite->Clear();
}

bool MandelbrotPart::IsRunning()
{
	return m_isWorkerRunning;
}

void MandelbrotPart::StopWorker()
{
	m_isWorkerRunning = false;
	if (m_worker && m_worker->joinable())
		m_worker->join();
}

// ---

Mandelbrot::Mandelbrot(std::function<void()> onCameraChange)
{
	InitializePalette();

	oxygine::Vector2 size = oxygine::getStage()->getSize();
	m_parent = new oxygine::Actor;

	m_camera = new Camera;
	m_camera->attachTo(this);
	m_camera->setSize(size);
	m_camera->setContent(m_parent.get());

	m_camera->_onCameraChange = [this, onCameraChange]()
	{
		m_scale *= m_camera->getCameraScale();

		oxygine::Vector2 offset = m_camera->getCameraOffset();
		oxygine::VectorD2 offsetIncrement = oxygine::VectorD2(offset.x, offset.y) *  GetPixelSize();

		m_offset -= offsetIncrement;

		onCameraChange();
	};

	const int WIDTH = 200;
	const int HEIGHT = 200;

	double pixelSize = GetPixelSize();
	for (int x = 0; x < size.x; x += WIDTH)
	{
		for (int y = 0; y < size.y; y += HEIGHT)
		{
			int width = x + WIDTH > (int)size.x ? (int)size.x - x : WIDTH;
			int height = y + HEIGHT >(int)size.y ? (int)size.y - y : HEIGHT;

			m_parts.push_back(std::unique_ptr<Part>(new Part(oxygine::Vector2{(float)x, (float)y}, width, height, m_parent.get())));
		}
	}
}

void Mandelbrot::Reset()
{
	for (auto & p : m_parts)
	{
		p->part.Clear();
		p->part.Apply();
	}

	m_camera->reset();
}

void Mandelbrot::Update(oxygine::VectorD2 offset, double scale)
{
	m_offset = offset;
	m_scale = scale;

	for (auto & p : m_parts)
	{
		p->part.Update(m_offset.x, m_offset.y, GetPixelSize(), GetMaximumIterations());
		p->apply = true;
	}
}

std::tuple<oxygine::VectorD2, double> Mandelbrot::GetCameraData()
{
	return std::tie(m_offset, m_scale);
}

double Mandelbrot::GetPixelSize()
{
	return (4.0 / (double)oxygine::getStage()->getWidth()) / m_scale;
}

size_t Mandelbrot::GetMaximumIterations()
{
	return MAXIMUM_ESCAPE_ITERATIONS;
}

void Mandelbrot::doUpdate(const UpdateState & us)
{
	for (auto & p : m_parts)
	{
		if (p->apply && !p->part.IsRunning())
		{
			if (p->part.Apply())
				p->apply = false;
		}
	}
}
