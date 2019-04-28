
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"


#include "Calibration.h"

 
#include <stdint.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

const int fps = 20;
const float calibrationSquareDimension = 0.024f;
const float arucoSquareDimension = 0.02f;
const Size chessboardDimensions = Size(9, 6);



void writeTextOnMarker(Mat& outputFrame, Mat& cameraMatrix, Mat& distanceCoefficients, String text, Point point)
{
	 
	//projectPoints(objPoints, rvec, tvec, cameraMatrix, distanceCoefficients, imgPoints, something);



}


int startWebcamMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions)
{
	Mat frame;
	vector<Point3d> objPoints; // Perhaps this thing is the problem since it's empty. This probably should be the markers. 
	vector<Point2d> imgPoints;
	/*ostringstream oCoordsString;
	string coordsString;*/
	vector<int> markerIds;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;
	aruco::DetectorParameters paramters;
	Ptr< aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);

	VideoCapture vid(0);

	if (!vid.isOpened())
	{
		return -1;
	}

	namedWindow("Webcam", WINDOW_AUTOSIZE);
	vector<Vec3d> rotationVectors, translationVectors;

	while (true) //Basically the main loop of when the camera is running. 
	{
		if (!vid.read(frame))
			break;

		aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds);
		aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimensions, cameraMatrix,
			distanceCoefficients, rotationVectors, translationVectors);


		for (int i = 0; i < markerIds.size(); i++)
		{

			aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.1f);
			
			//The below code is to print stuff on the screen. 
			
			//TODO: objPoints needs to be either Point2d or Point3d. It should probably be the values of the markers in the 3d plane, and imgPoints 
			//may be referring to the <<frame>> variable. 
			projectPoints(objPoints, rotationVectors[i], translationVectors[i], cameraMatrix, distanceCoefficients, imgPoints); // problem with some parameter
			
			ostringstream oCoordsString;
			oCoordsString << "MARKER Position x=" << translationVectors[i][0] << "y=" << translationVectors[i][1] << "z=" << translationVectors[i][2];
			string coordsString = oCoordsString.str();
			//putText(frame,coordsString, Point(imgPoints[0].x, imgPoints[0].y) /*Point(50,50)*/, FONT_HERSHEY_COMPLEX_SMALL, 0.5, Scalar(0,255,26) );

			cout << coordsString << endl;
			//translationVectors<double>at.(), tvecs[i][0][0], translationVectors self.tvecs[i][0][1], translationVectors self.tvecs[i][0][2])
			//cv2.putText(cv_image, str_position, (int(imgpts[0][0][0]), int(imgpts[0][0][1])), font, 1,

		}
		aruco::drawDetectedMarkers(frame, markerCorners, markerIds);


		//Get the points on the screen of an aruco marker. 
		//putText(frame, "Testing", Point(50, 50), FONT_HERSHEY_DUPLEX, 0.3f, Scalar(0, 143, 143), 2);
		imshow("Webcam", frame);
		if (waitKey(30) >= 0) break;

	}


	return 1;
}

int main(char argv, char** argc)

{
	cout << "just something to pring" << endl;
	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
	Mat distanceCoefficients;

	//slight improvement, perhaps the calibration input was all that was wrong. 
	//livestreamCameraCalibration(cameraMatrix, distanceCoefficients);
	loadCameraCalibration("CalibrationInfo", cameraMatrix, distanceCoefficients);

	startWebcamMonitoring(cameraMatrix, distanceCoefficients, arucoSquareDimension);

	return 0;



}