
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"

#include "Calibration.h"
#include "Instrument.h"
#include "InocLoop.h"
#include "Protocol.h" 

#include <stdint.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

const int fps = 20;
const float calibrationSquareDimension = 0.024f;
const float arucoSquareDimension = 0.01f; //Distances are based in meters.
const float arucoSquareDimensionSecondSet = 0.012;
const Size chessboardDimensions = Size(9, 6);



void writeTextOnMarker(Mat& outputFrame, Mat& cameraMatrix, Mat& distanceCoefficients, String text, Point point)
{
	 
	//projectPoints(objPoints, rvec, tvec, cameraMatrix, distanceCoefficients, imgPoints, something);



}



void distanceToCamera(Mat& inOutFrame, Mat& cameraMatrix, Mat& distanceCoefficients)
{

}

double euclideanDist(Vec3d pointA, Vec3d pointB)
{
	double dist = sqrt(pow(pointA[0] - pointB[0], 2) + pow(pointA[1] - pointB[1], 2) + pow(pointA[2] - pointB[2], 2));
	return dist;

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

vector<Point3d> get3dArucoSquareCorners(double side, Vec3d rvec, Vec3d tvec)
{
	//https://stackoverflow.com/questions/46363618/aruco-markers-with-opencv-get-the-3d-corner-coordinates
	double halfSide = side / 2;

	//Compute rot_mat
	Mat rotMat;
	Rodrigues(rvec, rotMat);
	//Transpose of rot_mat for easy column extraction.
	Mat rotMatTpose = rotMat.t();

	//E - 0 and F - 0 vectors
	double* tmp = rotMatTpose.ptr<double>(0);
	Point3d camWorldE(tmp[0] * halfSide,
		tmp[1] * halfSide,
		tmp[2] * halfSide);

	tmp = rotMatTpose.ptr<double>(1);
	Point3d camWorldF(tmp[0] * halfSide,
		tmp[1] * halfSide,
		tmp[2] * halfSide);

	//convert tvec to point
	Point3d tvec_3d(tvec[0], tvec[1], tvec[2]);
	//return vector:
	vector<Point3d> ret(4, tvec_3d);

	ret[0] += ret[0] + camWorldE + camWorldF;
	ret[1] += -camWorldE + camWorldF;
	ret[2] += -camWorldE - camWorldF;
	ret[3] += camWorldE - camWorldF;

	return ret;
}

//Getting 3d points
vector<Point3d> getAruco3dCornerCoords(double side, Vec3d rvec, Vec3d tvec)
{
	//https://stackoverflow.com/questions/46363618/aruco-markers-with-opencv-get-the-3d-corner-coordinates
	double halfSide = side / 2;

	//Compute rot_mat
	Mat rotMat;
	Rodrigues(rvec, rotMat);
	//Transpose of rot_mat for easy column extraction.
	Mat rotMatTpose = rotMat.t();

	//E - 0 and F - 0 vectors
	double* tmp = rotMatTpose.ptr<double>(0);
	Point3d camWorldE(tmp[0] * halfSide,
		tmp[1] * halfSide,
		tmp[2] * halfSide);

	tmp = rotMatTpose.ptr<double>(1);
	Point3d camWorldF(tmp[0] * halfSide,
		tmp[1] * halfSide,
		tmp[2] * halfSide);


	//Point3d camWorldO();

	//convert tvec to point
	Point3d tvec_3d(tvec[0], tvec[1], tvec[2]);
	//return vector:
	vector<Point3d> ret(4, tvec_3d);

	ret[0] += camWorldE + camWorldF;  //Top Left Corner
	ret[1] += -camWorldE + camWorldF; //Top Right Corner
	ret[2] += -camWorldE - camWorldF; //Bottom Right Corner
	ret[3] += camWorldE - camWorldF;  //Bottom Left Corner

	return ret;

}

//Perhaps change this to Point 3f to make it easier 
Point3d getAruco3dCenterCoords(double side, Vec3d rvec, Vec3d tvec)
{ 
	Mat rotMat;
	Rodrigues(rvec, rotMat);
	//Transpose of rotMat for easy column extraction.
	Mat rotMatTpose = rotMat.t();

	double* tmp = rotMatTpose.ptr<double>(0);
	Point3d camWorldO(tmp[0], tmp[1], tmp[2]); // points of the aruco marker in the frame
	Point3d tvec3D(tvec[0], tvec[1], tvec[2]);

	camWorldO += tvec3D;
	//cout << camWorldO.x << " " << camWorldO.y<< " " <<camWorldO.z << endl;

	return camWorldO;


}

void showCoordsAtPos(Mat& frame, /*String string,*/ Point position, Vec3d tvec)
{
	//The line below is how I was calling this function from the loop in startWebcamMonitoring
	//showCoordsAtPos(frame, Point(markerCorners[i][2].x, markerCorners[i][2].y), translationVectors[i]);
	ostringstream oCoordsString;
	oCoordsString << "MARKER Position x=" << tvec[0] << "y=" << tvec[1] << "z=" << tvec[2];
	string coordsString = oCoordsString.str();
	putText(frame, coordsString, position, FONT_HERSHEY_COMPLEX_SMALL, 0.5, Scalar(0, 155, 135));
}


//void instrumentScan(vector<Instrument>& instruments, int id)
//{
//	/*Id 0 InocLoop
//	Id 1 Bunsen Burner
//	Id 10 - 19 Ependorph tubes
//	Id 20 - 50 Petri dishes. 
//	*/
////Do subclasses all fit into an array with the superclass as identifier?
//	switch (id)
//	{
//	case 0:
//	{
//		InocLoop loop = InocLoop(id);
//		instruments.push_back(loop);
//		break;
//	}
//	case 1:
//	{
//		Instrument instrument = Instrument(id);
//		instruments.push_back(instrument);
//		break;
//	}
//	default:
//	{
//		Instrument instrument = Instrument(id);
//		instruments.push_back(instrument);
//		break;
//	}
//	}
//}

void detectDistanceLoopToVial(vector<Instrument> instruments, Mat &frame)
{
	vector<Instrument *> localInstrumentVector;
	Instrument *loop = NULL;
	Instrument *vial = NULL;
	double distance;

	for (int i = 0; i < instruments.size(); i++)
	{
		if (instruments[i].arucoId == 0)
		{
			loop =  &instruments[i]; //Trying to create a pointer to the loop. 
			localInstrumentVector.push_back(loop);
		}
		if (instruments[i].arucoId >= 10 && instruments[i].arucoId < 20)
		{
			vial = &instruments[i];
			localInstrumentVector.push_back(vial);
		}
	}

	//Store the points of these instruments
	//The 3d coordinates is the corresponding camera translation vector. 
	
	if (localInstrumentVector.size() > 1)
	{
		distance = euclideanDist(vial->threeDimCoordinates, loop->threeDimCoordinates);
		if (distance < 0.05)
		{
			cout << "Create Event" << endl;
			ostringstream confirm;
			confirm << "Less than 5cm";
			string confirmationString = confirm.str();
			putText(frame, confirmationString, Point(25,25), FONT_HERSHEY_COMPLEX_SMALL, 0.7, Scalar(0, 255, 135));
		}
	
	
		cout << distance << endl;
	}
	

	
}

Point3d tipOfLoop(Mat& frame, Vec3d initPoint, Vec3d rvec, Vec3d tvec, const Mat& camMatrix, const Mat& distCoeff)
{
	vector<Point3d> objPoints;
	vector<Point2d> imgPoints;
	Point3d centerPoint3d = initPoint;
	Point3d tipOfLoop = centerPoint3d;
 	objPoints.push_back(centerPoint3d);
 
	//Projection of the point where the tip ought to be.
	Mat rotMat;
	Rodrigues(rvec, rotMat);
	Mat rotMatTpose = rotMat.t();
	double* tmp = rotMatTpose.ptr<double>(0);
	Point3d camWorldE(tmp[0] * 0.108, tmp[1] * 0.108,	tmp[2] * 0.108);
	tipOfLoop += camWorldE;
	objPoints.push_back(tipOfLoop);
	//End of point projection code


	//It works after setting the rvec and tvec to (0,0,0). I don't really know why.
	projectPoints(objPoints, Vec3d(0,0,0), Vec3d(0, 0, 0), camMatrix, distCoeff, imgPoints);
	 
	//projectPoints(tipOfLoop, rvec, tvec, camMatrix, distCoeff, tipOfLoop2d);
	//The line is not really accurate
	line(frame, imgPoints[0], imgPoints[1], Scalar(0, 255, 255), 1, 1);
	circle(frame, imgPoints[1], 5, Scalar(0,0,255), 0.2);

	return tipOfLoop;
}

void drawTipOfLoopAndBurner(Mat& frame, Mat camMatrix, Mat distCoeff, vector<Point3d> objPoints)
{
	//vector<Point3d> objPoints;
	vector<Point2d> imgPoints;
		
	projectPoints(objPoints, Vec3d(0, 0, 0), Vec3d(0, 0, 0), camMatrix, distCoeff, imgPoints);
	line(frame, imgPoints[0], imgPoints[1], Scalar(0, 255, 255), 1, 1);
	circle(frame, imgPoints[1], 5, Scalar(0, 0, 255), 0.2);
}

bool alreadyScanned(vector<Instrument*> instruments, int id)
{
	for (vector<Instrument *>::iterator it = instruments.begin(); it != instruments.end(); ++it)
	{
		if ((*it)->arucoId == id)
			return true;
	}
	return false;
}


//MAKE THIS RETURN INT WHICH CORRESPONDS TO THE DISPATCH TO SEND
void checkProximity(vector<Instrument*> instruments, Protocol& protocol)
{
 
	Instrument * loop;
	Instrument * target;
	
	loop = instruments[0];
	//loop->createPointOfLoop();

	if (instruments.size() > 1)
	{
		for (int i = 1; i < instruments.size() ; i++) 
		{
		
			target = instruments[i];
			if (loop->madeContact(target)) // gotta test this, check if it reacts appropriately.
			{
				cout << "Contact" << endl;
				cout << loop->arucoId << " Has made contact with " << target->arucoId << endl;
				loop->react(target, protocol);
			}
		
		}
	}


	//Iterate through the instruments list
		//calculate distance to other instruments in the shortenedInstrList
			//If contanct then react(currentInstrument, targetInstrument)
		//just continue if nothing
}

int startWebcamMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions)
{
	Mat frame;
 	vector<int> markerIds;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;
	aruco::DetectorParameters paramters;
	Ptr<aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);
 	vector<Vec3d> rotationVectors, translationVectors;
	vector<int> acceptableInstruments = {0, 15, 27}; //Only add these instruments to the list of instruments
	vector<Instrument*> instruments; //Part of the object creation loop
	
	vector<Point3d> objectPoints = {Point3d(0,0,0), Point3d(0,0,0)};
	
	VideoCapture vid(0);

	
	Protocol currentProt;
	currentProt.start(); //Trying with this. 

	if (!vid.isOpened()) { return -1;}
	namedWindow("Webcam", WINDOW_AUTOSIZE);

	//Need to ensure that the loop is always the first element.

	while (true) //Basically the main loop of when the camera is running. 
	{
		if (!vid.read(frame))
			break;

		aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds);
		aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimensions, cameraMatrix, distanceCoefficients, rotationVectors, translationVectors);
				 
		for (int i = 0; i < markerIds.size(); i++)
		{
			aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.03f);
			
			
			if(!alreadyScanned(instruments, markerIds[i])) // redefine to take pointers and not actual objects
			{
				if (markerIds[i] == 0)
				{ //Trying to make sure that the first instrument is always the loop.
					instruments.insert(instruments.begin(), new Instrument(markerIds[i], translationVectors[i], cameraMatrix, distanceCoefficients));
					instruments[i]->createPointOfLoop();
				}
				else if (find (acceptableInstruments.begin(), acceptableInstruments.end(), markerIds[i])!= acceptableInstruments.end()) 
				{
					instruments.push_back(new Instrument(markerIds[i], translationVectors[i], cameraMatrix, distanceCoefficients));
				}
			}
			else
			{ // there are more markerIds than instruments this bug appears sometimes.
				instruments[i]->threeDimCoordinates = translationVectors[i];

				if (markerIds[i] == 0)  //The loop and Bunsen burner require these values
				{ 
					instruments[i]->rotationVec = rotationVectors[i]; 
					instruments[i]->translationVec = translationVectors[i];
			 		instruments[i]->createPointOfLoop();
					objectPoints[0] = instruments[i]->threeDimCoordinates;
					objectPoints[1] = instruments[i]->loopTip;

					drawTipOfLoopAndBurner(frame, cameraMatrix, distanceCoefficients, objectPoints);
				}
			}

		}


		if (instruments.size() > 1) 
		{ 
			checkProximity(instruments, currentProt);
			currentProt.current_state_ptr->myState(); //This seems to be working
			//cout << instruments[0]->threeDimCoordinates << endl;
			//cout << instruments[1]->threeDimCoordinates << endl;
		}
		
 		aruco::drawDetectedMarkers(frame, markerCorners, markerIds); 
		
		imshow("Webcam", frame);
		if (waitKey(30) >= 0) break;
	}
	return 1;
}

void simulatingStateMachine()
{
	 
	Protocol::start();
 
	while (true)
	{
		char c;
		cout << " c :Options, \n d: Dip loop in Vial, \n q: Query State, \n s: Streak, \n t: Sterilize, \n o: Stow, \n i: Soil " << endl;
		cin >> c;
		switch (c)
		{
		case 'q':
			cout << "Pressed q" << endl;
			Protocol::state<Protocol>().current_state_ptr->myState();
			break;

		case 'c':
			cout << "Pressed c" << endl;
			break;

		case 'd':
			cout << "Pressed d" << endl;
			Protocol::dispatch(LoopDippedInVial());
			break;

		case 's':
			cout << "Pressed s" << endl;
			Protocol::dispatch(Streak());
			break;

		case 't':
			cout << "Pressed t" << endl;
			Protocol::dispatch(LoopSterilize());
			break;

		case 'o':
			cout << "Pressed o" << endl;
			Protocol::dispatch(Stow());
			break;

		case 'i':
			cout << "Pressed i" << endl;
			Protocol::dispatch(Soil());
			break;

		default:
			cout << "NO valid input" << endl;

		}
	}
}

int main(char argv, char** argc)

{
	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
	Mat distanceCoefficients;

	//slight improvement, perhaps the calibration input was all that was wrong. 
	//livestreamCameraCalibration(cameraMatrix, distanceCoefficients);
	loadCameraCalibration("CalibrationInfo", cameraMatrix, distanceCoefficients);

	startWebcamMonitoring(cameraMatrix, distanceCoefficients, arucoSquareDimension);
	//simulatingStateMachine();

	return 0;
}