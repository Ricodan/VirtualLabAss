#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2\imgcodecs.hpp"
#include "opencv2\imgproc.hpp"
#include "opencv2\aruco.hpp"
#include "opencv2\calib3d.hpp"
 
#include "opencv2/highgui.hpp"
#include "Instrument.h"

#include <stdint.h>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;
const int fps = 20;

void takeDFT(Mat& source, Mat& destination)
{
	Mat originalComplex[2] = { source, Mat::zeros(source.size(), CV_32F) }; // Need to hold imaginary part and real part.
	Mat dftReady;
	merge(originalComplex, 2, dftReady);
	Mat dftOfOriginal;
	dft(dftReady, dftOfOriginal, DFT_COMPLEX_OUTPUT);
	destination = dftOfOriginal;
}

void recenterDFT(Mat& source)
{
	int centerX = source.cols / 2;
	int centerY = source.rows / 2;

	Mat q1(source, Rect(0, 0, centerX, centerY));
	Mat q2(source, Rect(centerX, 0, centerX, centerY));
	Mat q3(source, Rect(0, centerY, centerX, centerY));
	Mat q4(source, Rect(centerX, centerY, centerX, centerY));

	Mat swapMap;

	q1.copyTo(swapMap);
	q4.copyTo(q1);
	swapMap.copyTo(q4);

	q2.copyTo(swapMap);
	q3.copyTo(q2);
	swapMap.copyTo(q3);

}

void showDFT(Mat& source)
{
	//NOTE: Do not use the information generated here for processing. This is mostly for human visualization. 
	Mat splitArray[2] = { Mat::zeros(source.size(), CV_32F), Mat::zeros(source.size(), CV_32F) };

	split(source, splitArray);

	Mat dftMagnitude;

	magnitude(splitArray[0], splitArray[1], dftMagnitude);

	dftMagnitude += Scalar::all(1);
	log(dftMagnitude, dftMagnitude);
	//CV_MINMAX doesn't exist in 4.0, perhaps something else in 4.0. 
	normalize(dftMagnitude, dftMagnitude, 0, 1, NORM_MINMAX);
	recenterDFT(dftMagnitude);
	imshow("DFT", dftMagnitude);
	waitKey();
}

void invertDFT(Mat& source, Mat& destination)
{
	Mat inverse;
	dft(source, inverse, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);

	destination = inverse;

}

void createGaussian(Size& size, Mat& output, int uX, int uY, float sigmaX, float sigmaY, float amplitude = 1.0f)
{
	Mat temp = Mat(size, CV_32F);

	for (int r = 0; r < size.height; r++)
	{
		for (int c = 0; c < size.width; c++)
		{
			float x = ((c-uX)*((float)c -uX)) / (2.0f * sigmaX *sigmaX);
			float y = ((r - uY)*((float)r - uY)) / (2.0f * sigmaY *sigmaY);
			float value = amplitude * exp(-(x + y));
			temp.at<float>(r, c) = value;
		}
	}
	normalize(temp, temp, 0.0f, 1.0f, NORM_MINMAX);
	output = temp;
}

 
/*
int startWebcamMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions)
{

	bool* inCurrentProcess;
	int counter = 0;
	int* counterPtr = &counter;
	Mat frame;
	vector<int> markerIds;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;
	aruco::DetectorParameters paramters;
	Ptr<aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);
	vector<Vec3d> rotationVectors, translationVectors;
	vector<int> acceptableInstruments = { 0, 15, 27 }; //Only add these instruments to the list of instruments
	vector<Instrument*> instruments; //Part of the object creation loop

	vector<Point3d> objectPoints = { Point3d(0,0,0), Point3d(0,0,0) };

	VideoCapture vid(0);


	Protocol currentProt;
	currentProt.start(); //Trying with this. 

	if (!vid.isOpened()) { return -1; }
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
			//aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.03f);


			if (!alreadyScanned(instruments, markerIds[i])) // redefine to take pointers and not actual objects
			{
				if (markerIds[i] == 0)
				{ //Trying to make sure that the first instrument is always the loop.
					instruments.insert(instruments.begin(), new Instrument(markerIds[i], translationVectors[i], cameraMatrix, distanceCoefficients));
					instruments[i]->createPointOfLoop();
				}
				else if (find(acceptableInstruments.begin(), acceptableInstruments.end(), markerIds[i]) != acceptableInstruments.end())
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
			checkProximity(instruments, currentProt, counterPtr);
			currentProt.current_state_ptr->myState(); //This seems to be working
													  //cout << instruments[0]->threeDimCoordinates << endl;
													  //cout << instruments[1]->threeDimCoordinates << endl;
		}

		aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
		//screenText(frame, currentProt.current_state_ptr->myState() );

		imshow("Webcam", frame);
		if (waitKey(30) >= 0) break;
	}
	return 1;
}

*/


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

void checkProximity(vector<Instrument*> instruments, Protocol& protocol, int* counter)
{
	//Try first without regard for which instrument on is reacting with. 	 
	Instrument * loop;
	Instrument * target;

	loop = instruments[0];
	//loop->createPointOfLoop();

	if (instruments.size() > 1)
	{
		for (int i = 1; i < instruments.size(); i++)
		{
			target = instruments[i];
			if (loop->madeContact(target)) // gotta test this, check if it reacts appropriately.
			{
				//cout << "Contact" << endl;
				cout << loop->arucoId << " Has made contact with " << target->arucoId << endl;
				*counter += 1;
				if (*counter > 10)
				{
					cout << "Ract Called" << endl;
					loop->react(target, protocol);
					*counter = 0;
				}
			}
			else
			{
				*counter = 0;
			}
		}
	}
	//Iterate through the instruments list
	//calculate distance to other instruments in the shortenedInstrList
	//If contanct then react(currentInstrument, targetInstrument)
	//just continue if nothing
}




//void createArucoMarkers()
//{
//	Mat outputMarker;
//
//	Ptr<aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);
//
//	for (int i = 0; i < 50; i++)
//	{
//		aruco::drawMarker(markerDictionary, i, 500, outputMarker, 1);
//		ostringstream convert;
//		string imageName = "4x4Marker_";
//		convert << imageName << i << ".jpg";
//		imwrite(convert.str(), outputMarker);
//
//	}
//
//}
//
//int main(int argv, char* argc)
//{
//	//Lesson 1
//	/*Mat A;
//	A = Mat::zeros(100, 100, CV_8U);
//	namedWindow("x", WINDOW_AUTOSIZE);
//	imshow("x", A);
//	waitKey(0);
//	return 0;*/
//
//	//Lesson 2
//	/*Mat testColor = imread("me.jpg", 1);
//	Mat testGray = imread("me.jpg", IMREAD_GRAYSCALE);
//
//	imshow("gray", testGray);
//	imshow("color", testColor);
//
//	imwrite("outputGray.jpg", testGray);
//	waitKey();*/
//
//	//Lesson 3
//	/*Mat file1 = imread("me.jpg", IMREAD_UNCHANGED);
//	Mat file2 = imread("me.jpg", IMREAD_GRAYSCALE);
//
//	namedWindow("color", WINDOW_FREERATIO);
//	namedWindow("Fixed", WINDOW_AUTOSIZE);
//
//	imshow("color", file1);
//	imshow("Fixed", file2);
//
//	resizeWindow("color", file1.cols/2, file1.rows/2);
//	resizeWindow("Fixed", file1.cols/2, file1.rows/2);
//	
//	moveWindow("color", 300, 300);
//
//	waitKey();*/
//
//	//Lesson 4
//	//Mat original = imread("me.jpg", IMREAD_COLOR);
//	//Mat modified = imread("me.jpg", IMREAD_COLOR);
//
//	//for (int r = 0; r < modified.rows; r++) 
//	//{
//	//	for (int c = 0; c < modified.cols; c++)
//	//	{
//	//		modified.at<cv::Vec3b>(r, c)[0] = modified.at<Vec3b>(r, c)[0] * 0; //The blue channel comes first. It's BGR in OpenCV
//	//	}
//	//}
//	//
//	//imshow("Original", original);
//	//imshow("Modified", modified);
//	//waitKey();
//
//	//Lesson 5 Split and Merge
//
//	/*Mat original = imread("me.jpg", IMREAD_COLOR);
//	Mat splitChannels[3];
//
//	split(original, splitChannels);
//
//	imshow("B", splitChannels[0]);
//	imshow("G", splitChannels[1]);
//	imshow("R", splitChannels[2]);
//
//
//		
//	splitChannels[2] = Mat::zeros(splitChannels[2].size(), CV_8U);
//
//	Mat output;
//
//	merge(splitChannels, 3, output);
//
//	imshow("merged", output);
//
//	waitKey();*/
//
//	//Lesson 6 DFT of a Grayscale Imag
//
//	//Mat original = imread("me.jpg", IMREAD_GRAYSCALE);
//	////Normalising the image and convert to Floating point
//	//Mat originalFloat;
//	//original.convertTo(originalFloat, CV_32FC1, 1.0 / 255.0); 
//	//Mat dftOfOriginal;
//
//	////Lesson 7 Visualizing the DFT.
//	//takeDFT(originalFloat, dftOfOriginal);
//	//showDFT(dftOfOriginal);
//	//
//	////Lesson 8 Recentering the DFT. 
//	//Mat invertedDFT;
//	//invertDFT(dftOfOriginal, invertedDFT);
//	//imshow("InvertDFT Result", invertedDFT);
//	//waitKey();
//
//	// Lesson 10 2D Gaussian Blur
//	/*Mat output;
//	createGaussian(Size(256, 256), output, 256 / 2, 256 / 2, 10, 10);
//	imshow("Output", output);
//	waitKey();*/
//
//	//Lesson 11 Video Capture
//	/*Mat frame;
//	VideoCapture vid(0);
//
//	if (!vid.isOpened())
//	{
//		return -1;
//	}
//	while (vid.read(frame))
//	{
//		imshow("Stream", frame);
//
//		if (waitKey(1000 / fps) >= 0)
//			break;
//	}*/
//
//	
//
//}