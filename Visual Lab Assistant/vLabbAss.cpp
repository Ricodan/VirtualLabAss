
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
const float arucoSquareDimension = 0.01f;
const Size chessboardDimensions = Size(9, 6);



void writeTextOnMarker(Mat& outputFrame, Mat& cameraMatrix, Mat& distanceCoefficients, String text, Point point)
{
	 
	//projectPoints(objPoints, rvec, tvec, cameraMatrix, distanceCoefficients, imgPoints, something);



}


void distanceToCamera(Mat& inOutFrame, Mat& cameraMatrix, Mat& distanceCoefficients)
{

}

//Testing using this for the object points. 
vector<Point3d> Generate3DPoints()
{
	std::vector<cv::Point3d> points;

	double x, y, z;

	x = .5; y = .5; z = -.5;
	points.push_back(cv::Point3d(x, y, z));

	x = .5; y = .5; z = .5;
	points.push_back(cv::Point3d(x, y, z));

	x = -.5; y = .5; z = .5;
	points.push_back(cv::Point3d(x, y, z));

	x = -.5; y = .5; z = -.5;
	points.push_back(cv::Point3d(x, y, z));

	x = .5; y = -.5; z = -.5;
	points.push_back(cv::Point3d(x, y, z));

	x = -.5; y = -.5; z = -.5;
	points.push_back(cv::Point3d(x, y, z));

	x = -.5; y = -.5; z = .5;
	points.push_back(cv::Point3d(x, y, z));

	for (unsigned int i = 0; i < points.size(); ++i)
	{
		std::cout << points[i] << std::endl << std::endl;
	}

	return points;
}

vector<Point3d> getAruco3dCenterCoords(double side, Vec3d rvec, Vec3d tvec)
{
	//https://stackoverflow.com/questions/46363618/aruco-markers-with-opencv-get-the-3d-corner-coordinates

	//Compute rot_mat
	Mat rotMat;
	Rodrigues(rvec, rotMat);
	//Transpose of rot_mat for easy column extraction.
	Mat rotMatTpose = rotMat.t();

	//Convert tvec to point

	Point3f tvec_3d(tvec[0], tvec[1], tvec[2]);

	//return vector:

	vector<Point3d> ret(4, tvec_3d);
	//TODO get the proper coordinates. Also, why is it 4 coords in the example from SO?

	return ret;

}

Point3d getAruco2dCenterCoords(vector<Point2d> corners, double side, Vec3d rvec, Vec3d tvec)
{
	Mat rotMat;
	Rodrigues(rvec, rotMat);
	//Transpose of rotMat for easy column extraction.
	Mat rotMatTpose = rotMat.t();

	double* tmp = rotMatTpose.ptr<double>(0);
	Point3d camWorldO(tmp[0], tmp[1], tmp[2]); // points of the aruco marker in the frame
	Point3d tvec3D(tvec[0], tvec[1], tvec[2]);

	camWorldO += tvec3D;
	cout << camWorldO.x << " " << camWorldO.y << endl;

	return camWorldO;


}

void showCoordsAtPos(Mat& frame, /*String string,*/ Point position, Vec3d tvec)
{
	ostringstream oCoordsString;
	oCoordsString << "MARKER Position x=" << tvec[0] << "y=" << tvec[1] << "z=" << tvec[2];
	string coordsString = oCoordsString.str();
	putText(frame, coordsString, position, FONT_HERSHEY_COMPLEX_SMALL, 0.5, Scalar(0, 255, 26));
}


int startWebcamMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions)
{
	Mat frame;
	vector<Point3d> objPoints = Generate3DPoints(); // Perhaps this thing is the problem since it's empty. This probably should be the markers. Update:
													// It is indeed this the problem. Need valid points, no crash with Generate3DPoints().
	//Try using corners of the aruco markers, or at least, a corner. 
		 
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
		aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimensions, cameraMatrix, distanceCoefficients, rotationVectors, translationVectors);


		for (int i = 0; i < markerIds.size(); i++)
		{

			aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.1f);
			
			//The below code is to print stuff on the screen. 
			showCoordsAtPos(frame, Point(markerCorners[i][2].x, markerCorners[i][2].y), translationVectors[i]);
			//cout << translationVectors[i] << endl;
			cout << markerCorners[i][3] << endl;
		 
		}
		aruco::drawDetectedMarkers(frame, markerCorners, markerIds);

			
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