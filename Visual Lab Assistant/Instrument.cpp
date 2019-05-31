
 

#include "Instrument.h"

#include <iostream>

Instrument::Instrument(int id, cv::Vec3d markerCenterCoord)
{
	this->arucoId = id;
	this->threeDimCoordinates = markerCenterCoord;




}

void Instrument::react(Instrument* target)
{
	std::cout << "Just something" << std::endl;
	std::cout << "Reacting with" << target << std::endl;
}

double euclideanDistToInst(cv::Vec3d pointA, cv::Vec3d pointB)
{
	double dist = sqrt(pow(pointA[0] - pointB[0], 2) + pow(pointA[1] - pointB[1], 2) + pow(pointA[2] - pointB[2], 2));
	return dist;

}

bool Instrument::madeContact(Instrument* instA)
{
	double distance = euclideanDistToInst(this->threeDimCoordinates, instA->threeDimCoordinates);
	if (distance < 0.005) //The distance is set to react at when there's a centimeter of distance
	{
		return true;
	}
	return false;
}

Instrument::~Instrument()
{

}
