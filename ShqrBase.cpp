/* The Qr Engine Header
By Shlomo Hassid
*/

#ifndef ShqrBase_CPP
#define ShqrBase_CPP

#include <cstdio>
#include "ShqrBase.h"
#include <memory>
#include <stdexcept>
#include <iomanip>
#include <ctime>
#include <sstream>


ShqrBase::ShqrBase(int dbg)
{
	this->usedebug = dbg;
}

//Clean Image
void ShqrBase::cleanImageFromShadows(cv::Mat& input, cv::Mat& output) {

	//Split to RGB planes:
	std::vector<cv::Mat> rgbChannels(3);
	cv::split(input, rgbChannels);

	//Results:
	std::vector<cv::Mat> normplanes(3);
	for (int i = 0; i < 3; i++) {
		int dilate_size = 7;
		cv::Mat dilated_img;
		cv::Mat bg_img;
		cv::Mat diff_img;
		cv::Mat diff_img_black;
		cv::Mat norm_img;
		cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS,
			cv::Size(2 * dilate_size + 1, 2 * dilate_size + 1),
			cv::Point(dilate_size, dilate_size)
		);
		cv::dilate(rgbChannels[i], dilated_img, element);
		cv::medianBlur(dilated_img, bg_img, 21);

		cv::absdiff(rgbChannels[i], bg_img, diff_img_black);
		diff_img = cv::Scalar::all(255) - diff_img_black;
		cv::normalize(diff_img, norm_img, 0, 255, CV_MINMAX, CV_8UC1);
		normplanes[i] = norm_img.clone();
	}
	//Save:
	cv::merge(normplanes, output);
}
//Plot lines out of vector
void ShqrBase::drawVectorToLines(std::vector<cv::Point2f>& vec, cv::Mat& outimage, cv::Scalar color, int linewidth)
{
	for (int i = 0; i < vec.size(); i++) {
		int j = i + 1;
		if (j == vec.size()) {
			j = 0;
		}
		cv::line(outimage, vec[i],vec[j], color, linewidth);
	}
}
void ShqrBase::drawVectorToLines(std::vector<cv::Point>& vec, cv::Mat& outimage, cv::Scalar color, int linewidth)
{
	std::vector<cv::Point2f> v(vec.begin(), vec.end());
	this->drawVectorToLines(v, outimage, color, linewidth);
}
void ShqrBase::drawVectorToLines(cv::Mat& mat, cv::Mat& outimage, cv::Scalar color, int linewidth)
{
	std::vector<cv::Point> v = this->convMatToVec(mat);
	this->drawVectorToLines(v, outimage, color, linewidth);
}

// Function: Get the Intersection Point of the lines formed by sets of two points
bool ShqrBase::getIntersectionPoint(cv::Point2f a1, cv::Point2f a2, cv::Point2f b1, cv::Point2f b2, cv::Point2f& intersection)
{
	cv::Point2f p = a1;
	cv::Point2f q = b1;
	cv::Point2f r(a2 - a1);
	cv::Point2f s(b2 - b1);

	if (this->cross(r, s) == 0) { return false; }

	float t = this->cross(q - p, s) / this->cross(r, s);

	intersection = p + t*r;
	return true;
}

// Function: Routine to get Distance between two points
// Description: Given 2 points, the function returns the distance
float ShqrBase::cv_distance(cv::Point2f P, cv::Point2f Q)
{
	return std::sqrt(pow(std::abs(P.x - Q.x), 2) + std::pow(std::abs(P.y - Q.y), 2));
}

// Function: Perpendicular Distance of a Point J from line formed by Points L and M; Equation of the line ax+by+c=0
// Description: Given 3 points, the function derives the line quation of the first two points,
//	  calculates and returns the perpendicular distance of the the 3rd point from this line.
float ShqrBase::cv_lineEquation(cv::Point2f L, cv::Point2f M, cv::Point2f J)
{
	float a, b, c, pdist;

	a = -((M.y - L.y) / (M.x - L.x));
	b = 1.0;
	c = (((M.y - L.y) / (M.x - L.x)) * L.x) - L.y;

	// Now that we have a, b, c from the equation ax + by + c, time to substitute (x,y) by values from the Point J

	pdist = (a * J.x + (b * J.y) + c) / std::sqrt((a * a) + (b * b));
	return pdist;
}

// Function: Slope of a line by two Points L and M on it; Slope of line, S = (x1 -x2) / (y1- y2)
// Description: Function returns the slope of the line formed by given 2 points, the alignement flag
//	  indicates the line is vertical and the slope is infinity.
float ShqrBase::cv_lineSlope(cv::Point2f L, cv::Point2f M, int& alignement)
{
	float dx, dy;
	dx = M.x - L.x;
	dy = M.y - L.y;

	if (dy != 0)
	{
		alignement = 1;
		return (dy / dx);
	}
	else // Make sure we are not dividing by zero; so use 'alignement' flag
	{
		alignement = 0;
		return 0.0;
	}
}

// Function: Routine to calculate 4 Corners of the Marker in Image Space using Region partitioning
// Theory: OpenCV Contours stores all points that describe it and these points lie the perimeter of the polygon.
//	The below function chooses the farthest points of the polygon since they form the vertices of that polygon,
//	exactly the points we are looking for. To choose the farthest point, the polygon is divided/partitioned into
//	4 regions equal regions using bounding box. Distance algorithm is applied between the centre of bounding box
//	every contour point in that region, the farthest point is deemed as the vertex of that region. Calculating
//	for all 4 regions we obtain the 4 corners of the polygon ( - quadrilateral).
void ShqrBase::cv_getVertices(std::vector<std::vector<cv::Point> > contours, int c_id, float slope, std::vector<cv::Point2f>& quad)
{
	cv::Rect box;
	box = cv::boundingRect(contours[c_id]);

	cv::Point2f M0, M1, M2, M3;
	cv::Point2f A, B, C, D, W, X, Y, Z;

	A = box.tl();
	B.x = (float)box.br().x;
	B.y = (float)box.tl().y;
	C = box.br();
	D.x = (float)box.tl().x;
	D.y = (float)box.br().y;


	W.x = (A.x + B.x) / 2;
	W.y = A.y;

	X.x = B.x;
	X.y = (B.y + C.y) / 2;

	Y.x = (C.x + D.x) / 2;
	Y.y = C.y;

	Z.x = D.x;
	Z.y = (D.y + A.y) / 2;

	float dmax[4];
	dmax[0] = 0.0;
	dmax[1] = 0.0;
	dmax[2] = 0.0;
	dmax[3] = 0.0;

	float pd1 = 0.0;
	float pd2 = 0.0;

	if (slope > 5 || slope < -5)
	{

		for (int i = 0; i < contours[c_id].size(); i++)
		{
			pd1 = this->cv_lineEquation(C, A, contours[c_id][i]);	// Position of point w.r.t the diagonal AC 
			pd2 = this->cv_lineEquation(B, D, contours[c_id][i]);	// Position of point w.r.t the diagonal BD

			if ((pd1 >= 0.0) && (pd2 > 0.0))
			{
				this->cv_updateCorner(contours[c_id][i], W, dmax[1], M1);
			}
			else if ((pd1 > 0.0) && (pd2 <= 0.0))
			{
				this->cv_updateCorner(contours[c_id][i], X, dmax[2], M2);
			}
			else if ((pd1 <= 0.0) && (pd2 < 0.0))
			{
				this->cv_updateCorner(contours[c_id][i], Y, dmax[3], M3);
			}
			else if ((pd1 < 0.0) && (pd2 >= 0.0))
			{
				this->cv_updateCorner(contours[c_id][i], Z, dmax[0], M0);
			}
			else
				continue;
		}
	}
	else
	{
		int halfx = (int)(A.x + B.x) / 2;
		int halfy = (int)(A.y + D.y) / 2;

		for (int i = 0; i < contours[c_id].size(); i++)
		{
			if ((contours[c_id][i].x < halfx) && (contours[c_id][i].y <= halfy))
			{
				this->cv_updateCorner(contours[c_id][i], C, dmax[2], M0);
			}
			else if ((contours[c_id][i].x >= halfx) && (contours[c_id][i].y < halfy))
			{
				this->cv_updateCorner(contours[c_id][i], D, dmax[3], M1);
			}
			else if ((contours[c_id][i].x > halfx) && (contours[c_id][i].y >= halfy))
			{
				this->cv_updateCorner(contours[c_id][i], A, dmax[0], M2);
			}
			else if ((contours[c_id][i].x <= halfx) && (contours[c_id][i].y > halfy))
			{
				this->cv_updateCorner(contours[c_id][i], B, dmax[1], M3);
			}
		}
	}

	quad.push_back(M0);
	quad.push_back(M1);
	quad.push_back(M2);
	quad.push_back(M3);

}

// Function: Compare a point if it more far than previously recorded farthest distance
// Description: Farthest Point detection using reference point and baseline distance
void ShqrBase::cv_updateCorner(cv::Point2f P, cv::Point2f ref, float& baseline, cv::Point2f& corner)
{
	float temp_dist;
	temp_dist = this->cv_distance(P, ref);

	if (temp_dist > baseline)
	{
		baseline = temp_dist;			// The farthest distance is the new baseline
		corner = P;						// P is now the farthest point
	}

}

// Function: Sequence the Corners wrt to the orientation of the QR Code
void ShqrBase::cv_updateCornerOr(int orientation, std::vector<cv::Point2f> IN, std::vector<cv::Point2f> &OUT)
{
	cv::Point2f M0, M1, M2, M3;
	if (orientation == this->CV_QR_NORTH)
	{
		M0 = IN[0];
		M1 = IN[1];
		M2 = IN[2];
		M3 = IN[3];
	}
	else if (orientation == this->CV_QR_EAST)
	{
		M0 = IN[1];
		M1 = IN[2];
		M2 = IN[3];
		M3 = IN[0];
	}
	else if (orientation == this->CV_QR_SOUTH)
	{
		M0 = IN[2];
		M1 = IN[3];
		M2 = IN[0];
		M3 = IN[1];
	}
	else if (orientation == this->CV_QR_WEST)
	{
		M0 = IN[3];
		M1 = IN[0];
		M2 = IN[1];
		M3 = IN[2];
	}

	OUT.push_back(M0);
	OUT.push_back(M1);
	OUT.push_back(M2);
	OUT.push_back(M3);
}

float ShqrBase::cross(cv::Point2f v1, cv::Point2f v2)
{
	return v1.x*v2.y - v1.y*v2.x;
}

double ShqrBase::distance_to_Line(cv::Point line_start, cv::Point line_end, cv::Point point)
{
	double normalLength = _hypot(line_end.x - line_start.x, line_end.y - line_start.y);
	double distance = (double)((point.x - line_start.x) * (line_end.y - line_start.y) - (point.y - line_start.y) * (line_end.x - line_start.x)) / normalLength;
	return distance;
}
std::vector<cv::Point2d> ShqrBase::convLineFromV4toV2(cv::Mat& img, std::vector<cv::Point> points, cv::Mat& line)
{
	double a, b, c;
	std::vector<cv::Point2d> v(2);
	b = -line.at<float>(0, 0);
	a = line.at<float>(1, 0);
	c = -(a*line.at<float>(2, 0) + b*line.at<float>(3, 0));

	v[0] = cv::Point2d(0, -c / b); //X
	v[1] = cv::Point2d(img.cols, (-a*img.cols - c) / b); //Y
	return v;
}
void ShqrBase::drawFittedLine(cv::Mat& img, std::vector<cv::Point> points, cv::Mat line, cv::Scalar color, int thickness)
{
	std::vector<cv::Point2d> v = this->convLineFromV4toV2(img, points, line);
	cv::line(img, v[0], v[1], color, thickness);
}

std::vector<cv::Point> ShqrBase::convMatToVec(cv::Mat& mat) 
{
	std::vector<cv::Point> points;
	//Loop over each pixel and create a point
	for (int x = 0; x < mat.cols; x++)
		for (int y = 0; y < mat.rows; y++)
			points.push_back(cv::Point(x, y));
	return points;
}

std::vector<cv::Point> ShqrBase::convRotatedToVec(cv::RotatedRect& rot) 
{
	std::vector<cv::Point> v(4);
	cv::Point2f vtx[4];
	rot.points(vtx);
	for (int j = 0; j < 4; j++)
		v[j] = vtx[j], vtx[(j + 1) % 4];
	return v;
}

//inflates a rectangle by PAD:
//IMPORTANT -> only works when TL corner is upwords!
std::vector<cv::Point> ShqrBase::padVecRec(std::vector<cv::Point>& v, int pad) {
	std::vector<cv::Point> rv(4);
	if ((int)v.size() == 4) {
		//Top Left Corner:
		rv[0] = cv::Point(v[0].x + pad, v[0].y + pad);
		//Top Right Corner:
		rv[1] = cv::Point(v[1].x - pad, v[1].y + pad);
		//Bottom Right Corner:
		rv[2] = cv::Point(v[2].x - pad, v[2].y - pad);
		//Bottom Left Corner:
		rv[3] = cv::Point(v[3].x + pad, v[3].y - pad);
	}
	return rv;
}

std::string ShqrBase::saveToFolder(cv::Mat& mat, const std::string& _pathSwap, const std::string& _name)
{
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	std::ostringstream oss;
	oss << std::put_time(&tm, "%d%m%Y_%H%M%S");

	std::string path = _pathSwap + _name + "_" + oss.str() + ".jpg";
	cv::imwrite(path, mat);

	return path;
}

std::string ShqrBase::exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::shared_ptr<FILE> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe) return "E";
	while (!feof(pipe.get())) {
		if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
			result += buffer.data();
	}
	return result;
}

std::string ShqrBase::executeQr(const std::string& pathApp, const std::string& pathImg)
{
	char temp[512];
	sprintf(temp, "%s %s -%s", pathApp.c_str(),  pathImg.c_str(), "q");
	return exec((char *)temp);
}

std::vector<std::string> ShqrBase::explode(std::string str, std::string token) {
	std::vector<std::string>result;
	while (str.size()) {
		int index = (int)str.find(token);
		if (index != std::string::npos) {
			result.push_back(str.substr(0, index));
			str = str.substr(index + token.size());
			if (str.size() == 0)result.push_back(str);
		}
		else {
			result.push_back(str);
			str = "";
		}
	}
	return result;
}
std::string ShqrBase::implode(std::vector<std::string>& vec, const char* delim) {
	std::stringstream container;
	copy(vec.begin(), vec.end(), std::ostream_iterator<std::string>(container, delim));
	std::string res = container.str();
	return res.substr(0, (unsigned)res.length() - (unsigned)strlen(delim));
}
std::vector<int> ShqrBase::splitInt(const std::string& str) {
	std::vector<int> vec;
	for (size_t i = 0; i < str.size(); ++i)
	{                                 // This converts the char into an int and pushes it into vec
		vec.push_back(str[i] - '0');  // The digits will be in the same order as before
	}
	return vec;
}


std::string ShqrBase::ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}
#endif