#include "Instrument.h"
#include "Protocol.h"

#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"

#include <iostream>

using namespace cv;

Instrument::Instrument(int id, cv::Vec3d markerCenterCoord, cv::Mat camMat, cv::Mat distCoeff)
{
	this->arucoId = id;
	this->threeDimCoordinates = markerCenterCoord;
	this->cameraMatrix = camMat;
	this->distanceCoefficients = distCoeff;
	assignType(id);

}

void Instrument::assignType(int id)
{
	if (id == 0)
	{
		this->iType = LOOP;
	}
	else if (id == 1 )
	{
		this->iType = BURNER;
	}
	else if (id >= 10 && id < 20)
	{
		this -> iType = EPENDORPH;
	}
	else if (id >= 20 && id < 30)
	{
		this->iType = PETRI;
	}
}

void Instrument::createPointOfLoop()
{
	Point3d centerPoint3d = this->threeDimCoordinates;
	Point3d tipOfLoop = centerPoint3d;

	//Projection of the point where the tip ought to be.
	Mat rotMat;
	Rodrigues(this->rotationVec, rotMat);
	Mat rotMatTpose = rotMat.t();
	double* tmp = rotMatTpose.ptr<double>(0);
	Point3d prolongPoint(tmp[0] * 0.108, tmp[1] * 0.108, tmp[2] * 0.108);
	tipOfLoop += prolongPoint;
	//End of point projection code
	
	this->loopTip = tipOfLoop;
}

double euclideanDistToInst(cv::Point3d pointA, cv::Point3d pointB)
{
	double dist = sqrt(pow(pointA.x - pointB.x, 2) + pow(pointA.y - pointB.y, 2) + pow(pointA.z - pointB.z, 2));
	return dist;
}

void Instrument::react(Instrument* target, Protocol protocol)
{
	if (target->hasDisengaged == true)
	{
		if (target->iType == EPENDORPH)
		{
			std::cout << "Reacted with Ependorph tube" << std::endl;
			protocol.dispatch(LoopDippedInVial());
			target->hasDisengaged = false;
		}
		else if (target->iType == BURNER)
		{
			std::cout << "Reacted with Bunsen Burner" << std::endl;
			protocol.dispatch(LoopSterilize());
			target->hasDisengaged = false;
		}
		else if (target->iType == PETRI)
		{
			std::cout << "Reacted with Petri dish" << std::endl;
			protocol.dispatch(Streak());
			target->hasDisengaged = false;
		}
	}
}



//It is assumed that only the loop will be making contact with other things.  
bool Instrument::madeContact(Instrument* instA)
{
	double distance = euclideanDistToInst(this->loopTip, instA->threeDimCoordinates);
	std::cout << "distance " << this->arucoId << " to " << instA->arucoId << " " << distance << std::endl;
	if (distance < 0.05) 
	{
		return true;
	}
	else if (distance > 0.1)
	{
		std::cout << "Is disengaged True" << endl;
		instA->hasDisengaged = true;
	}
	return false;
}

Instrument::~Instrument()
{

}
