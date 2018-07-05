/* The Qr Engine Header
By Shlomo Hassid
*/

#ifndef ShqrBase_H
#define ShqrBase_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include "Inc.h"

class ShqrBase
{

public:


	const int CV_QR_NORTH = 0;
	const int CV_QR_EAST = 1;
	const int CV_QR_SOUTH = 2;
	const int CV_QR_WEST = 3;

	int usedebug = 0;

	ShqrBase(int dbg);

	void cleanImageFromShadows(cv::Mat& input, cv::Mat& output);
	void drawVectorToLines(std::vector<cv::Point2f>& vec, cv::Mat& outimage, cv::Scalar color, int linewidth);
	void drawVectorToLines(std::vector<cv::Point>& vec, cv::Mat& outimage, cv::Scalar color, int linewidth);
	void drawVectorToLines(cv::Mat& mat, cv::Mat& outimage, cv::Scalar color, int linewidth);
	
	bool getIntersectionPoint(cv::Point2f a1, cv::Point2f a2, cv::Point2f b1, cv::Point2f b2, cv::Point2f& intersection);

	float cv_distance(cv::Point2f P, cv::Point2f Q);					// Get Distance between two points

	float cv_lineEquation(cv::Point2f L, cv::Point2f M, cv::Point2f J);		// Perpendicular Distance of a Point J from line formed by Points L and M; Solution to equation of the line Val = ax+by+c 

	float cv_lineSlope(cv::Point2f L, cv::Point2f M, int& alignement);	// Slope of a line by two Points L and M on it; Slope of line, S = (x1 -x2) / (y1- y2)
	
	void cv_getVertices(std::vector<std::vector<cv::Point> > contours, int c_id, float slope, std::vector<cv::Point2f>& X);
	
	void cv_updateCorner(cv::Point2f P, cv::Point2f ref, float& baseline, cv::Point2f& corner);
	
	void cv_updateCornerOr(int orientation, std::vector<cv::Point2f> IN, std::vector<cv::Point2f> &OUT);

	float cross(cv::Point2f v1, cv::Point2f v2);

	template <typename T>
	bool inline isBetween(T value, T min, T max){ return (value < max) && (value > min); }

	double distance_to_Line(cv::Point line_start, cv::Point line_end, cv::Point point);

	std::vector<cv::Point2d> convLineFromV4toV2(cv::Mat& img, std::vector<cv::Point> points, cv::Mat& line);

	void drawFittedLine(cv::Mat& img, std::vector<cv::Point> points, cv::Mat line, cv::Scalar color, int thickness);

	std::vector<cv::Point> convMatToVec(cv::Mat& mat);

	std::vector<cv::Point> convRotatedToVec(cv::RotatedRect& rot);

	std::vector<cv::Point> padVecRec(std::vector<cv::Point>& v, int pad);
};

#endif