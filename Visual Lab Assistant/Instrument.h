#pragma once
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"

#include "Protocol.h"

#include <stdint.h>
#include <iostream>

class Instrument
{
public:
	int arucoId;
	cv::Point coordinates; //where on the screen the marker is located
	cv::Point3d threeDimCoordinates;
	cv::Point3d loopTip;
	cv::Point3d flameTip;
	cv::Vec3d rotationVec;
	cv::Vec3d translationVec;
	bool hasDisengaged = true;

	enum instrType {
		LOOP = 0,
		EPENDORPH = 1,
		BURNER = 2,
		PETRI = 3
	};
	instrType iType;


	void react(Instrument* target, Protocol protocol);
	bool madeContact(Instrument* instA);
	void assignType(int id);
	void createPointOfLoop();

	Instrument(int id, cv::Vec3d markerCenterCoord, cv::Mat camMat, cv::Mat distCoeff);
	~Instrument();

private:
	 //activates when the instrument is separated from the target instrument
	cv::Mat cameraMatrix;
	cv::Mat distanceCoefficients;

};

