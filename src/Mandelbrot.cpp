#include "Mandelbrot.h"

// ---

std::vector<oxygine::Color> g_colors;

const oxygine::Color & GetColor(size_t escape)
{
	return g_colors[escape];
}

void InitializeColors()
{
	g_colors.resize(MAXIMUM_ESCAPE_ITERATIONS);
	for (size_t i = 0; i < MAXIMUM_ESCAPE_ITERATIONS; ++i)
	{
		unsigned char value = (unsigned char)((float(i) / (float)MAXIMUM_ESCAPE_ITERATIONS) * 255.0);
		g_colors[i].r = value;
		g_colors[i].g = value;
		g_colors[i].b = value;
	}
	g_colors[MAXIMUM_ESCAPE_ITERATIONS - 1] = oxygine::Color::Black;
}

size_t CountEscapeIterations(std::complex<double> c)
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

// ---

MandelbrotPart::MandelbrotPart(oxygine::Vector2 position, int width, int height, oxygine::Actor * parent)
	: m_width(width), m_height(height)
{
	m_sprite = new PixelSprite(width, height);
	m_sprite->setPosition(position);
	m_sprite->attachTo(parent);
}

MandelbrotPart::~MandelbrotPart()
{
	m_isWorkerRunning = false;
	if (m_worker && m_worker->joinable())
		m_worker->join();
}

void MandelbrotPart::Update(double x, double y, double pixelSize)
{
	m_isWorkerRunning = false;
	if (m_worker && m_worker->joinable())
		m_worker->join();

	m_isWorkerRunningLock.lock();
	m_isWorkerRunning = true;
	m_isWorkerRunningLock.unlock();

	m_worker.reset(new std::thread([this, x, y, pixelSize]() {

		oxygine::VectorD2 offset(m_sprite->getPosition().x*pixelSize - 2.5, m_sprite->getPosition().y*pixelSize - 1.5);

		for (int i = 0; i < m_width; ++i)
		{
			for (int j = 0; j < m_height; ++j)
			{
				std::complex<double> c(
					((double)i) * pixelSize + x + offset.x,
					((double)j) * pixelSize + y + offset.y);

				m_sprite->SetPixel(i, j, GetColor(CountEscapeIterations(c)));

				if (!m_isWorkerRunning)
					break;
			}
		}

		m_isWorkerRunningLock.lock();
		m_isWorkerRunning = false;
		m_isWorkerRunningLock.unlock();

	}));
}

void MandelbrotPart::Apply()
{
	m_isWorkerRunningLock.lock();
	if (!m_isWorkerRunning)
		m_sprite->ApplyPixelBuffer();
	m_isWorkerRunningLock.unlock();
}

bool MandelbrotPart::IsRunning()
{
	return m_isWorkerRunning;
}

// ---

Mandelbrot::Mandelbrot()
{
	InitializeColors();

	oxygine::Vector2 size = oxygine::getStage()->getSize();
	m_parent = new oxygine::Actor;

	m_camera = new Camera;
	m_camera->attachTo(this);
	m_camera->setSize(size);
	m_camera->setContent(m_parent.get());

	m_camera->_onCameraChange = [this]()
	{
		m_scale *= m_camera->getCameraScale();

		double pixelSize = GetPixelSize();

		oxygine::Vector2 offset = m_camera->getCameraOffset();
		oxygine::VectorD2 offsetIncrement = oxygine::VectorD2(offset.x, offset.y) * pixelSize;

		m_offset -= offsetIncrement;

		for (auto & p : m_parts)
		{
			p->part.Update(m_offset.x, m_offset.y, pixelSize);
			p->apply = true;
		}

		m_camera->reset();
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

	for (auto & p : m_parts)
	{
		p->part.Update(m_offset.x, m_offset.y, GetPixelSize());
		p->apply = true;
	}
}

double Mandelbrot::GetPixelSize()
{
	return (4.0 / (double)oxygine::getStage()->getWidth()) / m_scale;
}

void Mandelbrot::doUpdate(const UpdateState & us)
{
	for (auto & p : m_parts)
	{
		if (p->apply && !p->part.IsRunning())
		{
			p->part.Apply();
			p->apply = false;
		}
	}
}
