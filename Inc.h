/* The Used includestructures Header
By Shlomo Hassid
*/

#ifndef Inc_H
#define Inc_H

#include <opencv2/opencv.hpp>
#include <iostream>

#define SHQR_MAX_WIDTH_SCALE_INPUT 800
#define SHQR_SAVED_RAW_QR_SIZE 200
#define SHQR_SAVED_QR_BORDER_SIZE 10

#define SHQR_CHECKBOX_IN_EXAM_PERCENT_AREA 0.75
#define SHQR_CHECKBOX_MINAREA 150
#define SHQR_CHECKBOX_MAXAVGDIST_REGLINE 5
#define SHQR_CHECKBOX_MAXDIST_REGLINE 5
#define SHQR_CHECKBOX_RANGE_MINMAX_BETWEEN 0.2
#define SHQR_CHECKBOX_RANGE_MINMAX_MARKED 0.1

#define SHQR_QR_STRING_TOKEN "\n"
#define SHQR_QR_PARTS_TOKEN ":"
#define SHQR_QR_MINPARTS_COUNT 7

enum QuestTypes {
	american_four,
	american_five,
	true_false,
	open
};
struct ShqrMarkers {
	std::vector<cv::Point2f> L_TL;
	std::vector<cv::Point2f> M_TR;
	std::vector<cv::Point2f> O_BL;
	cv::Point2f N_BR_CORNER;
	int orientation;
};

struct checkbox {
	cv::Point2f center;
	double area;
	std::vector<cv::Point> raw_cnt;
	std::vector<cv::Point> approx_cnt;
	cv::Rect bounds;
	double meanpixels;
	bool mark;
	checkbox()
	{
		this->mark = false;
	};
	checkbox(
		cv::Point2f _center,
		std::vector<cv::Point> _raw_cnt,
		std::vector<cv::Point> _approx_cnt,
		cv::Rect _bounds,
		double _area
	)
	{
		this->center = _center;
		this->raw_cnt = _raw_cnt;
		this->approx_cnt = _approx_cnt;
		this->bounds = _bounds;
		this->area = _area;
		this->mark = false;
	}

	//For sorting by hieght
	bool operator < (const checkbox& check) const
	{
		return (center.y < check.center.y);
	}
	bool operator > (const checkbox& check) const
	{
		return (center.y > check.center.y);
	}
};

struct question {
	
	std::vector<int> checkboxesId;
	cv::RotatedRect bounds;
	bool mark;
	QuestTypes  type;
	question()
	{
		this->mark = false;
	};
};
struct qrContainer {
	cv::Mat qr;
	cv::Mat qr_raw;
	cv::Mat qr_gray;
	cv::Mat qr_thres;
	ShqrMarkers markers;
	std::vector<cv::Point2f> src_qr_border;
	std::vector<cv::Point2f> dst_qr_border;
};

struct workimage {

	cv::Mat original;     //Holds base image
	cv::Mat gray;         // The grayscale image
	cv::Mat edges;        // The image after edge detection
	cv::Mat thres;
	cv::Mat masscenters;  // Mass ceneters ploted
	cv::Mat withselectedconturs;
	cv::Mat traces;

	//Data:
	std::vector<std::vector<cv::Point>> contours; // All the conturs found
	std::vector<cv::Vec4i> hierarchy;             // the conturs found hierarchy
	std::vector<cv::Moments> mu;				  // Moments for conturs
	std::vector<cv::Point2f> mc;                  // center of mass for conturs

	//Qr
	bool found = false;
	qrContainer Qr;
};
struct examProc {
	cv::Mat gray;
	cv::Mat blur;
	cv::Mat edges_thres_blur;
	cv::Mat edges_thres_gray;
	cv::Mat thres_blur;
	cv::Mat thres_gray;
	cv::Mat traces;
	cv::Mat masscenters;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	std::vector<cv::Moments> mu;
	std::vector<cv::Point2f> mc;
};
struct examContainer {

	std::string qrstring;

	cv::Mat base_gray;
	cv::Mat base_with_area;
	cv::Mat exam_with_suggest_fix;
	cv::Mat exam_after_per_fix_crop;

	std::vector<cv::Point2f> src_base_area_points;
	std::vector<cv::Point2f> dst_base_area_points;

	std::vector<checkbox> checkboxes;
	std::vector<question> questions;

	cv::Mat cleanExam;
	examProc proc;
};

struct imagecontainer {

	cv::Mat   base;
	workimage top;
	workimage bottom;
	ShqrMarkers base_markers_top;
	ShqrMarkers base_markers_bottom;
	examContainer  exam;
};

#ifdef _WIN32
	#define SH_PAUSE system("pause")
#endif
#endif //Inc_H