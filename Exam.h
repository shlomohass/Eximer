/* The Qr Engine Header
By Shlomo Hassid
*/

#ifndef Exam_H
#define Exam_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include "ShqrBase.h"
#include "Inc.h"

class Exam : public ShqrBase
{

public:

	Exam(int dbg);

	//Get marked Structure and find Exam Area:
	bool findExamArea(imagecontainer& image);

	//Save Exam and fix:
	bool setCropExamArea(imagecontainer& image);

	//Get Exam infos:
	bool parseExamImage(examContainer& exam);

	//Get checkboxes:
	int findcheckboxes(examContainer& exam, float minleft);

	//Filter by regression:
	int filterByReg(examContainer& exam, std::vector<cv::Point2d> regline);
	
	//Filter checkboxes:
	int filterCheckAndBound(examContainer& exam);

	//create question boxes:
	int createQuestions(examContainer& exam);

	//Find Marked answers:
	int findMarkedCheckboxes(examContainer& exam);

	//try to correct:
	int tryCorrectCheckboxes(examContainer& exam);

	virtual ~Exam();

private:
};

#endif //Exam_H