#pragma once
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
 
#include "InstrState.h"

#include <stdint.h>
#include <iostream>

class Instrument
{
public:
	int arucoId;
	InstrState state;
	cv::Point coordinates; //where on the screen the marker is located
	cv::Vec3d threeDimCoordinates;
	
	enum instrType {
		LOOP = 0,
		EPENDORPH = 1,
		BURNER = 2,
		PETRI = 3
	};
	instrType iType;

	//date of the last calibration 
	int day, month, year;

	

	

	void react(Instrument* target);
	bool madeContact(Instrument* instA);
	 

	Instrument(int id, cv::Vec3d markerCenterCoord);
	~Instrument();
};

