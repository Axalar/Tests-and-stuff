// TCCproject.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>

using namespace cv;
using namespace std;

void segmentationSG() {}
void findCorners() {}
void perspectiveCorrection() {}
void approxSides() {}

// Sem gaussiana
void segmentationSG(Mat img, vector<vector<Point>>& filtered_contours) {

	// Conversão para mask baseada em cor e saturação
	cv::Mat hsv, HSVthresh;
	cv::cvtColor(img, hsv, CV_BGR2HSV);

	float low_H = 25; //25
	float low_S = 170; //150 basler ou 50 webcam
	float low_V = 0;
	float high_H = 30; //30 basler ou 35 webcam
	float high_S = 255; //255
	float high_V = 255;
	
	inRange(hsv, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), HSVthresh);

	//namedWindow("Display HSVthreshBASE", WINDOW_NORMAL);
	//imshow("Display HSVthreshBASE", HSVthresh);

	//------------------------------------------------------------------------------
	// Normalizando e aplicando operação morph. open

	Mat normed, opened;
	normalize(HSVthresh, normed, 0, 255, NORM_MINMAX, CV_8UC1);

	Mat structure = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	morphologyEx(normed, opened, MORPH_OPEN, structure);
	morphologyEx(opened, opened, MORPH_CLOSE, structure);

	//namedWindow("Display opened", WINDOW_NORMAL);
	//imshow("Display opened", opened);

	//------------------------------------------------------------------------------
	// Encontrando contornos

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(opened, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	printf("Contours found: %d \n", contours.size());// <------

	// draw
	/*
	//Mat drawing = Mat::zeros(opened.size(), CV_8UC3);
	Mat drawing0;
	drawing0 = img.clone();
	for (size_t i = 0; i < contours.size(); i++)
	{
		drawContours(drawing0, contours, (int)i, Scalar(255,255,0), 2, 8, hierarchy, 0, Point());
	}
	//*/
	// show
	//namedWindow("Display drawing", WINDOW_NORMAL);
	//imshow("Display drawing", drawing0);
	

	//------------------------------------------------------------------------------
	// Descartando contornos muito pequenos

	//vector<vector<Point>> filtered_contours;
	for (int i = 0; i < contours.size(); i++)
	{
		
		double ctLen = arcLength(contours[i], true);
		double ctArea = contourArea(contours[i]);
		if (ctArea > 1200)
		{
			filtered_contours.push_back(contours[i]);
			printf("Comprimento %d: %f \n", i, ctLen);// <------

			//double ctArea = contourArea(contours[i]);
			printf("Area %d: %f \n", i, ctArea);// <------
		}
	}

	size_t numcont = filtered_contours.size();

	printf("Filtered contours found: %d \n", filtered_contours.size());// <------

	//findCorners(filtered_contours);

	//------------------------------------------------------------------------------
	// aproximação poligonal dos contornos
	/*
	Mat drawing1;
	drawing1 = img.clone();
	vector<vector<Point>> aproxpolys(numcont);
	for (size_t k = 0; k < numcont; k++)
		approxPolyDP(filtered_contours[k], aproxpolys[k], 20, true);
	// draw
	//Mat drawingaprox = Mat::zeros(opened.size(), CV_8UC3);
	for (size_t i = 0; i < aproxpolys.size(); i++)
	{
		drawContours(drawing1, aproxpolys, (int)i, Scalar(255, 0, 255), 2, 8, hierarchy, 0, Point());
	}
	// show
	namedWindow("Display drawingaprox", WINDOW_NORMAL);
	imshow("Display drawingaprox", drawing1);
	*/
	//------------------------------------------------------------------------------
	// convex hull
	/*
	Mat drawing2;
	drawing2 = img.clone();
	vector<vector<Point>> hull(numcont);
	for (size_t i = 0; i < numcont; i++)
	{
		convexHull(filtered_contours[i], hull[i]);
		drawContours(drawing2, hull, (int)i, Scalar(0, 255, 255), 2, 8);
	}
	//show
	//namedWindow("Display hull", WINDOW_NORMAL);
	//imshow("Display hull", drawing2);
	//*/

	//------------------------------------------------------------------------------
	// poly approx of convex hull
	/*
	Mat drawing3;
	drawing3 = img.clone();
	vector<vector<Point>> aproxpolysCH(numcont);
	for (size_t k = 0; k < numcont; k++)
	{
		double ctLen = arcLength(hull[k], true);
		approxPolyDP(hull[k], aproxpolysCH[k], 0.02*ctLen, true);
	}

	// draw
	
	//Mat drawingaprox = Mat::zeros(opened.size(), CV_8UC3);
	for (size_t i = 0; i < aproxpolysCH.size(); i++)
	{
		drawContours(drawing3, aproxpolysCH, (int)i, Scalar(255, 0, 0), 2, 8);
		printf("Numero de pontos no cont. aprox. %d: %d \n", (int)i, (int)aproxpolysCH[i].size());// <------
	}

	// show
	//namedWindow("Display aproxPoly do hull", WINDOW_NORMAL);
	//imshow("Display aproxPoly do hull", drawing3);
	//*/
	
	//------------------------------------------------------------------------------
	// teste aprox. de poligonos descartando pontos
	/*
	//printf("\nAproximacao dos hulls:\n\n");// <------
	vector<Point> hull_copy;
	Point Ant, Prox, Cur;
	double distance = 1000;
	double menor_dist;
	size_t menor_dist_index = -1;
	for (size_t k = 0; k < numcont; k++)
	{
		
		while (hull[k].size() > 4)
		{
			menor_dist = 5000000;
			menor_dist_index = -1;
			hull_copy = hull[k];
			//printf("Hull numero: %d\n", k);// <------
			//printf("Pontos no hull: %d\n", (int)hull_copy.size());// <------

			for (size_t i = 0; i < hull_copy.size(); i++)
			{
				Cur = hull_copy[i];
				//rintf("Pontos %d: %d, %d\n", (int)i, (int)Cur.x, (int)Cur.y);// <------

				if (i == 0)
				{
					Ant = hull_copy.back();
					Prox = hull_copy[i + 1];
				}
				else if ((int)i == ((int)hull_copy.size() - 1))
				{
					Ant = hull_copy[i - 1];
					Prox = hull_copy[0];
				}
				else
				{
					Ant = hull_copy[i - 1];
					Prox = hull_copy[i + 1];
				}

				distance = std::abs((Prox.y - Ant.y)*Cur.x - (Prox.x - Ant.x)*Cur.y + Prox.x*Ant.y - Prox.y*Ant.x) /
					std::sqrt(((Prox.y - Ant.y)*(Prox.y - Ant.y)) + ((Prox.x - Ant.x)*(Prox.x - Ant.x)));

				if (distance < menor_dist)
				{
					menor_dist = distance;
					menor_dist_index = i;
				}


				//printf("Pontos distancia %d: %f\n", (int)i, distance);// <------

			}

			hull[k].erase(hull[k].begin() + menor_dist_index);

		}
		
	}

	Mat drawing4;
	drawing4 = img.clone();
	for (size_t i = 0; i < numcont; i++)
	{
		drawContours(drawing4, hull, (int)i, Scalar(255, 255, 100), 2, 8);
	}
	//show
	namedWindow("Display hull aproximado", WINDOW_NORMAL);
	imshow("Display hull aproximado", drawing4);
	//*/

	// separar os pontos originais do hull em 4 partes usando os 4 pontos restantes
	// do algoritmo acima e usat lineFit para aproximar as linhas independentemente
	// https://docs.opencv.org/3.4.1/dd/d49/tutorial_py_contour_features.html

	//------------------------------------------------------------------------------
	// Encontrando cantos pelo angulo entre os segmentos de reta que formam o hull
	/*
	printf("\nAproximacao dos hulls:\n\n");
	vector<Point> hull_copy;
	Point Ant, Prox, Cur;
	double leng12, leng23, leng13, alpha;
	double alpha_thresh = 150;
	vector<vector<int>> all_indexes;
	vector<int> indexes;

	for (size_t k = 0; k < numcont; k++)
	{

		hull_copy = hull[k];
		printf("\nHull numero: %d\n", k);
		printf("Pontos no hull: %d\n", (int)hull_copy.size());

		for (size_t i = 0; i < hull_copy.size(); i++)
		{
			Cur = hull_copy[i];
			printf("Ponto %d: %d, %d\n", (int)i, (int)Cur.x, (int)Cur.y);
			circle(drawing2, Cur, 3, Scalar(255, 255, 0), FILLED);

			if (i == 0)
			{
				Ant = hull_copy.back();
				Prox = hull_copy[i + 1];
			}
			else if ((int)i == ((int)hull_copy.size() - 1))
			{
				Ant = hull_copy[i - 1];
				Prox = hull_copy[0];
			}
			else
			{
				Ant = hull_copy[i - 1];
				Prox = hull_copy[i + 1];
			}

			leng12 = sqrt(((Cur.y - Ant.y)*(Cur.y - Ant.y)) + ((Cur.x - Ant.x)*(Cur.x - Ant.x)));
			leng13 = sqrt(((Cur.y - Prox.y)*(Cur.y - Prox.y)) + ((Cur.x - Prox.x)*(Cur.x - Prox.x)));
			leng23 = sqrt(((Ant.y - Prox.y)*(Ant.y - Prox.y)) + ((Ant.x - Prox.x)*(Ant.x - Prox.x)));

			alpha = acos((pow(leng12, 2) + pow(leng13, 2) - pow(leng23, 2))/(2 * leng12 * leng13));
			alpha = alpha * 180 / CV_PI;

			if (alpha > 180)
			{
				alpha = 360 - alpha;
			}

			if (alpha < alpha_thresh)
			{
				indexes.push_back((int)i);
				printf("Index: %d\n", (int)i);
				circle(drawing2, Cur, 3, Scalar(0, 0, 255), FILLED);
			}

			printf("Ponto %d alpha: %f\n", (int)i, alpha);

		}

		all_indexes.push_back(indexes);
		indexes.clear();

	}

	//show
	namedWindow("Display hull aproximado", WINDOW_NORMAL);
	imshow("Display hull aproximado", drawing2);
	//*/

	//------------------------------------------------------------------------------
	// aproximação dos lados do poligono por retas
	/*
	vector<Point> side;
	vector<Vec4d> lines;
	Vec4f line0;
	double x = 0, y = 0, righty = 0, lefty = 0 , vx, vy;
	Point pt1, pt2;

	for (int i = 1; i < all_indexes.size(); i++)
	{
		
		indexes = all_indexes[i];
		hull_copy = hull[i];

		for (int k = 0; k < indexes.size(); k++)
		{
			
			if (k == 0)
			{
				side.assign(hull_copy.begin() + indexes[indexes.size()-1], hull_copy.end());
				side.insert(side.end(), hull_copy.begin(), hull_copy.begin()  + indexes[0]);
			}
			else
			{
				side.assign(hull_copy.begin() + indexes[k - 1], hull_copy.begin() + indexes[k]);
			}

			fitLine(side, line0, DIST_L2, 0, 0.001, 0.001);
			printf("Linha %d %d: xv=%f, yv=%f, x0=%f, y0=%f\n", i, k, line0[0], line0[1], line0[2], line0[3]);
			//lines.push_back(line0);

			vx = line0[0];
			vy = line0[1];
			x = line0[2];
			y = line0[3];
			lefty = ((-x * vy / vx) + y);
			righty = (((img.size[1] - x)*vy / vx) + y);
			line(drawing2, Point(img.size[1] - 1, righty), Point(0, lefty), Scalar(255, 0, 0), 2);
			//circle(drawing2, Point(x0, y0), 3, Scalar(255, 0, 0), FILLED);
			
		}

	}

	imshow("Display hull aproximado", drawing2);
	//*/

	//------------------------------------------------------------------------------
	// tentar outros métodos para encontrar as quinas
	// tentar algoritmo de Aggarwal
	// https://stackoverflow.com/questions/11602259/find-the-smallest-containing-convex-polygon-with-a-given-number-of-points

	//------------------------------------------------------------------------------
	// HT
	/*
	Mat drawing4 = Mat::zeros(opened.size(), CV_8UC3);
	Mat drawing5 = img.clone();
	drawContours(drawing4, hull, 0, Scalar(255, 255, 255), 2, 8);
	cvtColor(drawing4, drawing4, COLOR_RGB2GRAY);

	vector<Vec2f> lines;
	HoughLines(drawing4, lines, 1, CV_PI / 180, 70, 0, 0);

	// Draw the lines
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 2000 * (-b));
		pt1.y = cvRound(y0 + 2000 * (a));
		pt2.x = cvRound(x0 - 2000 * (-b));
		pt2.y = cvRound(y0 - 2000 * (a));
		line(drawing5, pt1, pt2, Scalar(0, 5*(int)i, (255-5*(int)i)), 3, CV_AA);
	}

	printf("Numero de retas encontradas: %d", (int)lines.size());
	namedWindow("Display HT", WINDOW_NORMAL);
	imshow("Display HT", drawing5);
	*/

}

// transferir algoritmo depois de encontrar hulls para esta função
void findCorners(vector<vector<Point>> contours, vector<vector<Point>>& corners, Mat img) {
	//*
	//printf("\nAproximacao dos hulls:\n\n");// <------

	vector<Point> hull, hull_copy, side;
	Point Ant, Prox, Cur;
	double menor_dist, distance;
	size_t menor_dist_index = -1;

	for (size_t k = 0; k < contours.size(); k++)
	{
		
		convexHull(contours[k], hull);
		hull_copy = hull;

		//printf("Hull numero: %d\n", k);// <------

		while (hull_copy.size() > 4)
		{
			menor_dist = 5000000;
			menor_dist_index = -1;
			//printf("Hull numero: %d\n", k);// <------
			//printf("Pontos no hull: %d\n", (int)hull_copy.size());// <------

			for (size_t i = 0; i < hull_copy.size(); i++)
			{
				Cur = hull_copy[i];
				//printf("Pontos %d: %d, %d\n", (int)i, (int)Cur.x, (int)Cur.y);// <------

				if (i == 0)
				{
					Ant = hull_copy.back();
					Prox = hull_copy[i + 1];
				}
				else if ((int)i == ((int)hull_copy.size() - 1))
				{
					Ant = hull_copy[i - 1];
					Prox = hull_copy[0];
				}
				else
				{
					Ant = hull_copy[i - 1];
					Prox = hull_copy[i + 1];
				}

				distance = std::abs((Prox.y - Ant.y)*Cur.x - (Prox.x - Ant.x)*Cur.y + Prox.x*Ant.y - Prox.y*Ant.x) /
					std::sqrt(((Prox.y - Ant.y)*(Prox.y - Ant.y)) + ((Prox.x - Ant.x)*(Prox.x - Ant.x)));

				if (distance < menor_dist)
				{
					menor_dist = distance;
					menor_dist_index = i;
				}

				//printf("Pontos distancia %d: %f\n", (int)i, distance);// <------
				
			}

			hull_copy.erase(hull_copy.begin() + menor_dist_index);

		}

		corners.push_back(hull_copy);

		//draw
		//circle(img, Point(hull_copy[0].x, hull_copy[0].y), 3, Scalar(0, 255, 255), FILLED);
		//circle(img, Point(hull_copy[1].x, hull_copy[1].y), 3, Scalar(0, 255, 255), FILLED);
		//circle(img, Point(hull_copy[2].x, hull_copy[2].y), 3, Scalar(0, 255, 255), FILLED);
		//circle(img, Point(hull_copy[3].x, hull_copy[3].y), 3, Scalar(0, 255, 255), FILLED);

		//drawContours(img, corners, (int)k, Scalar(255, 255, 100), 2, 8);
	}

	//show
	//cv::namedWindow("Display lines", WINDOW_NORMAL);
	//cv::imshow("Display lines", img);
	//*/

}

void approxSides(vector<vector<Point>> hulls, vector<vector<Point>> corners, Mat img) {

	//*
	vector<Point> hull, corners_copy, side;
	Vec4d line0;
	std::vector<Point>::iterator begining, ending;

	for (size_t k = 0; k < hulls.size(); k++)
	{
		corners_copy = corners[k];
		hull = hulls[k];

		for (size_t i = 0; i < corners_copy.size(); i++)
		{

			begining = find(hull.begin(), hull.end(), corners_copy[i]);

			if ((int)i == 3)
			{
				ending = find(hull.begin(), hull.end(), corners_copy[0]);
				side.assign(begining, hull.end());
				side.insert(side.end(), hull.begin(), ending + 1);
			}
			else
			{
				ending = find(hull.begin(), hull.end(), corners_copy[i + 1]);
				side.assign(begining, ending + 1);
			}

			fitLine(side, line0, DIST_L2, 0, 0.01, 0.01);
			//printf("Linha %d %d: xv=%f, yv=%f, x0=%f, y0=%f\n", i, k, line0[0], line0[1], line0[2], line0[3]);// <------

			//vx = line0[0];
			//vy = line0[1];
			//x = line0[2];
			//y = line0[3];
			//lefty = ((-x * vy / vx) + y);
			//righty = (((img.size[1] - x)*vy / vx) + y);
			double lefty = ((-line0[2] * line0[1] / line0[0]) + line0[3]);
			double righty = (((img.size[1] - line0[2])*line0[1] / line0[0]) + line0[3]);

			//show
			//line(img, Point(img.size[1] - 1, righty), Point(0, lefty), Scalar(255, 0, 0), 2);
			//for (size_t j = 0; j < side.size(); j++)
			//	circle(img, Point(side[j].x, side[j].y), 3, Scalar(0, 0, 255), FILLED);
			//circle(img, Point(hull_copy[0].x, hull_copy[0].y), 3, Scalar(0, 255, 255), FILLED);
			//circle(img, Point(hull_copy[1].x, hull_copy[1].y), 3, Scalar(0, 255, 255), FILLED);
			//circle(img, Point(hull_copy[2].x, hull_copy[2].y), 3, Scalar(0, 255, 255), FILLED);
			//circle(img, Point(hull_copy[3].x, hull_copy[3].y), 3, Scalar(0, 255, 255), FILLED);

		}

		//drawContours(img, corners, (int)k, Scalar(255, 255, 100), 2, 8);
		
	}
	
	//*/
}

void perspectiveCorrection(Mat img, vector<Point> corners, Mat& img_out) {

	// Tutorial homografia:
	// https://docs.opencv.org/3.4.1/d9/dab/tutorial_homography.html
	
	// Função findHomography:
	// https://docs.opencv.org/3.4.1/d9/d0c/group__calib3d.html#ga4abc2ece9fab9398f2e560d53c8c9780

	// Função warpPerspective:
	// https://docs.opencv.org/3.4.1/da/d54/group__imgproc__transform.html#gaf73673a7e8e18ec6963e3774e6a94b87

}

int main() {
	
	string datasetpath("C:\\Users\\Usuario\\source\\repos\\Tests-and-stuff\\TCCproject\\dataset_tcc");
	string dataset("\\basler_melhor");
	string imgfile("\\img03.bmp");
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

	vector<vector<Point>> contornos, cantos;

	segmentationSG(image, contornos);
	findCorners(contornos, cantos, image);

	waitKey(0); // Wait for a keystroke in the window
	return 0;

}



