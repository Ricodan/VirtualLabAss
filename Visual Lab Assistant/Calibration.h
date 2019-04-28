#pragma once
#ifndef _calibration_h_
#define _calibration_h_

#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"

#include <stdint.h>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

void createArucoMarkers();
void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners);
void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& allFoundCorners, bool showResults = true);
void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distanceCoeffiecients);
bool saveCameraCalibration(string name, Mat cameraMatrix, Mat distanceCoefficients);
bool loadCameraCalibration(string name, Mat& cameraMatrix, Mat& distanceCoefficients);
//int startWebcamMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions);
void livestreamCameraCalibration(Mat& cameraMatrix, Mat& distanceCoefficients);

#endif