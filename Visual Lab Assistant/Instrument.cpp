#include "Instrument.h"
#include "Protocol.h"

#include <iostream>

Instrument::Instrument(int id, cv::Vec3d markerCenterCoord)
{
	this->arucoId = id;
	this->threeDimCoordinates = markerCenterCoord;
	assignType(id);

}

void Instrument::assignType(int id)
{
	if (id == 0)
	{
		this->iType = LOOP;
	}
	else if (id == 1)
	{
		this->iType = BURNER;
	}
	else if (id >= 10 && id < 20)
	{
		this -> iType = EPENDORPH;
	}
	else if (id >= 20)
	{
		this->iType = PETRI;
	}
}

void Instrument::react(Instrument* target)
{
 	
	if (target->iType == EPENDORPH)
	{
		std::cout << "Reacted with Ependorph tube" << std::endl;
		Protocol::dispatch(LoopDippedInVial());
	}
	else if (target->iType == BURNER)
	{
		std::cout << "Reacted with Bunsen Burner" << std::endl;
		Protocol::dispatch(LoopSterilize());
	}
	else if (target->iType == PETRI)
	{
		std::cout << "Reacted with Petri dish" << std::endl;
		Protocol::dispatch(Streak());
	}

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
