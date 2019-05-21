#pragma once
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/aruco.hpp"

#include "InstrState.h"

#include <stdint.h>
#include <iostream>

class Instrument
{
public:
	int arucoId;
	InstrState state;
	//date of the last calibration 
	int day, month, year;

	cv::Point coordinates; //where on the screen the marker is located





	Instrument(int id);
	~Instrument();
};

