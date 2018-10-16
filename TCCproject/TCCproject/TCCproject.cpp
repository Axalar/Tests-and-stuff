// TCCproject.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>

using namespace cv;
using namespace std;

// Com gaussiana
Mat segmentationCG(Mat img) {
	
	cv::Mat hsv, HSVthresh;
	cv::cvtColor(img, hsv, CV_BGR2HSV);

	float low_H = 25; //25
	float low_S = 50; //150
	float low_V = 0;
	float high_H = 35; //26
	float high_S = 255; //255
	float high_V = 255;
	
	GaussianBlur(hsv, hsv, Size(11, 11), 0, 0);

	inRange(hsv, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), HSVthresh);

	namedWindow("Display HSVthreshFbase", WINDOW_AUTOSIZE);
	imshow("Display HSVthreshFbase", HSVthresh);

	erode(HSVthresh, HSVthresh, Mat(), Point(-1, -1), 6);
	dilate(HSVthresh, HSVthresh, Mat(), Point(-1, -1), 4);
	
	namedWindow("Display HSVthreshF", WINDOW_AUTOSIZE);
	imshow("Display HSVthreshF", HSVthresh);
	
	//cv::Mat cannyHSVt;
	//cv::Canny(HSVthresh, cannyHSVt, 50, 150);

	//namedWindow("Display cannyHSVtF", WINDOW_AUTOSIZE);
	//imshow("Display cannyHSVtF", cannyHSVt);
	
	return HSVthresh;
}

// Sem gaussiana
void segmentationSG(Mat img) {

	// Conversão para mask baseada em cor e saturação
	cv::Mat hsv, HSVthresh;
	cv::cvtColor(img, hsv, CV_BGR2HSV);

	float low_H = 25; //25
	float low_S = 170; //150 basler ou 50 webcam
	float low_V = 0;
	float high_H = 26; //26 basler ou 35 webcam
	float high_S = 255; //255
	float high_V = 255;
	
	inRange(hsv, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), HSVthresh);

	namedWindow("Display HSVthreshBASE", WINDOW_AUTOSIZE);
	imshow("Display HSVthreshBASE", HSVthresh);

	//------------------------------------------------------------------------------
	// Normalizando e aplicando operação morph. open

	Mat normed, opened;
	normalize(HSVthresh, normed, 0, 255, NORM_MINMAX, CV_8UC1);

	Mat structure = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	morphologyEx(normed, opened, MORPH_OPEN, structure);
	morphologyEx(opened, opened, MORPH_CLOSE, structure);

	namedWindow("Display opened", WINDOW_AUTOSIZE);
	imshow("Display opened", opened);

	//------------------------------------------------------------------------------
	// Encontrando contornos

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(opened, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	// draw
	Mat drawing = Mat::zeros(opened.size(), CV_8UC3);
	for (size_t i = 0; i < contours.size(); i++)
	{
		drawContours(img, contours, (int)i, Scalar(255,255,0), 2, 8, hierarchy, 0, Point());
	}
	// show
	//namedWindow("Display drawing", WINDOW_AUTOSIZE);
	//imshow("Display drawing", img);

	//------------------------------------------------------------------------------
	// aproximação polygonal dos contornos

	vector<vector<Point>> aproxpolys;
	aproxpolys.resize(contours.size());
	for (size_t k = 0; k < contours.size(); k++)
		approxPolyDP(contours[k], aproxpolys[k], 10, true);
	// draw
	Mat drawingaprox = Mat::zeros(opened.size(), CV_8UC3);
	for (size_t i = 0; i < aproxpolys.size(); i++)
	{
		drawContours(img, aproxpolys, (int)i, Scalar(255, 0, 255), 2, 8, hierarchy, 0, Point());
	}
	// show
	//namedWindow("Display drawingaprox", WINDOW_AUTOSIZE);
	//imshow("Display drawingaprox", img);

	//------------------------------------------------------------------------------
	// min area rect

	// esta parte precisa ser alterada para filtrar contornos por área
	/*
	int biggestContourIdx = -1;
	float biggestContourArea = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		float ctArea = contourArea(contours[i]);
		if (ctArea > biggestContourArea)
		{
			biggestContourArea = ctArea;
			biggestContourIdx = i;
		}
	}
	*/

	// compute the rotated bounding rect of the biggest contour! (this is the part that does what you want/need)
	vector<RotatedRect> boundingBox;
	boundingBox.resize(contours.size());
	for (int i = 0; i < contours.size(); i++)
		boundingBox[i] = minAreaRect(contours[i]);
	// one thing to remark: this will compute the OUTER boundary box, so maybe you have to erode/dilate if you want something between the ragged lines

	// draw the rotated rect
	Point2f corners[4];
	for (int i = 0; i < boundingBox.size(); i++)
	{
		boundingBox[i].points(corners);
		line(img, corners[0], corners[1], Scalar(0, 255, 0), 1, LINE_AA);
		line(img, corners[1], corners[2], Scalar(0, 255, 0), 1, LINE_AA);
		line(img, corners[2], corners[3], Scalar(0, 255, 0), 1, LINE_AA);
		line(img, corners[3], corners[0], Scalar(0, 255, 0), 1, LINE_AA);
	}

	namedWindow("Display minrect", WINDOW_AUTOSIZE);
	imshow("Display minrect", img);



}

void locatePlaca(Mat img) {
	
	// Blobs
	/*
	vector<KeyPoint> keypoints;

	// Set up the detector parameters.
	SimpleBlobDetector::Params params;

	// Filter by Color
	params.filterByColor = false;
	params.blobColor = 255;

	// Filter by Area.
	params.filterByArea = true;
	params.minArea = 1000;

	// Filter by Circularity
	params.filterByCircularity = false;
	//params.minCircularity = 0.0;

	// Filter by Convexity
	params.filterByConvexity = false;
	//params.minConvexity = 0.9;

	// Filter by Inertia
	params.filterByInertia = false;
	//params.minInertiaRatio = 0.01;

	// Set up detector with params
	Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);

	detector->detect(img, keypoints);

	// Draw detected blobs as red circles.
	// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
	Mat im_with_keypoints;
	drawKeypoints(img, keypoints, im_with_keypoints, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	// Show blobs
	imshow("keypoints", im_with_keypoints);
	*/

	// Harris Corners
	/*
	int thresh = 200;

	Mat dst;// = Mat::zeros(img.size(), CV_32FC1);
	cornerHarris(img, dst, 2, 3, 0.04);
	//namedWindow("Display corners", WINDOW_AUTOSIZE);
	//imshow("Display corners", dst);

	Mat dst_norm, dst_norm_scaled;
	normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
	convertScaleAbs(dst_norm, dst_norm_scaled);
	for (int i = 0; i < dst_norm.rows; i++)
	{
		for (int j = 0; j < dst_norm.cols; j++)
		{
			if ((int)dst_norm.at<float>(i, j) > thresh)
			{
				circle(dst_norm_scaled, Point(j, i), 5, Scalar(0), 2, 8, 0);
			}
		}
	}
	namedWindow("Display corners", WINDOW_AUTOSIZE);
	imshow("Display corners", dst_norm_scaled);
	*/
	
	// Hough Line Transform
	/*
	Mat dst, cdst, cdstP;

	// Edge detection
	Canny(img, dst, 50, 200, 3);
	
	// Copy edges to the images that will display the results in BGR
	cvtColor(dst, cdst, COLOR_GRAY2BGR);
	cdstP = cdst.clone();
	
	// Standard Hough Line Transform
	vector<Vec2f> lines; // will hold the results of the detection
	HoughLines(dst, lines, 1, CV_PI / 180, 100, 0, 0); // runs the actual detection
	
	// Draw the lines
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 10000 * (-b));
		pt1.y = cvRound(y0 + 10000 * (a));
		pt2.x = cvRound(x0 - 10000 * (-b));
		pt2.y = cvRound(y0 - 10000 * (a));
		line(cdst, pt1, pt2, Scalar(0, 0, 255), 3, CV_AA);
	}
	imshow("Detected Lines (in red) - Standard Hough Line Transform", cdst);
	*/

}

int main() {

	string datasetpath("C:\\Users\\Usuario\\source\\repos\\Tests-and-stuff\\TCCproject\\dataset_tcc");
	string dataset("\\basler_melhor");
	string imgfile("\\img24.bmp");
	//string dataset("\\webcam");
	//string imgfile("\\img20.jpg");
	imgfile = datasetpath + dataset + imgfile;

	Mat image;
	image = imread(imgfile, IMREAD_COLOR); // Read the file

	if (!image.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	//namedWindow("Display window", WINDOW_AUTOSIZE); // Create a window for display.
	//imshow("Display window", image); // Show our image inside it.

	
	segmentationSG(image);

	waitKey(0); // Wait for a keystroke in the window
	return 0;

}



