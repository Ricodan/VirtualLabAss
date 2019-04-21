#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

#include <stdint.h>
using namespace std;
using namespace cv;

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
int main(int argv, char* argc)
{
	//Lesson 1
	/*Mat A;
	A = Mat::zeros(100, 100, CV_8U);
	namedWindow("x", WINDOW_AUTOSIZE);
	imshow("x", A);
	waitKey(0);
	return 0;*/

	//Lesson 2
	/*Mat testColor = imread("me.jpg", 1);
	Mat testGray = imread("me.jpg", IMREAD_GRAYSCALE);

	imshow("gray", testGray);
	imshow("color", testColor);

	imwrite("outputGray.jpg", testGray);
	waitKey();*/

	//Lesson 3
	/*Mat file1 = imread("me.jpg", IMREAD_UNCHANGED);
	Mat file2 = imread("me.jpg", IMREAD_GRAYSCALE);

	namedWindow("color", WINDOW_FREERATIO);
	namedWindow("Fixed", WINDOW_AUTOSIZE);

	imshow("color", file1);
	imshow("Fixed", file2);

	resizeWindow("color", file1.cols/2, file1.rows/2);
	resizeWindow("Fixed", file1.cols/2, file1.rows/2);
	
	moveWindow("color", 300, 300);

	waitKey();*/

	//Lesson 4
	//Mat original = imread("me.jpg", IMREAD_COLOR);
	//Mat modified = imread("me.jpg", IMREAD_COLOR);

	//for (int r = 0; r < modified.rows; r++) 
	//{
	//	for (int c = 0; c < modified.cols; c++)
	//	{
	//		modified.at<cv::Vec3b>(r, c)[0] = modified.at<Vec3b>(r, c)[0] * 0; //The blue channel comes first. It's BGR in OpenCV
	//	}
	//}
	//
	//imshow("Original", original);
	//imshow("Modified", modified);
	//waitKey();


	//Lesson 5 Split and Merge

	/*Mat original = imread("me.jpg", IMREAD_COLOR);
	Mat splitChannels[3];

	split(original, splitChannels);

	imshow("B", splitChannels[0]);
	imshow("G", splitChannels[1]);
	imshow("R", splitChannels[2]);


		
	splitChannels[2] = Mat::zeros(splitChannels[2].size(), CV_8U);

	Mat output;

	merge(splitChannels, 3, output);

	imshow("merged", output);

	waitKey();*/


	//Lesson 6 DFT of a Grayscale Imag

	Mat original = imread("me.jpg", IMREAD_GRAYSCALE);
	//Normalising the image and convert to Floating point
	Mat originalFloat;
	original.convertTo(originalFloat, CV_32FC1, 1.0 / 255.0); 
	
	Mat dftOfOriginal;

	//Lesson 7 Visualizing the DFT.
	takeDFT(originalFloat, dftOfOriginal);
	showDFT(dftOfOriginal);
	
	//Lesson 8 Recentering the DFT. 
	Mat invertedDFT;
	invertDFT(dftOfOriginal, invertedDFT);

	imshow("InvertDFT Result", invertedDFT);
	waitKey();

}