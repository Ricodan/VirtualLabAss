#include "Instrument.h"


Instrument::Instrument(int id, cv::Vec3d markerCenterCoord)
{
	this->arucoId = id;
	this->threeDimCoordinates = markerCenterCoord;
}


Instrument::~Instrument()
{

}
