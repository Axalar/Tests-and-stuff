// TCCproject.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "pch.h"
#include <iostream>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

using namespace cv;
using namespace std;


class Sign {
	
	vector<std::string> lines;
	int numLines = 0;

	public:

	int getNumLines() {
		return numLines;
	}

	std::string getLine(int i) {
		if (i >= 0 && i < numLines)
			return lines[i];
		else
		{
			fprintf(stderr, "Line value out of bounds.");
			return NULL;
		}
	}

	void push_backLine(std::string s) {
		lines.push_back(s);
		numLines = numLines + 1;
	}

	Sign(vector<std::string> text) {
		lines = text;
		numLines = text.size();
	}

	Sign(std::string s) {
		lines.push_back(s);
		numLines = numLines + 1;
	}

	Sign() {}

};

class Reading {

	vector<std::string> lines;
	vector<int> confidences;
	int numLines = 0;

public:

	int getNumLines() {
		return numLines;
	}

	std::string getLine(int i) {
		if (i >= 0 && i < numLines)
			return lines[i];
		else
		{
			fprintf(stderr, "Line value out of bounds.");
			return NULL;
		}
	}

	int getConf(int i) {
		if (i >= 0 && i < numLines)
			return confidences[i];
		else
		{
			fprintf(stderr, "Line value out of bounds.");
			return NULL;
		}
	}

	void setLines(vector<std::string> x) {
		lines.swap(x);
	}

	void setConf(vector<int> x) {
		confidences.swap(x);
	}

	Reading() {}

	Reading(vector<std::string> text, vector<int> conf) {
		lines = text;
		confidences = conf;
		numLines = text.size();
	}

};

class MapNode {

	int id;
	vector<Sign> signs;
	int numSigns = 0;

public:

	int getId() {
		return id;
	}

	int getNumSigns() {
		return numSigns;
	}

	Sign getSign(int i) {
		if (i >= 0 && i < numSigns) {
			return signs[i];
		}
		else
		{
			fprintf(stderr, "Sign index out of bounds.");
			exit(1);
		}
	}

	void setId(int ID) {
		id = ID;
	}

	void push_backSign(Sign s) {
		signs.push_back(s);
		numSigns = numSigns + 1;
	}

	MapNode (vector<Sign> x, int ID) {
		id = ID;
		signs = x;
		numSigns = x.size();
	}

	MapNode() {}

};

class Map {

	vector<MapNode> nodes;
	int numNodes = 0;

public:

	MapNode getNode(int i) {
		if (i >= 0 && i < numNodes) {
			return nodes[i];
		}
		else
		{
			fprintf(stderr, "Node index out of bounds.");
			exit(1);
		}
		
	}

	int getNumNodes() {
		return numNodes;
	}

	void push_backNode(MapNode n) {
		nodes.push_back(n);
		numNodes = numNodes + 1;
	}

	void push_backNode() {
		MapNode n;
		nodes.push_back(n);
		numNodes = numNodes + 1;
	}

	Map(vector<MapNode> x) {
		nodes = x;
		numNodes = x.size();
	}

	Map() {}

};

class Match {

	int node = NULL;
	int sign = NULL;
	int reading = NULL;
	int similariti = -1;

public:

	int getNodeId() {
		return node;
	}

	int getSignsIdx() {
		return sign;
	}

	int getReadingIdx() {
		return reading;
	}

	int getSim() {
		return similariti;
	}

	void setNodeId(int n) {
		node = n;
	}

	void setSign(int s) {
		sign = s;
	}

	void setSim(int s) {
		similariti = s;
	}

	Match(int n, int r) {
		node = n;
		reading = r;
	}

};

//*
template<typename T>
typename T::size_type LevensteinDistance(const T &source, const T &target) {
	if (source.size() > target.size()) {
		return LevensteinDistance(target, source);
	}

	using TSizeType = typename T::size_type;
	const TSizeType min_size = source.size(), max_size = target.size();
	std::vector<TSizeType> lev_dist(min_size + 1);

	for (TSizeType i = 0; i <= min_size; ++i) {
		lev_dist[i] = i;
	}

	for (TSizeType j = 1; j <= max_size; ++j) {
		TSizeType previous_diagonal = lev_dist[0], previous_diagonal_save;
		++lev_dist[0];

		for (TSizeType i = 1; i <= min_size; ++i) {
			previous_diagonal_save = lev_dist[i];
			if (source[i - 1] == target[j - 1]) {
				lev_dist[i] = previous_diagonal;
			}
			else {
				lev_dist[i] = std::min(std::min(lev_dist[i - 1], lev_dist[i]), previous_diagonal) + 1;
			}
			previous_diagonal = previous_diagonal_save;
		}
	}

	return lev_dist[min_size];
}
//*/

// Sem gaussiana
void segmentationSG(Mat img, vector<vector<Point>>& filtered_contours) {

	// Conversão para mask baseada em cor e saturação
	cv::Mat hsv, HSVthresh;
	cv::cvtColor(img, hsv, CV_BGR2HSV);

	float low_H = 23; //25 basler melhor ou 23 basler teste
	float low_S = 50; //150 basler melhor ou 50 basler teste
	float low_V = 0;
	float high_H = 30; //30 basler melhor ou 35 webcam
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

	//std::printf("Contours found: %d \n", contours.size());// <------
	//cout << "Contours found: " << contours.size() << endl;// <------

	// draw
	/*
	//Mat drawing = Mat::zeros(opened.size(), CV_8UC3);
	Mat drawing0;
	drawing0 = img.clone();
	for (size_t i = 0; i < contours.size(); i++)
	{
		drawContours(drawing0, contours, (int)i, Scalar(0, 0, 255), 2, 8, hierarchy, 0, Point());
	}
	
	// show
	namedWindow("Display drawing", WINDOW_NORMAL);
	imshow("Display drawing", drawing0);
	//*/

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
			//printf("Comprimento %d: %f \n", i, ctLen);// <------

			//double ctArea = contourArea(contours[i]);
			//printf("Area %d: %f \n", i, ctArea);// <------
		}
	}

	size_t numcont = filtered_contours.size();

	//cout << "Filtered contours found: " << filtered_contours.size() << endl;// <------

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
	//*/
	//------------------------------------------------------------------------------
	// convex hull
	/*
	Mat drawing2;
	drawing2 = img.clone();
	vector<vector<Point>> hull(numcont);
	for (size_t i = 0; i < numcont; i++)
	{
		convexHull(filtered_contours[i], hull[i]);
		drawContours(drawing2, hull, (int)i, Scalar(255, 0, 0), 2, 8);
	}
	//show
	namedWindow("Display hull", WINDOW_NORMAL);
	imshow("Display hull", drawing2);
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
	namedWindow("Display aproxPoly do hull", WINDOW_NORMAL);
	imshow("Display aproxPoly do hull", drawing3);
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

	vector<Point> hull;
	Point Ant, Prox, Cur;
	double menor_dist, distance;
	size_t menor_dist_index = -1;

	for (size_t k = 0; k < contours.size(); k++)
	{
		
		convexHull(contours[k], hull);

		//printf("Hull numero: %d\n", k);// <------

		//double ctArea = contourArea(contours[k]);
		//double huArea = contourArea(hull);
		//if (std::abs(huArea - ctArea) < 1.00*ctArea) // regular threshold para regeição de falsos positivos
		//{

			while (hull.size() > 4)
			{
				menor_dist = 5000000;
				menor_dist_index = -1;
				//printf("Hull numero: %d\n", k);// <------
				//printf("Pontos no hull: %d\n", (int)hull_copy.size());// <------

				for (size_t i = 0; i < hull.size(); i++)
				{
					Cur = hull[i];
					//printf("Pontos %d: %d, %d\n", (int)i, (int)Cur.x, (int)Cur.y);// <------

					if (i == 0)
					{
						Ant = hull.back();
						Prox = hull[i + 1];
					}
					else if ((int)i == ((int)hull.size() - 1))
					{
						Ant = hull[i - 1];
						Prox = hull[0];
					}
					else
					{
						Ant = hull[i - 1];
						Prox = hull[i + 1];
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

				hull.erase(hull.begin() + menor_dist_index);

			}

			corners.push_back(hull);

			//draw
			//circle(img, Point(hull[0].x, hull[0].y), 5, Scalar(0, 0, 255), FILLED);
			//circle(img, Point(hull[1].x, hull[1].y), 5, Scalar(0, 0, 255), FILLED);
			//circle(img, Point(hull[2].x, hull[2].y), 5, Scalar(0, 0, 255), FILLED);
			//circle(img, Point(hull[3].x, hull[3].y), 5, Scalar(0, 0, 255), FILLED);

			//drawContours(img, corners, (int)k, Scalar(0, 255, 0), 2, 8);

		//}
	}

	/*/show
	cv::namedWindow("Display lines", WINDOW_NORMAL);
	cv::imshow("Display lines", img);
	//*/

}

void perspectiveCorrection(Mat img, vector<Point> corners, Mat& img_out) {

	// Tutorial homografia:
	// https://docs.opencv.org/3.4.1/d9/dab/tutorial_homography.html
	
	// Função findHomography:
	// https://docs.opencv.org/3.4.1/d9/d0c/group__calib3d.html#ga4abc2ece9fab9398f2e560d53c8c9780

	// Função warpPerspective:
	// https://docs.opencv.org/3.4.1/da/d54/group__imgproc__transform.html#gaf73673a7e8e18ec6963e3774e6a94b87

	int X = 500, Y = 100;
	vector<Point2f> origin, destiny;
	Mat H;
	Point2f centroide;

	centroide = Point2f((corners[0].x + corners[1].x + corners[2].x + corners[3].x)/4,
		(corners[0].y + corners[1].y + corners[2].y + corners[3].y)/4);

	for (size_t i = 0; i < 4; i++)
	{
		origin.push_back(Point2f(corners[i].x, corners[i].y));
	}

	//circle(img, Point(origin[0].x, origin[0].y), 3, Scalar(0, 255, 255), FILLED); // amarelo
	//circle(img, Point(origin[1].x, origin[1].y), 3, Scalar(0, 0, 255), FILLED); // vermelho
	//circle(img, Point(origin[2].x, origin[2].y), 3, Scalar(0, 255, 0), FILLED); // verde
	//circle(img, Point(origin[3].x, origin[3].y), 3, Scalar(255, 0, 0), FILLED); // azul

	if (origin[0].x >= centroide.x && origin[1].x < centroide.x)
	{
		destiny.push_back(Point2f(X, Y));
		destiny.push_back(Point2f(0, Y));
		destiny.push_back(Point2f(0, 0));
		destiny.push_back(Point2f(X, 0));
	}
	else
	{
		destiny.push_back(Point2f(X, 0));
		destiny.push_back(Point2f(X, Y));
		destiny.push_back(Point2f(0, Y));
		destiny.push_back(Point2f(0, 0));
	}

	H = findHomography(origin, destiny);
	warpPerspective(img, img_out, H, Size(X, Y));

}

void performOcr(Mat sign, vector<Reading>& readings) {


	Mat adthresh, otthresh;

	cvtColor(sign, sign, CV_RGB2GRAY);

	adaptiveThreshold(sign, adthresh, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 61, 10);
	threshold(sign, otthresh, 0, 255, THRESH_OTSU);

	cvtColor(adthresh, adthresh, CV_GRAY2RGB);
	cvtColor(otthresh, otthresh, CV_GRAY2RGB);
	
	//cv::resize(sign, sign, cv::Size(), 1.5, 1);

	//------------
	//*
	char *outTextO, *outTextA;
	float conf;
	vector<std::string> fullTextO, fullTextA;
	vector<int> allConfsO, allConfsA;

	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	if (api->Init("C:\\Users\\alpha\\source\\repos\\Tests-and-stuff\\TCCproject", "por", tesseract::OEM_TESSERACT_ONLY)) {
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}

	//printf("\n\nIdioma: %s\n", api->GetInitLanguagesAsString());


	printf("OCR with otsu thresh.\n"); // <----------
	api->SetImage(otthresh.data, otthresh.cols, otthresh.rows, 3, otthresh.step);

	// Get OCR result
	outTextO = api->GetUTF8Text();
	//conf = api->MeanTextConf();
	//printf("OCR output:\n%s", outText);
	//printf("OCR confidence: %f\n", conf);
	delete[] outTextO;
	
	Boxa* boxes = api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);
	printf("Found %d textline image components.\n", boxes->n);

	for (int i = 0; i < boxes->n; i++) {

		BOX* box = boxaGetBox(boxes, i, L_CLONE);

		int q1 = box->x, q2 = box->y, q3 = box->w, q4 = box->h;

		if (box->x > 0)
			q1 = box->x - 1;
		if (box->y > 0)
			q2 = box->y - 1;
		if ((box->w + box->x) < 499)
			q3 = box->w + 1;
		if ((box->h + box->y) < 99)
			q4 = box->h + 1;

		//api->SetRectangle(box->x - 1, box->y - 1, box->w + 1, box->h + 1);
		api->SetRectangle(q1, q2, q3, q4);
		
		char* ocrResult = api->GetUTF8Text();
		int conf = api->MeanTextConf();

		//circle(otthresh, Point(box->x, box->y), 3, Scalar(0, 0, 0), FILLED);
		//circle(otthresh, Point(box->x + box->w, box->y), 3, Scalar(0, 0, 0), FILLED);
		//circle(otthresh, Point(box->x, box->y + box->h), 3, Scalar(0, 0, 0), FILLED);
		//circle(otthresh, Point(box->x + box->w, box->y + box->h), 3, Scalar(0, 0, 0), FILLED);

		//cv::namedWindow("batata", WINDOW_AUTOSIZE);
		//cv::imshow("batata", otthresh);
		
		if ((unsigned)strlen(ocrResult) > 2)
		{
			fullTextO.push_back(std::string(ocrResult, (unsigned)strlen(ocrResult) - 2));
			allConfsO.push_back(conf);
		}
		else
		{
			fullTextO.push_back(std::string(ocrResult, (unsigned)strlen(ocrResult)));
			allConfsO.push_back(conf);
		}
		
		fprintf(stdout, "Box[%d]: x=%d, y=%d, w=%d, h=%d, confidence: %d, text: %s",
			i, box->x, box->y, box->w, box->h, conf, ocrResult);
		
		delete[] ocrResult;

	}

	//-----------------------------------------------------------
	//*
	printf("\nOCR with adaptative thresh.\n"); // <----------
	api->SetImage(adthresh.data, adthresh.cols, adthresh.rows, 3, adthresh.step);

	// Get OCR result
	outTextA = api->GetUTF8Text();

	delete[](outTextA);
	
	Boxa* boxesA = api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);
	printf("Found %d textline image components.\n", boxesA->n);
	//*
	for (int i = 0; i < boxesA->n; i++) {

		BOX* boxA = boxaGetBox(boxesA, i, L_CLONE);

		int q1 = boxA->x, q2 = boxA->y, q3 = boxA->w, q4 = boxA->h;

		if (boxA->x > 1)
			q1 = boxA->x - 1;
		if (boxA->y > 1)
			q2 = boxA->y - 1;
		if ((boxA->w + boxA->x) < 499)
			q3 = boxA->w + 1;
		if ((boxA->h + boxA->y) < 99)
			q4 = boxA->h + 1;

		//api->SetRectangle(boxA->x - 1, boxA->y - 1, boxA->w + 1, boxA->h + 1);
		api->SetRectangle(q1, q2, q3, q4);
		char* ocrResult = api->GetUTF8Text();
		int conf = api->MeanTextConf();

		if ((unsigned)strlen(ocrResult) > 2)
		{
			fullTextA.push_back(std::string(ocrResult, (unsigned)strlen(ocrResult) - 2));
			allConfsA.push_back(conf);
		}
		else
		{
			fullTextA.push_back(std::string(ocrResult, (unsigned)strlen(ocrResult)));
			allConfsA.push_back(conf);
		}

		fprintf(stdout, "Box[%d]: x=%d, y=%d, w=%d, h=%d, confidence: %d, text: %s",
			i, boxA->x, boxA->y, boxA->w, boxA->h, conf, ocrResult);

		delete[] ocrResult;

	}
	//*/
	//-----------------------------------------------------------

	if (allConfsO.size() > 0 && allConfsA.size() > 0)
	{
		if (allConfsO[0] >= 80 && allConfsO[0] >= allConfsA[0])
		{
			Reading temp(fullTextO, allConfsO);
			readings.push_back(temp);
		}
		else if (allConfsA[0] >= 80 && allConfsA[0] > allConfsO[0])
		{
			Reading temp(fullTextA, allConfsA);
			readings.push_back(temp);
		}
	}
	else
	{
		if (allConfsO.size() > 0)
		{
			if (allConfsO[0] >= 80)
			{
				Reading temp(fullTextO, allConfsO);
				readings.push_back(temp);
			}
		}
		else if (allConfsA.size() > 0)
		{
			if (allConfsA[0] >= 80)
			{
				Reading temp(fullTextA, allConfsA);
				readings.push_back(temp);
			}
		}
	}

	//* Destroy used object and release memory
	api->End();
	//*/

}

Map initializeMap() {
	/*
	string l1("B013");
	string l2("B014");
	string l3("B012");
	string l4("Apoio Técnico Química");

	string l5("B015");
	string l6("LASER");
	string l7("B016");
	string l8("LABIND");
	string l9("B011");
	string l10("LABQO");

	string l11("B018");
	string l12("LEM");

	string l13("B008");
	string l14("B009");

	string l15("B107");

	string l16("B116");
	string l17("LABEX");
	string l18("B117");
	string l19("LABCOP");
	*/
	vector<std::string> lines;
	lines.push_back(string("B013"));
	lines.push_back(string("B014"));
	lines.push_back(string("B012"));
	lines.push_back(string("Apoio Técnico Química"));
	lines.push_back(string("B015"));
	lines.push_back(string("LASER"));
	lines.push_back(string("B016"));
	lines.push_back(string("LABIND"));
	lines.push_back(string("B011"));
	lines.push_back(string("LABQO"));
	lines.push_back(string("B018"));
	lines.push_back(string("LEM"));
	lines.push_back(string("B008"));
	lines.push_back(string("B009"));
	lines.push_back(string("B107"));
	lines.push_back(string("B116"));
	lines.push_back(string("LABEX"));
	lines.push_back(string("B117"));
	lines.push_back(string("LABCOP"));

	//
	lines.push_back(string("B001"));
	lines.push_back(string("B002"));
	lines.push_back(string("B003"));
	lines.push_back(string("B004"));
	lines.push_back(string("B005"));
	lines.push_back(string("B006"));
	lines.push_back(string("B007"));
	lines.push_back(string("B010"));
	lines.push_back(string("B020"));
	lines.push_back(string("B021"));
	lines.push_back(string("B101"));
	lines.push_back(string("B102"));
	lines.push_back(string("B103"));
	lines.push_back(string("B104"));
	lines.push_back(string("B106"));
	lines.push_back(string("B107"));
	//

	//int spn[6] = {4, 6, 2, 2, 1, 4};
	vector<MapNode> nodes;
	vector<Sign> signs1;
	signs1.push_back(Sign(lines[0]));
	signs1.push_back(Sign(lines[1]));
	signs1.push_back(Sign(lines[2]));
	signs1.push_back(Sign(lines[3]));
	MapNode n1(signs1, 1);
	nodes.push_back(n1);

	vector<Sign> signs2;
	signs2.push_back(Sign(lines[4]));
	signs2.push_back(Sign(lines[5]));
	signs2.push_back(Sign(lines[6]));
	signs2.push_back(Sign(lines[7]));
	signs2.push_back(Sign(lines[8]));
	signs2.push_back(Sign(lines[9]));
	MapNode n2(signs2, 2);
	nodes.push_back(n2);

	vector<Sign> signs3;
	signs3.push_back(Sign(lines[10]));
	signs3.push_back(Sign(lines[11]));
	MapNode n3(signs3, 3);
	nodes.push_back(n3);

	vector<Sign> signs4;
	signs4.push_back(Sign(lines[12]));
	signs4.push_back(Sign(lines[13]));
	MapNode n4(signs4, 4);
	nodes.push_back(n4);

	vector<Sign> signs5;
	signs5.push_back(Sign(lines[14]));
	MapNode n5(signs5, 5);
	nodes.push_back(n5);

	vector<Sign> signs6;
	signs6.push_back(Sign(lines[15]));
	signs6.push_back(Sign(lines[16]));
	signs6.push_back(Sign(lines[17]));
	signs6.push_back(Sign(lines[18]));
	MapNode n6(signs6, 6);
	nodes.push_back(n6);

	vector<Sign> signs7;
	signs7.push_back(Sign(lines[19]));
	signs7.push_back(Sign(lines[20]));
	signs7.push_back(Sign(lines[21]));
	signs7.push_back(Sign(lines[22]));
	signs7.push_back(Sign(lines[23]));
	signs7.push_back(Sign(lines[24]));
	signs7.push_back(Sign(lines[25]));
	signs7.push_back(Sign(lines[26]));
	signs7.push_back(Sign(lines[27]));
	signs7.push_back(Sign(lines[28]));
	signs7.push_back(Sign(lines[29]));
	signs7.push_back(Sign(lines[30]));
	signs7.push_back(Sign(lines[31]));
	signs7.push_back(Sign(lines[32]));
	signs7.push_back(Sign(lines[33]));
	signs7.push_back(Sign(lines[34]));
	MapNode n7(signs7, 7);
	nodes.push_back(n7);

	Map map(nodes);
	//Map map;
	return map;
}

void localization(vector<Reading> readings) {

	Map map = initializeMap();
	int numNodes = map.getNumNodes();
	int numReads = readings.size();
	int numSigns = 0;
	vector<Match> matches;


	if (numReads > 0)
	{
		cout << "Localizacao iniciada." << endl;
		//*
		for (int i = 0; i < readings.size(); i++)
		{
			matches.push_back(Match(NULL, i));
		}
		//*/
		for (int n = 0; n < numNodes/* && matches.size() < numReads*/; n++) // laço dos nós
		{
			numSigns = map.getNode(n).getNumSigns();
			for (int s = 0; s < numSigns/* && matches.size() < numReads*/; s++) // laço das placas
			{
				for (int r = 0; r < numReads/* && matches.size() < numReads*/; r++) // laço das leituras
				{
					//str1.compare(0, str1.size(), strt, 0, str1.size()) == 0
					/*
					if (map.getNode(n).getSign(s).getLine(0).compare(0,
						map.getNode(n).getSign(s).getLine(0).size(), readings[r].getLine(0), 0,
						map.getNode(n).getSign(s).getLine(0).size()) == 0)
					{
						matches.push_back(Match(n, r));

						cout << "Match found!" << endl;
						cout << "Node:" << n << " Reading: " << r
							<< " Placa: " << map.getNode(n).getSign(s).getLine(0)
							<< " Leitura: " << readings[r].getLine(0) << endl;
						cout << "Size plate: " << map.getNode(n).getSign(s).getLine(0).size()
							<< " Size reading: " << readings[r].getLine(0).size() << endl;
					}
					//*/

					//*--------------
					//cout << LevensteinDistance(map.getNode(n).getSign(s).getLine(0), readings[r].getLine(0)) << endl;
					int sim = (int)LevensteinDistance(map.getNode(n).getSign(s).getLine(0), readings[r].getLine(0));
					if (sim < 2)
					{
						for (int m = 0; m < matches.size(); m++) // laço de matches
						{
							//cout << "match " << m << " signs " << s << " reading " << r << " node " << n << endl;
							if (matches[m].getReadingIdx() == r && matches[m].getSim() == -1)
							{
								cout << "New match found! Reading: " << readings[r].getLine(0) << " Signs: " << map.getNode(n).getSign(s).getLine(0)
									<< " Similaridade: " << sim << endl;
								matches[m].setNodeId(n);
								matches[m].setSign(s);
								matches[m].setSim(sim);
							}
							else if (matches[m].getReadingIdx() == r && matches[m].getSim() > sim)
							{
								cout << "Better match found! Reading: " << readings[r].getLine(0) << " Signs: " << map.getNode(n).getSign(s).getLine(0)
									<< " Similaridade: " << sim << endl;
								matches[m].setNodeId(n);
								matches[m].setSign(s);
								matches[m].setSim(sim);
							}
						}
					}
					//--------------
					//*/
				}
			}
		}

		// Diagnóstico dos resultados
		cout << "Localizacao finalizada." << endl;
		cout << endl;
		cout << "Resultados:" << endl;
		cout << "Readings: " << numReads << endl;
		int quantMatch = 0;
		int wrongMatches = 0;
		for (size_t i = 0; i < matches.size(); i++)
		{
			if (matches[i].getSim() > -1)
			{
				quantMatch = quantMatch + 1;
				cout << "Match found!" << endl;
				cout << "Node:" << matches[i].getNodeId() + 1 << " Reading: " << matches[i].getReadingIdx()
					<< " Placa: " << map.getNode(matches[i].getNodeId()).getSign(matches[i].getSignsIdx()).getLine(0)
					<< " Leitura: " << readings[matches[i].getReadingIdx()].getLine(0) << endl;
				cout << "Size plate: " << map.getNode(matches[i].getNodeId()).getSign(matches[i].getSignsIdx()).getLine(0).size()
					<< " Size reading: " << readings[matches[i].getReadingIdx()].getLine(0).size() << endl;
			}

			if (matches[i].getNodeId() == 7)
			{
				wrongMatches++;
			}

		}
		cout << "Matches: " << quantMatch << endl;
		cout << "Wrong matches: " << wrongMatches << endl;
		cout << "Taxa de matching: " << quantMatch / numReads << endl;
	}
	else
	{
		cout << "Localizacao finalizada." << endl;
		cout << "Nao foram encontradas placas." << endl;
	}
}

int main() {
	
	string datasetpath("C:\\Users\\alpha\\source\\repos\\Tests-and-stuff\\TCCproject\\dataset_tcc");
	//string dataset("\\basler_melhor");
	//string imgfile("\\img03.bmp");
	string dataset("\\basler_teste");
	string imgfile("\\img12.tiff");
	//string dataset("\\webcam");
	//string imgfile("\\img20.jpg");
	imgfile = datasetpath + dataset + imgfile;

	Mat image, sign;
	image = imread(imgfile, IMREAD_COLOR); // Read the file

	if (!image.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	//namedWindow("Display window", WINDOW_NORMAL); // Create a window for display.
	//imshow("Display window", image); // Show our image inside it.

	vector<vector<Point>> contours, corners;

	//printf("Image size %d x %d\n", image.size[0], image.size[1]);

	segmentationSG(image, contours);

	//*
	findCorners(contours, corners, image);

	vector<Reading> readings;

	cout << "Filtered contours found: " << contours.size() << endl;// <------

	for (size_t i = 0; i < corners.size(); i++)
	{
		
		perspectiveCorrection(image, corners[i], sign);

		//*
		std::stringstream s, s1;
		s << "Display placa " << i;
		cv::namedWindow(s.str(), WINDOW_AUTOSIZE);
		cv::imshow(s.str(), sign);
		//*/

		performOcr(sign, readings);

	}

	localization(readings);

	/*
	char* t;
	char a[] = "abacate";
	t = a;
	string str;
	str = string(t);
	std::cout << str << endl;
	//*/

	waitKey(0); // Wait for a keystroke in the window
	return 0;

}



