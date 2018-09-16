// TCCproject.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;


void locatePlaca(Mat img) {

	Mat imgs;
	cvtColor(img, imgs, cv::COLOR_RGB2GRAY);

	namedWindow("Display grayscale", WINDOW_AUTOSIZE);
	imshow("Display grayscale", imgs);

	Mat imgt;
	//threshold(imgs, imgt, 177, 225, THRESH_OTSU);
	//adaptiveThreshold(imgs, imgt, 225, BORDER_REPLICATE, THRESH_BINARY_INV, 101, 2);
	adaptiveThreshold(imgs, imgt, 225, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 15, 2);
	namedWindow("Display threshold", WINDOW_AUTOSIZE);
	imshow("Display threshold", imgt);
}

// Com gaussiana
void segmentationCG(Mat img) {
	
	cv::Mat hsv, HSVthresh;
	cv::cvtColor(img, hsv, CV_BGR2HSV);

	float low_H = 25; //25.2
	float low_S = 150; //180
	float low_V = 0;
	float high_H = 26; //25.5
	float high_S = 255; //255
	float high_V = 255;
	
	GaussianBlur(hsv, hsv, Size(11, 11), 0, 0);

	inRange(hsv, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), HSVthresh);

	namedWindow("Display HSVthreshFbase", WINDOW_AUTOSIZE);
	imshow("Display HSVthreshFbase", HSVthresh);

	/*
	erode(HSVthresh, HSVthresh, getStructuringElement(MORPH_ELLIPSE,
		Size(2 * 6 + 1, 2 * 6 + 1),
		Point(6, 6)));
	dilate(HSVthresh, HSVthresh, getStructuringElement(MORPH_RECT,
		Size(2 * 6 + 1, 2 * 6 + 1),
		Point(6, 6)));
	*/
	
	
	namedWindow("Display HSVthreshF", WINDOW_AUTOSIZE);
	imshow("Display HSVthreshF", HSVthresh);
	
	//cv::Mat cannyHSVt;
	//cv::Canny(HSVthresh, cannyHSVt, 50, 150);

	//namedWindow("Display cannyHSVtF", WINDOW_AUTOSIZE);
	//imshow("Display cannyHSVtF", cannyHSVt);
	
}

// Sem gaussiana
void segmentationSG(Mat img) {

	cv::Mat hsv, HSVthresh;
	cv::cvtColor(img, hsv, CV_BGR2HSV);

	float low_H = 25; //25.2
	float low_S = 150; //180
	float low_V = 0;
	float high_H = 26; //25.5
	float high_S = 255; //255
	float high_V = 255;
	
	inRange(hsv, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), HSVthresh);

	//namedWindow("Display HSVthreshBASE", WINDOW_AUTOSIZE);
	//imshow("Display HSVthreshBASE", HSVthresh);

	// Mais iterações são mais eficiêntes que janelas maiores

	erode(HSVthresh, HSVthresh, getStructuringElement(MORPH_ELLIPSE,
		Size(2 * 1 + 1, 2 * 1 + 1),
		Point(1, 1)));
	dilate(HSVthresh, HSVthresh, getStructuringElement(MORPH_ELLIPSE,
		Size(2 * 4 + 1, 2 * 4 + 1),
		Point(4, 4)));
	erode(HSVthresh, HSVthresh, getStructuringElement(MORPH_ELLIPSE,
		Size(2 * 6 + 1, 2 * 6 + 1),
		Point(6, 6)));
	dilate(HSVthresh, HSVthresh, getStructuringElement(MORPH_RECT,
		Size(2 * 6 + 1, 2 * 6 + 1),
		Point(6, 6)));

	//namedWindow("Display HSVthresh", WINDOW_AUTOSIZE);
	//imshow("Display HSVthresh", HSVthresh);

	cv::Mat cannyHSVt;
	cv::Canny(HSVthresh, cannyHSVt, 50, 150);

	//namedWindow("Display cannyHSVt", WINDOW_AUTOSIZE);
	//imshow("Display cannyHSVt", cannyHSVt);
	
}


int main() {

	string datasetpath("C:\\Users\\Usuario\\source\\repos\\Tests-and-stuff\\TCCproject\\dataset_tcc");
	string dataset("\\basler_melhor");
	string imgfile("\\img02.bmp");
	imgfile = datasetpath + dataset + imgfile;


	Mat image;
	image = imread(imgfile, IMREAD_COLOR); // Read the file

	if (!image.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	segmentationCG(image);

	//namedWindow("Display window", WINDOW_AUTOSIZE); // Create a window for display.
	//imshow("Display window", image); // Show our image inside it.

	waitKey(0); // Wait for a keystroke in the window
	return 0;

}



