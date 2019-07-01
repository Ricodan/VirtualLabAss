
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"

#include "Calibration.h"
#include "Instrument.h"
#include "Protocol.h" 

#include <stdint.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <ctime>

using namespace std;
using namespace cv;

//Distances are based in meters.
const int fps = 20;
const float calibrationSquareDimension = 0.024f;
const float arucoSquareDimension = 0.01f; 
const float arucoSquareDimensionSecondSet = 0.0123;
const Size chessboardDimensions = Size(9, 6);



double euclideanDist(Vec3d pointA, Vec3d pointB)
{
	double dist = sqrt(pow(pointA[0] - pointB[0], 2) + pow(pointA[1] - pointB[1], 2) + pow(pointA[2] - pointB[2], 2));
	return dist;

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
	putText(frame, coordsString, position, FONT_HERSHEY_PLAIN, 0.5, Scalar(0, 155, 135));
}

void screenText(Mat& frame, ostringstream& oString, string currentState)
{
	//The line below is how I was calling this function from the loop in startWebcamMonitoring
	//showCoordsAtPos(frame, Point(markerCorners[i][2].x, markerCorners[i][2].y), translationVectors[i]);
	/* s.str("");
	   s.clear(); */
	string outString; 

 
	if (currentState == "Soiled" || currentState == "Base State")
	{
		oString << "RESTART!";
		outString = oString.str();
		putText(frame, outString, Point(10, 400), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255));
		oString.str("");
	}
	else
	{
		oString << currentState;
		outString = oString.str();
		putText(frame, outString, Point(10, 400), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 150));
		oString.str("");
	}
}

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
			//Trying to create a pointer to the loop. 
			loop =  &instruments[i]; 
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
	Point3d prolongPoint(tmp[0] * 0.108, tmp[1] * 0.108,	tmp[2] * 0.108);
	tipOfLoop += prolongPoint;
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


struct instrumentData
{
	Instrument* instrument;
	Vec3d rvec, tvec;
	int* counter = new int(0);
};

void checkProximity(map<int, instrumentData*>* instrumentsMap, Protocol& protocol)
{
	instrumentData* loop;
	instrumentData* stowP;
	instrumentData* target;

	loop = (*instrumentsMap)[0]; 
	stowP = (*instrumentsMap)[11];
	loop->instrument->createPointOfLoop();
		
	for (auto const& inst : (*instrumentsMap)) 
	{
		//As long as the instrument is neither a loop nor a location
		if(inst.first != 0 && inst.first != 11)
		{
			target = inst.second; //This will contain instrument info, but not the key
		
			if (loop->instrument->madeContact(target->instrument)) 
			{
				//cout << loop->instrument->arucoId << " Has made contact with " << target->instrument->arucoId << endl;
				*(target->counter) += 1;
				//cout << "Counter: " << *(target->counter) << endl;
					
				if (*(target->counter) > 6 && target->instrument->hasDisengaged == true)
				{
					loop->instrument->react(target->instrument, protocol);
					*(target->counter) = 0;
				}
			}
			else if (stowP->instrument->stowContact(target->instrument))
			{
				stowP->instrument->stowPointReact(target->instrument, protocol);
			}
			else
			{
				*(target->counter) = 0;
			}
		}
		
	}
}

void storeMarkersMap(map<int, instrumentData*>* markerMap, Mat& frame, vector<int> markerIds, vector<int> acceptableInstruments, vector<Vec3d> rotationVectors, vector<Vec3d> translationVectors, Mat cameraMatrix, Mat distanceCoefficients)
{
	//Most of the time the ids will already exist, so better to optimize for that
	//Find indexes of undesired markers detected and remove the equivalent elements at the same index from other instruments. 

	for (int i = 0; i < markerIds.size(); i++)
	{
		int key = markerIds[i];
	
		if (find(acceptableInstruments.begin(), acceptableInstruments.end(), markerIds[i]) != acceptableInstruments.end())
		{
			if (markerMap->count(markerIds[i]) > 0)
			{	
				// key  exists, do something with iter->second (the value)
				(*markerMap)[key]->rvec = rotationVectors[i];
				(*markerMap)[key]->tvec = translationVectors[i];
				(*markerMap)[key]->instrument->threeDimCoordinates = translationVectors[i];
				(*markerMap)[key]->instrument->rotationVec = rotationVectors[i];
				//aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.03f);
			}
			else
			{ 
				cout << "Created new instrument" << endl;
				instrumentData* newInst = new instrumentData{ 
					new Instrument(markerIds[i], translationVectors[i], cameraMatrix, distanceCoefficients),
					rotationVectors[i], translationVectors[i]};
				markerMap->insert(map<int, instrumentData*>::value_type(key, newInst));
				newInst->instrument->threeDimCoordinates = translationVectors[i];
			}
		}
	}
}



struct testData
{
	time_t start, end;
	int frameCounter;
};

void performanceTest(Mat& frame, vector<int> markerIds, testData* testInfo, clock_t* timer, double* fps)
{
	ostringstream stringBuilder;
	testInfo->frameCounter += 1;
	
	if (clock()/ CLOCKS_PER_SEC - *timer > 1)
	{
		time(&testInfo->end);
		double seconds = difftime(testInfo->end, testInfo->start);
		*fps = testInfo->frameCounter / seconds;
		cout << "The current estimated FPS is: " << *fps << endl;
		*timer = clock() / CLOCKS_PER_SEC;
		time(&testInfo->start);
		testInfo->frameCounter = 0;
	}

	string outputString;
	stringBuilder << "The current estimated FPS is: " << *fps;
	outputString = stringBuilder.str();
	putText(frame, outputString, Point(10, 10), FONT_HERSHEY_PLAIN, 1, Scalar(0, 155, 135));
}

void distanceDetectionTest(Mat& frame, vector<int> markers, clock_t* timer, ostringstream* stringBuilder, double* misses, double* hits, double* frames)
{
	*frames += 1;
	if (markers.size() == 0)
	{
		*misses += 1;
	}
	if (markers.size() > 0)
	{
		*hits += 1;
	}

	if (clock() / CLOCKS_PER_SEC - *timer > 1)
	{ //Every second check what percentage have missed and hit
		(*stringBuilder).str("");
		(*stringBuilder).clear();

		cout << "% Hit: " << *misses/(*frames) *100 << endl;
		cout << "% Miss: " << *hits/ (*frames) *100 << endl;

		*stringBuilder << "% Hit: " << *hits / (*frames) * 100;
		*stringBuilder << "% Miss: " << *misses / (*frames) * 100; 
		*timer = clock() / CLOCKS_PER_SEC;
		*frames = 0;
		*hits = 0;
		*misses = 0;
	}
	string outputString;
	outputString = (*stringBuilder).str();
	putText(frame, outputString, Point(10, 20), FONT_HERSHEY_PLAIN, 1, Scalar(116, 240, 20));
}

Mat createThreshold(Mat& frame)
{
	Mat grayScaleMat(frame.size(), CV_8U);

	cvtColor(frame, grayScaleMat, COLOR_BGR2GRAY);

	//Binary Image
	Mat binaryMat(grayScaleMat.size(), grayScaleMat.type());

	//Apply Thresholding

	//threshold(grayScaleMat, binaryMat, 100, 200, THRESH_OTSU);
	adaptiveThreshold(grayScaleMat, binaryMat, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 5, 2);
	//namedWindow("GrayOutPut", WINDOW_AUTOSIZE);
	imshow("Output", binaryMat);


	return binaryMat;
}

int mainFlow(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions)
{
	ostringstream oString;
	
	map< int, instrumentData*> instrumentMap;
	Mat frame;
	Mat thresholdedMat;
	vector<int> markerIds;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;
	Ptr<aruco::DetectorParameters> parameters = aruco::DetectorParameters::create();
	Ptr<aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);
	vector<Vec3d> rotationVectors, translationVectors;
	vector<int> acceptableInstruments = {0, 11, 15, 25, 27, 28 };
	vector<Point3d> objectPoints = { Point3d(0,0,0), Point3d(0,0,0) };
	Protocol currentProt;
	currentProt.start();

	//parameters->create();
	//parameters->adaptiveThreshWinSizeMax = 23;
	//parameters->adaptiveThreshWinSizeMin = 3;
	//parameters->adaptiveThreshWinSizeStep = 10;

	//Variables used in testing
	clock_t timeElapsed;
	testData testInfo;
	timeElapsed = clock() / CLOCKS_PER_SEC;
	time(&testInfo.start);
	double fps = 0;
	double* fpsPtr = &fps;
	//Distance check test variables
	double frames, hits, miss = 0;
	double* framesP = &frames;
	double* hitsP = &hits;
	double* missP = &miss;
	ostringstream stringStream;
	ostringstream* stringBuilder = &stringStream;


	VideoCapture vid(0);
	//Need to calibrate first with this resolution to ensure correct results´. 
	//vid.set(3, 1280);
	//vid.set(4, 720);

	if (!vid.isOpened()) { return -1; }
	namedWindow("Webcam", WINDOW_AUTOSIZE);

	//Initiliazing with the loop already in the hashMap
	instrumentData loop, stowPlace;
	loop.instrument = new Instrument(0, Vec3d(0, 0, 0), cameraMatrix, distanceCoefficients);
	stowPlace.instrument = new Instrument(11, Vec3d(2, 2, 2), cameraMatrix, distanceCoefficients);
	instrumentMap[0] = &loop;
	instrumentMap[11] = &stowPlace;
	

	while (true) 
	{
		if (!vid.read(frame))
			break;
	 
		 
		aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds, parameters);
		aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimensions, cameraMatrix, distanceCoefficients, rotationVectors, translationVectors);

		storeMarkersMap(&instrumentMap, frame, markerIds, acceptableInstruments, rotationVectors, translationVectors, cameraMatrix, distanceCoefficients);
	 	checkProximity(&instrumentMap, currentProt);

		objectPoints[0] = loop.instrument->threeDimCoordinates;
		objectPoints[1] = loop.instrument->loopTip;
		//drawTipOfLoopAndBurner(frame, cameraMatrix, distanceCoefficients, objectPoints);
		aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
		screenText(frame, oString,  currentProt.current_state_ptr->myState());
		
		//performanceTest(frame, markerIds, &testInfo, &timeElapsed, fpsPtr);
		//distanceDetectionTest(frame, markerIds, &timeElapsed, stringBuilder, missP, hitsP, framesP);
		  

		cv::imshow("Webcam", frame);
		if (waitKey(30) >= 0) break;
	}
	return 1;
}


int main(char argv, char** argc)

{
	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
	Mat distanceCoefficients;
	
	//livestreamCameraCalibration(cameraMatrix, distanceCoefficients);
	loadCameraCalibration("CalibrationInfo", cameraMatrix, distanceCoefficients);
	mainFlow(cameraMatrix, distanceCoefficients, arucoSquareDimensionSecondSet);

	return 0;
}