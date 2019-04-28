
#include "Calibration.h"
#include <stdint.h>
#include <sstream>
#include <iostream>
#include <fstream>

#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

const int fps = 20;
const float calibrationSquareDimension = 0.024f;
const float arucoSquareDimension = 0.02f;
const Size chessboardDimensions = Size(9, 6);



void createArucoMarkers()
{
	Mat outputMarker;

	Ptr<aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);

	for (int i = 0; i < 50; i++)
	{
		aruco::drawMarker(markerDictionary, i, 500, outputMarker, 1);
		ostringstream convert;
		string imageName = "4x4Marker_";
		convert << imageName << i << ".jpg";
		imwrite(convert.str(), outputMarker);

	}

}

//This function basically creates all the values where the squares would meet in real life. 
void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners)
{
	for (int i = 0; i < boardSize.height; i++)
	{
		for (int j = 0; j < boardSize.width; j++)
		{
			corners.push_back(Point3f(j * squareEdgeLength, i * squareEdgeLength, 0.0f));
		}
	}
}

void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& allFoundCorners, bool showResults)
{
	for (vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++)
	{
		vector<Point2f> pointBuf;
		bool found = findChessboardCorners(*iter, Size(9, 6), pointBuf, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE );

		if (found)
		{
			allFoundCorners.push_back(pointBuf);
		}
		if (showResults)
		{
			drawChessboardCorners(*iter, Size(9, 6), pointBuf, found);
			imshow("Looking for corners", *iter);
			waitKey(0);
		}
	}
}

void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distanceCoeffiecients)
{
	vector<vector<Point2f>> checkerboardImageSpacePoints;
	getChessboardCorners(calibrationImages, checkerboardImageSpacePoints, false);

	vector<vector<Point3f>> worldSpaceCornerPoints(1); 

	createKnownBoardPosition(boardSize, squareEdgeLength, worldSpaceCornerPoints[0]);
	worldSpaceCornerPoints.resize(checkerboardImageSpacePoints.size(), worldSpaceCornerPoints[0]);

	vector<Mat> rVectors, tVectors;
	distanceCoeffiecients = Mat::zeros(8, 1, CV_64F);

	calibrateCamera(worldSpaceCornerPoints, checkerboardImageSpacePoints, boardSize, cameraMatrix, distanceCoeffiecients, rVectors, tVectors);

}

bool saveCameraCalibration(string name, Mat cameraMatrix, Mat distanceCoefficients)
{
	ofstream outStream(name);
	if (outStream)
	{
		uint16_t rows = cameraMatrix.rows;
		uint16_t  columns = cameraMatrix.cols;

		outStream << rows << endl;
		outStream << columns << endl;

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double value = cameraMatrix.at<double>(r, c);
				outStream << value << endl;
			}
		}
		rows = distanceCoefficients.rows;
		columns = distanceCoefficients.cols;

		outStream << rows << endl;
		outStream << columns << endl;

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double value = distanceCoefficients.at<double>(r, c);
				outStream << value << endl;
			}
		}

		outStream.close();
		return true;

	}

	return false;
}

bool loadCameraCalibration(string name, Mat& cameraMatrix, Mat& distanceCoefficients)
{
	ifstream inStream(name);
	if (inStream)
	{
		uint16_t rows;
		uint16_t columns;

		inStream >> rows;
		inStream >> columns;

		cameraMatrix = Mat(Size(columns, rows), CV_64F);

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				inStream >> read;
				cameraMatrix.at<double>(r, c) = read;
				cout << cameraMatrix.at<double>(r, c) << "\n";
			}
		}
		//Distance Coefficients

		inStream >> rows;
		inStream >> columns;

		distanceCoefficients = Mat::zeros(rows, columns, CV_64F);

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				inStream >> read;
				distanceCoefficients.at<double>(r, c) = read;
				cout << distanceCoefficients.at<double>(r, c) << "\n";
			}
		}
		inStream.close();
		return true;
	}
	return false;
}
//
//int startWebcamMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions)
//{
//	Mat frame;
//	vector<int> markerIds;
//	vector<vector<Point2f>> markerCorners, rejectedCandidates;
//	aruco::DetectorParameters paramters;
//
//	Ptr< aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);
//
//	VideoCapture vid(0);
//
//	if (!vid.isOpened())
//	{
//		return -1;
//	}
//
//	namedWindow("Webcam", WINDOW_AUTOSIZE);
//	vector<Vec3d> rotationVectors, translationVectors;
//
//	while (true)
//	{
//		if (!vid.read(frame))
//			break;
//
//		aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds);
//		aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimensions, cameraMatrix, 
//			distanceCoefficients, rotationVectors, translationVectors);
//	
//
//		for (int i = 0; i < markerIds.size(); i++)
//		{
//			aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.1f);
//
//		}
//		imshow("Webcam", frame);
//		if (waitKey(30) >= 0) break;
//
//	}
//	return 1;
//}

void livestreamCameraCalibration(Mat& cameraMatrix, Mat& distanceCoefficients)
{
	Mat frame;
	Mat drawToF;
	
	vector<Mat> savedImages;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;
	VideoCapture vid(0);
	int imageCounter = 0;

	if (!vid.isOpened())
	{
		return;
	}

	int framesPerSecond = 20;
	namedWindow("Webcam", WINDOW_AUTOSIZE);
	
	while (true)
	{
		if (!vid.read(frame))
			break;

		vector<Vec2f> foundPoints;
		bool found = false;

		found = findChessboardCorners(frame, chessboardDimensions, foundPoints, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);
		frame.copyTo(drawToF);
		drawChessboardCorners(drawToF, chessboardDimensions, foundPoints, found);

		if (found)
			imshow("Webcam", drawToF);
		else
			imshow("Webcam", frame);
		char character = waitKey(1000 / fps);

		switch (character)
		{
		case ' ':
			//saving image
			if (found)
			{
				Mat temp;
				frame.copyTo(temp);
				savedImages.push_back(temp);
				imageCounter++;
				cout << imageCounter << endl;
			}
			break;

		case 13:
			//start calibration
			if (savedImages.size() > 25)
			{
				cout << "Starting Camera Calibration" << endl;
				cameraCalibration(savedImages, chessboardDimensions, calibrationSquareDimension, cameraMatrix, distanceCoefficients);
				saveCameraCalibration("CalibrationInfo", cameraMatrix, distanceCoefficients);
			}
			cout << "Finished with Camera Calibration" << endl;
			break;

		case 27:
			//exit
			return;
			break;
		}
	}
	return;
}