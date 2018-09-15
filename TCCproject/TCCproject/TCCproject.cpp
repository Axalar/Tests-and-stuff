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


void segmentation(Mat img) {
	
	cv::Mat hsv, HSVthresh;
	cv::cvtColor(img, hsv, CV_BGR2HSV);

	std::vector<cv::Mat> channels;
	cv::split(hsv, channels);

	cv::Mat H = channels[0];
	cv::Mat S = channels[1];
	cv::Mat V = channels[2];

	float low_H = 25; //25.2
	float low_S = 200;
	float low_V = 0;
	float high_H = 26; //25.5
	float high_S = 255;
	float high_V = 255;

	inRange(hsv, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), HSVthresh);

	dilate(HSVthresh, HSVthresh, cv::Mat());
	erode(HSVthresh, HSVthresh, cv::Mat());
	
	cv::Mat shiftedH = H.clone();
	int shift = 156; // in openCV hue values go from 0 to 180 (so have to be doubled to get to 0 .. 360) because of byte range from 0 to 255
	for (int j = 0; j < shiftedH.rows; ++j)
		for (int i = 0; i < shiftedH.cols; ++i)
		{
			shiftedH.at<unsigned char>(j, i) = (shiftedH.at<unsigned char>(j, i) + shift) % 180;
		}
	
	cv::Mat cannyH;
	cv::Canny(H, cannyH, 50, 150);

	cv::Mat cannyS;
	cv::Canny(S, cannyS, 190, 200);

	cv::Mat cannyHSVt;
	cv::Canny(HSVthresh, cannyHSVt, 50, 150);

	//namedWindow("Display hue", WINDOW_AUTOSIZE);
	//imshow("Display hue", H);

	//namedWindow("Display shiftedH", WINDOW_AUTOSIZE);
	//imshow("Display shiftedH", shiftedH);

	//namedWindow("Display cannyH", WINDOW_AUTOSIZE);
	//imshow("Display cannyH", cannyH);

	namedWindow("Display HSVthresh", WINDOW_AUTOSIZE);
	imshow("Display HSVthresh", HSVthresh);

	namedWindow("Display cannyHSVt", WINDOW_AUTOSIZE);
	imshow("Display cannyHSVt", cannyHSVt);

	/*
	namedWindow("Display saturation", WINDOW_AUTOSIZE);
	imshow("Display saturation", S);
	
	//namedWindow("Display value", WINDOW_AUTOSIZE);
	//imshow("Display value", V);

	namedWindow("Display cannyS", WINDOW_AUTOSIZE);
	imshow("Display cannyS", cannyS);

	Mat imgt;
	adaptiveThreshold(S, imgt, 225, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 15, 2);

	erode(imgt, imgt, cv::Mat());
	dilate(imgt, imgt, cv::Mat());
	erode(imgt, imgt, cv::Mat());
	dilate(imgt, imgt, cv::Mat());
	erode(imgt, imgt, cv::Mat());

	namedWindow("Display threshold", WINDOW_AUTOSIZE);
	imshow("Display threshold", imgt);
	*/
}


int main() {

	string datasetpath("C:\\Users\\Usuario\\source\\repos\\Tests-and-stuff\\TCCproject\\dataset_tcc");
	string dataset("\\basler_melhor");
	string imgfile("\\img25.bmp");
	imgfile = datasetpath + dataset + imgfile;


	Mat image;
	image = imread(imgfile, IMREAD_COLOR); // Read the file

	if (!image.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	segmentation(image);

	namedWindow("Display window", WINDOW_AUTOSIZE); // Create a window for display.
	imshow("Display window", image); // Show our image inside it.

	waitKey(0); // Wait for a keystroke in the window
	return 0;

}



