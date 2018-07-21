/* The Qr Engine CPP
By Shlomo Hassid
*/

#ifndef Exam_CPP
#define Exam_CPP

#include "Exam.h"



Exam::Exam(int dbg) : ShqrBase(dbg)
{

};


bool Exam::findExamArea(imagecontainer& image) {

	cv::Point2f inter_TR;
	cv::Point2f inter_Bl;

	//Find intersections:
	bool testInterTR = this->getIntersectionPoint(
		image.base_markers_top.O_BL[3], image.base_markers_top.N_BR_CORNER,
		image.base_markers_bottom.L_TL[0], image.base_markers_bottom.O_BL[3],
		inter_TR
	);

	bool testInterBL = this->getIntersectionPoint(
		image.base_markers_top.L_TL[0], image.base_markers_top.O_BL[3],
		image.base_markers_bottom.O_BL[3], image.base_markers_bottom.N_BR_CORNER,
		inter_Bl
	);

	// TODO : Check intersections are on the map:

	//Save the exam source 4 point:
	image.exam.src_base_area_points.push_back(cv::Point2f(image.base_markers_top.O_BL[3]));
	image.exam.src_base_area_points.push_back(inter_TR);
	image.exam.src_base_area_points.push_back(cv::Point2f(image.base_markers_bottom.O_BL[3]));
	image.exam.src_base_area_points.push_back(inter_Bl);

	//Set the base with area:
	image.exam.base_with_area = image.base.clone();
	image.exam.base_gray = cv::Mat(image.base.size(), CV_MAKETYPE(image.base.depth(), 1));
	cv::cvtColor(image.base, image.exam.base_gray, CV_RGB2GRAY);

	//Plot with dst points after result:
	this->drawVectorToLines(
		image.exam.src_base_area_points,
		image.exam.base_with_area,
		CV_RGB(0, 51, 255),
		3
	);

	return true;
}

bool Exam::setCropExamArea(imagecontainer& image) {

	//Find bounding box:
	cv::Rect bounds = cv::boundingRect(image.exam.src_base_area_points);

	float max_width = (float)bounds.width,
		  max_height = (float)bounds.height;

	//Create and fill the dst vector:
	image.exam.dst_base_area_points.push_back(cv::Point2f((float)bounds.x, (float)bounds.y));
	image.exam.dst_base_area_points.push_back(cv::Point2f((float)bounds.x + (float)bounds.width, (float)bounds.y));
	image.exam.dst_base_area_points.push_back(cv::Point2f((float)bounds.x + (float)bounds.width, (float)bounds.y + bounds.height));
	image.exam.dst_base_area_points.push_back(cv::Point2f((float)bounds.x, (float)bounds.y + (float)bounds.height));

	//Plot the needed operation:
	image.exam.exam_with_suggest_fix = image.exam.base_with_area.clone();
	cv::rectangle(image.exam.exam_with_suggest_fix, bounds, CV_RGB(0, 255, 0), 2);

	image.exam.exam_after_per_fix_crop = cv::Mat(image.exam.base_gray.size(), CV_MAKETYPE(image.exam.base_gray.depth(), 1));
	cv::Mat warp_matrix = cv::getPerspectiveTransform(image.exam.src_base_area_points, image.exam.dst_base_area_points);
	cv::warpPerspective(
		image.exam.base_with_area,
		image.exam.exam_after_per_fix_crop,
		warp_matrix,
		image.exam.base_gray.size()//cv::Size(max_width, max_height)
	);

	//Plot with dst points after result:
	this->drawVectorToLines(
		image.exam.dst_base_area_points,
		image.exam.exam_after_per_fix_crop,
		CV_RGB(0, 255, 0),
		2
	);

	//Save clean:
	bounds -= cv::Size(8, 8);
	bounds += cv::Point(4, 4);
	image.exam.cleanExam = image.exam.exam_after_per_fix_crop(bounds);

	return true;
}

bool Exam::parseExamImage(examContainer& exam) {

	/*
	//Create base:
	exam.proc.gray = cv::Mat(exam.cleanExam.size(), CV_MAKETYPE(exam.cleanExam.depth(), 1));
	exam.proc.edges_thres_blur = cv::Mat(exam.cleanExam.size(), CV_MAKETYPE(exam.cleanExam.depth(), 1));
	exam.proc.edges_thres_gray = cv::Mat(exam.cleanExam.size(), CV_MAKETYPE(exam.cleanExam.depth(), 1));
	exam.proc.traces = exam.cleanExam.clone();
	exam.proc.thres_blur = cv::Mat(exam.cleanExam.size(), CV_8UC1);
	exam.proc.thres_gray = cv::Mat(exam.cleanExam.size(), CV_8UC1);

	//Use the filters:
	cv::cvtColor(exam.cleanExam, exam.proc.gray, CV_RGB2GRAY);
	cv::GaussianBlur(exam.proc.gray, exam.proc.blur, cv::Size(1, 1), 0);
	cv::threshold(exam.proc.blur, exam.proc.thres_blur, 130, 255, CV_THRESH_BINARY_INV);
	cv::threshold(exam.proc.gray, exam.proc.thres_gray, 110, 255, CV_THRESH_BINARY_INV);
	cv::Canny(exam.proc.thres_blur, exam.proc.edges_thres_blur, 100, 200, 3);
	cv::Canny(exam.proc.thres_gray, exam.proc.edges_thres_gray, 100, 200, 3);

	cv::cvtColor(exam.proc.thres_blur, exam.proc.masscenters, CV_GRAY2RGB);
	*/

	
	exam.proc.gray = cv::Mat(exam.cleanExam.size(), CV_MAKETYPE(exam.cleanExam.depth(), 1));
	exam.proc.traces = exam.cleanExam.clone();
	exam.proc.thres_blur = cv::Mat(exam.cleanExam.size(), CV_8UC1);
	cv::cvtColor(exam.cleanExam, exam.proc.gray, CV_RGB2GRAY);
	cv::adaptiveThreshold(exam.proc.gray, exam.proc.thres_blur, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 13, 8);
	cv::cvtColor(exam.proc.thres_blur, exam.proc.masscenters, CV_GRAY2RGB);
	
	//Find conturs in image:
	cv::findContours(exam.proc.thres_blur, exam.proc.contours, exam.proc.hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	//Get mass centers and moments:
	exam.proc.mu = std::vector<cv::Moments>(exam.proc.contours.size());
	exam.proc.mc = std::vector<cv::Point2f>(exam.proc.contours.size());
	for (int i = 0; i < exam.proc.contours.size(); i++)
	{
		exam.proc.mu[i] = cv::moments(exam.proc.contours[i], false);
		exam.proc.mc[i] = cv::Point2f((float)exam.proc.mu[i].m10 / (float)exam.proc.mu[i].m00, (float)exam.proc.mu[i].m01 / (float)exam.proc.mu[i].m00);
		//Plot the mass centers: 
		cv::circle(exam.proc.masscenters,
			cv::Point((int)exam.proc.mc[i].x, (int)exam.proc.mc[i].y),
			1, CV_RGB(233, 255, 0), 1
		);
	}

	return true;
}

int Exam::findcheckboxes(examContainer& exam, float minleft) {

	//rules: 
	//  -> must have > 1 childrens perfect is 4
	//  -> should be rect.
	//  -> must be after the min left last quarter
	//  -> must be big enough

	// Start processing the contour data
	float minpixel = minleft * exam.cleanExam.cols;
	std::vector<int> candids;

	int mark = 0; //Candid as contour markers will hold the contour index.

	for (int i = 0; i < exam.proc.contours.size(); i++)
	{
		int k = i;
		int c = 0;

		//Count childs:
		while (exam.proc.hierarchy[k][2] != -1)
		{
			k = exam.proc.hierarchy[k][2];
			c = c + 1;
		}
		if (exam.proc.hierarchy[k][2] != -1) c = c + 1; //Add last one
		//Check if candid:

		if (c >= 1)
		{

			//Is it in bounds?
			if (exam.proc.mc[i].x < minpixel) {
				//Draw rejected:
				cv::drawContours(exam.proc.traces, exam.proc.contours, i, CV_RGB(255, 25, 0), 1);
				continue;
			}

			//Does it have parents?
			if (exam.proc.hierarchy[i][3] != -1) {
				//Draw rejected:
				cv::drawContours(exam.proc.traces, exam.proc.contours, i, CV_RGB(255, 25, 0), 1);
				continue;
			}
			//Is it large enough?
			double cArea = cv::contourArea(exam.proc.contours[i]);
			if (cArea < SHQR_CHECKBOX_MINAREA) {
				cv::drawContours(exam.proc.traces, exam.proc.contours, i, CV_RGB(255, 25, 0), 1);
				continue;
			}

			//Get bounding rectangle:
			cv::Rect bounding = cv::boundingRect(exam.proc.contours[i]);

			//Get approximation contour:
			double epsilon = 0.1*cv::arcLength(exam.proc.contours[i], true);
			std::vector<cv::Point> approx;
			cv::approxPolyDP(cv::Mat(exam.proc.contours[i]), approx, epsilon, true);
			
			//Is it a rect or square:
			if (approx.size() < 4) {
				cv::drawContours(exam.proc.traces, exam.proc.contours, i, CV_RGB(255, 25, 0), 1);
				continue;
			}

			//Save
			cv::drawContours(exam.proc.traces, exam.proc.contours, i, CV_RGB(0, 0, 255), 3);
			this->drawVectorToLines(approx, exam.proc.traces, CV_RGB(255, 204, 0), 1);
			candids.push_back(i); //Save the contour.
			exam.checkboxes.push_back(checkbox(exam.proc.mc[i], exam.proc.contours[i], approx, bounding, cArea));
		}
	}

	return true;
}

int Exam::filterByReg(examContainer& exam, std::vector<cv::Point2d> regline)
{
	//all dist:
	int max_index  = -1;
	double maxdist = 0;
	double avgDist = 0;
	for (int i = 0; i < exam.checkboxes.size(); i++) {
		double dist = std::fabs(this->distance_to_Line(regline[0], regline[1], exam.checkboxes[i].center));
		avgDist += dist;
		if (maxdist <= dist) {
			maxdist = dist;
			max_index = i;
		}
	}
	avgDist = avgDist / (double)exam.checkboxes.size();

	if (
		(avgDist > SHQR_CHECKBOX_MAXAVGDIST_REGLINE || maxdist > SHQR_CHECKBOX_MAXDIST_REGLINE) 
		&& exam.checkboxes.size() >= 2
	) {
		//Remove:
		return max_index;
	} else {
		//Finished:
		return -1;
	}
}
//Filter checkboxes + reorder checkboxes:
int Exam::filterCheckAndBound(examContainer& exam) {

	//Needed centers for regression line:
	std::vector<cv::Point> centers(exam.checkboxes.size());
	for (int i = 0; i < exam.checkboxes.size(); i++) {
		centers[i] = exam.checkboxes[i].center;
	}

	//Filter:
	int filter = 0;
	std::vector<checkbox> filtered;
	cv::Mat reg_line;
	//Loop through until its fine:
	while (filter >= 0) {
		std::vector<cv::Point2d> reg_line_draw;
		//Create Regression line:
		cv::fitLine(centers, reg_line, CV_DIST_L2, 0, 0.01, 0.01);
		reg_line_draw = this->convLineFromV4toV2(exam.proc.traces, centers, reg_line);
		//Filter:
		filter = this->filterByReg(exam, reg_line_draw);
		if (filter >= 0) { //need to throw one
			filtered.push_back(exam.checkboxes[filter]);
			exam.checkboxes.erase(exam.checkboxes.begin() + filter);
			centers.erase(centers.begin() + filter);
		}
	}

	//Draw the line + removed checkboxes:
	this->drawFittedLine(exam.proc.traces, centers, reg_line, CV_RGB(204, 0, 255), 1);
	for (int i = 0; i < filtered.size(); i++) {
		this->drawVectorToLines(filtered[i].raw_cnt, exam.proc.traces, CV_RGB(255, 0, 0), 3);
		cv::circle(exam.proc.traces, filtered[i].center, 3, CV_RGB(255, 0, 0), CV_FILLED);
	}

	//Sort:
	std::sort(exam.checkboxes.begin(), exam.checkboxes.end()); //asc
	//std::sort(goodcheck.begin(), goodcheck.end(), std::greater<checkbox>()); //desc

	//Plot Indexes:
	for (int i = 0; i < exam.checkboxes.size(); i++) {
		cv::putText(
			exam.proc.traces,
			std::to_string(i + 1),
			cv::Point((int)exam.checkboxes[i].center.x + 23, (int)exam.checkboxes[i].center.y + 7),
			CV_FONT_HERSHEY_PLAIN,
			1,
			CV_RGB(0, 0, 255),
			2
		);
	}

	return (int)exam.checkboxes.size();
}

//create question boxes -> use Qr info:
int Exam::createQuestionsQrInfo(examContainer& exam, QrStrParser& parsedqr) {

	int totalcandid = (int)exam.checkboxes.size();
	int checkboxesNeedle = 0;
	for (int i = 0; i < parsedqr.QuestionCount; i++) {
		exam.questions.push_back(question());
		for (int j = 0; j < parsedqr.AnswersCount[i]; j++) {
			exam.questions.back().checkboxesId.push_back(checkboxesNeedle);
			checkboxesNeedle++;
		}
	}

	//Create bounding boxes:
	for (int i = 0; i < (int)exam.questions.size(); i++) {
		std::vector<cv::Point> contour(4);
		std::vector<cv::Point> box;
		std::vector<cv::Point> inflated_box;
		checkbox first = exam.checkboxes[exam.questions[i].checkboxesId[0]];
		checkbox last = exam.checkboxes[exam.questions[i].checkboxesId.back()];
		contour[0] = cv::Point(first.bounds.x, first.bounds.y);
		contour[1] = cv::Point(first.bounds.x + first.bounds.width, first.bounds.y);
		contour[2] = cv::Point(last.bounds.x + last.bounds.width, last.bounds.y + last.bounds.height);
		contour[3] = cv::Point(last.bounds.x, last.bounds.y + last.bounds.height);

		//Get four corners
		inflated_box = this->padVecRec(contour, -5);
		cv::RotatedRect rect = cv::minAreaRect(inflated_box);
		box = this->convRotatedToVec(rect);

		this->drawVectorToLines(box, exam.proc.traces, CV_RGB(0, 204, 0), 2);
		//Save:
		exam.questions[i].bounds = rect;

	}
	return (int)exam.questions.size();
}

//create question boxes -> use avg distance:
int Exam::createQuestionsByDistance(examContainer& exam) {

	int totalcheck = (int)exam.checkboxes.size();
	float dist		= 0;
	float range		= (float)SHQR_CHECKBOX_RANGE_MINMAX_BETWEEN;
	float average	= 0;
	float count		= 0;

	for (int i = 0; i < totalcheck; i++) {
		if (i + 1 < totalcheck) {
			dist = this->cv_distance(exam.checkboxes[i].center, exam.checkboxes[i + 1].center);
			average += dist;
			count++;
			if (i == 0) {
				exam.questions.push_back(question());
				exam.questions.back().checkboxesId.push_back(i);
			}
			else {
				float limit = (average / (float)count);
				exam.questions.back().checkboxesId.push_back(i);
				if (!this->isBetween(dist, limit - range*limit, limit + range*limit)) {
					//Its a new question:
					average -= dist;
					count--;
					exam.questions.push_back(question());
				}
			}
		} else {
			//Last Element:
			exam.questions.back().checkboxesId.push_back(i);
			break;
		}
	}

	//Create bounding boxes:
	for (int i = 0; i < (int)exam.questions.size(); i++) {
		std::vector<cv::Point> contour(4);
		std::vector<cv::Point> box;
		std::vector<cv::Point> inflated_box;
		checkbox first = exam.checkboxes[exam.questions[i].checkboxesId[0]];
		checkbox last = exam.checkboxes[exam.questions[i].checkboxesId.back()];
		contour[0] = cv::Point(first.bounds.x, first.bounds.y);
		contour[1] = cv::Point(first.bounds.x + first.bounds.width, first.bounds.y);
		contour[2] = cv::Point(last.bounds.x + last.bounds.width, last.bounds.y + last.bounds.height);
		contour[3] = cv::Point(last.bounds.x, last.bounds.y + last.bounds.height);

		//Get four corners
		inflated_box = this->padVecRec(contour, -5);
		cv::RotatedRect rect = cv::minAreaRect(inflated_box);
		box = this->convRotatedToVec(rect);

		this->drawVectorToLines(box, exam.proc.traces, CV_RGB(0, 204, 0), 2);
		//Save:
		exam.questions[i].bounds = rect;

	}
	return (int)exam.questions.size();
}

//Find Marked answers:
int Exam::findMarkedCheckboxes(examContainer& exam) {

	std::vector<int> vectFound;
	double avgMean = 0;
	double range = SHQR_CHECKBOX_RANGE_MINMAX_MARKED;
	int count_question_marked = 0;

	//Loop through questions:
	for (int i = 0; i < (int)exam.questions.size(); i++) {
		//Loop through checkboxes:
		for (int c = 0; c < (int)exam.questions[i].checkboxesId.size(); c++) {
			checkbox *check = &exam.checkboxes[exam.questions[i].checkboxesId[c]];
			cv::Mat check_img = exam.proc.gray(check->bounds);
			cv::Scalar mean = cv::mean(check_img);
			avgMean += mean[0];
			check->meanpixels = mean[0];
		}
	}
	avgMean = (avgMean / (double)exam.checkboxes.size()) * (1 - range);

	//We have Average now mark:
	for (int i = 0; i < (int)exam.questions.size(); i++) {
		//Loop through checkboxes:
		bool found = false;
		for (int c = 0; c < (int)exam.questions[i].checkboxesId.size(); c++) {
			checkbox *check = &exam.checkboxes[exam.questions[i].checkboxesId[c]];
			if (check->meanpixels < avgMean) {
				vectFound.push_back(exam.questions[i].checkboxesId[c]);
				check->mark = true;
				found = true;
			}
		}
		exam.questions[i].mark = found;
		count_question_marked += found ? 1 : 0;
	}

	//Draw found:
	for (int i = 0; i < (int)vectFound.size(); i++) {
		cv::circle(
			exam.proc.traces,
			exam.checkboxes[vectFound[i]].center,
			5,
			CV_RGB(255, 204, 0),
			-1
		);
	}
	return count_question_marked;
}

//try to correct:
int Exam::tryCorrectCheckboxes(examContainer& exam) {
	return 0;
}


Exam::~Exam()
{
}

#endif //Exam_CPP