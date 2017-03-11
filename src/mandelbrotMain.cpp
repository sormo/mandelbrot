#include "oxygine-framework.h"
#include "Mandelbrot.h"

Resources gameResources;

void mandelbrotPreinit() {}

//called from main.cpp
void mandelbrotInit()
{
	//load xml file with resources definition
	gameResources.loadXML("res.xml");

	spMandelbrot m = new Mandelbrot;
	m->attachTo(oxygine::getStage());
}


//called each frame from main.cpp
void mandelbrotUpdate()
{
}

//called each frame from main.cpp
void mandelbrotDestroy()
{
	//free previously loaded resources
	gameResources.free();
}
