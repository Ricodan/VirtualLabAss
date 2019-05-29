#include "Instrument.h"
#include <iostream>

Instrument::Instrument(int id, cv::Vec3d markerCenterCoord)
{
	this->arucoId = id;
	this->threeDimCoordinates = markerCenterCoord;




}
void Instrument::react()
{
	std::cout << "Just something" << std::endl;
}

Instrument::~Instrument()
{

}
