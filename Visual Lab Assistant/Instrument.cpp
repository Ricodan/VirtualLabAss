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

cv::Point3d Instrument::createPointOfLoop()
{
	std::vector<Point3d> objPoints;
	std::vector<Point2d> imgPoints;
	Point3d centerPoint3d = this->threeDimCoordinates;
	Point3d tipOfLoop = centerPoint3d;
	objPoints.push_back(centerPoint3d);

	//Projection of the point where the tip ought to be.
	Mat rotMat;
	Rodrigues(this->rotationVec, rotMat);
	Mat rotMatTpose = rotMat.t();
	double* tmp = rotMatTpose.ptr<double>(0);
	Point3d camWorldE(tmp[0] * 0.108, tmp[1] * 0.108, tmp[2] * 0.108);
	tipOfLoop += camWorldE;
	objPoints.push_back(tipOfLoop);
	//End of point projection code

	//It works after setting the rvec and tvec to (0,0,0). I don't really know why.
	//projectPoints(objPoints, Vec3d(0, 0, 0), Vec3d(0, 0, 0), this->cameraMatrix, this->distanceCoefficients, imgPoints);
	
	this->loopTip = tipOfLoop;

	return tipOfLoop;
}

void Instrument::react(Instrument* target, Protocol protocol)
{
 	
	if (target->iType == EPENDORPH)
	{
		std::cout << "Reacted with Ependorph tube" << std::endl;
		protocol.dispatch(LoopDippedInVial());
	}
	else if (target->iType == BURNER)
	{
		std::cout << "Reacted with Bunsen Burner" << std::endl;
		protocol.dispatch(LoopSterilize());
	}
	else if (target->iType == PETRI)
	{
		std::cout << "Reacted with Petri dish" << std::endl;
		protocol.dispatch(Streak());
	}

}

double euclideanDistToInst(cv::Vec3d pointA, cv::Vec3d pointB)
{
	double dist = sqrt(pow(pointA[0] - pointB[0], 2) + pow(pointA[1] - pointB[1], 2) + pow(pointA[2] - pointB[2], 2));
	return dist;

}

bool Instrument::madeContact(Instrument* instA)
{
	double distance = euclideanDistToInst(this->loopTip, instA->threeDimCoordinates);
	if (distance < 0.01) //The distance is set to react at when there's a centimeter of distance
	{
		return true;
	}
	return false;
}

Instrument::~Instrument()
{

}
