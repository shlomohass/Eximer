
//______________________________________________________________________________________
// Program : Exam Parser QRcode Base + Checkboxes
// Author  : Shlomo Hassid
//______________________________________________________________________________________

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include "Inc.h"
#include "Shqr.h"
#include "QrStrParser.h"
#include "Exam.h"

int main(int argc, char **argv)
{

	//Message Results:
	std::vector<std::string> resmes = {
		/* 0 -> Good */		"SUC",
		/* 1 -> Error */	"E1:Unable to find image.",
		/* 2 -> Error */	"E2:Can't extract Qr's in given input.",
		/* 3 -> Error */	"E3:Can't parse Qr and get correct parts."
	};

	//Get settings:
	int mes_return = 0;
	int debug_mode = 1;
	std::string pathToSwap = "D:\\Dev\\Examer\\swap\\";
	std::string pathToExec = "C:\\\"Program Files (x86)\"\\ZBar\\bin\\zbarimg";

	//Create ShQr;
	Shqr qrscanner(debug_mode);


	//Get target image to process:
	imagecontainer image;
	image.base = cv::imread(argv[1]);
	if (!image.base.empty()) {

		//Scale to max if needed
		qrscanner.scaleDownWidth(image.base, SHQR_MAX_WIDTH_SCALE_INPUT);

		//Clean image:
		qrscanner.cleanImageFromShadows(image.base, image.base);

		//Load Halfs:
		image.top.original = image.base(cv::Rect(0, 0, image.base.cols, image.base.rows / 2));
		image.bottom.original = image.base(cv::Rect(0, image.base.rows / 2, image.base.cols, image.base.rows / 2));

		qrscanner.proc_image(image.top);
		qrscanner.proc_image(image.bottom);


		bool suc_scanner_top = qrscanner.find_markers(image.top);
		bool suc_qr_top = qrscanner.saveQr(image.top);
		bool suc_scanner_bottom = qrscanner.find_markers(image.bottom);
		bool suc_qr_bottom = qrscanner.saveQr(image.bottom);

		if (debug_mode && suc_scanner_top && suc_qr_top) {
			//cv::imshow("Image", image.top.original);
			//cv::imshow("Gray top", image.top.gray);
			//cv::imshow("Edges top", image.top.edges);
			//cv::imshow("Mass top", image.top.masscenters);
			//cv::imshow("Markers top", image.top.withselectedconturs);
			//cv::imshow("Fixed qr top", image.top.Qr.qr);
			//cv::imshow("Fixed qr_raw top", image.top.Qr.qr_raw);
		}
		if (debug_mode && suc_scanner_bottom && suc_qr_bottom) {
			//cv::imshow("Gray bottom", image.bottom.gray);
			//cv::imshow("Edges bottom", image.bottom.edges);
			//cv::imshow("Mass bottom", image.bottom.masscenters);
			//cv::imshow("Markers bottom", image.bottom.withselectedconturs);
			//cv::imshow("Fixed qr bottom", image.bottom.Qr.qr);
			//cv::imshow("Fixed qr_raw bottom", image.bottom.Qr.qr_raw);
		}


		//Parse the Qr 
		QrStrParser parsedqr(debug_mode);
		int validateResult = -1;
		if (suc_qr_top || suc_qr_bottom) {
			//Save to swao and create the path:
			std::string pathsaved = qrscanner.saveToFolder(
				suc_qr_top ? image.top.Qr.qr : image.bottom.Qr.qr,
				pathToSwap,
				"qrfound"
			);
			//Parse with cmd (zbar - zbarimg)
			image.exam.qrstring = qrscanner.executeQr(pathToExec, pathsaved);

			//Validate and create the elements:
			validateResult = parsedqr.parse(image.exam.qrstring);

			//Only if we have the markers found and Qr parsed:
			if (validateResult == 0) {

				//Update Markers - to be in the same position of the entire page:
				qrscanner.setFinalBaseMarkers(image);

				//Declare:
				Exam exam(debug_mode);

				//Find exam area:
				exam.findExamArea(image);

				//Set Exam:
				exam.setCropExamArea(image);

				//Parse and proc:
				exam.parseExamImage(image.exam);

				//Find checkboxes:
				exam.findcheckboxes(image.exam, SHQR_CHECKBOX_IN_EXAM_PERCENT_AREA);

				//Filter out checkboxes:
				int check_count = exam.filterCheckAndBound(image.exam);

				//Check if enough checkboxes were fond
				//Try to adjust if needed:

				//Create question groups:
				int quest_count = exam.createQuestionsQrInfo(image.exam, parsedqr);

				//Mark answers:
				int mark_count = exam.findMarkedCheckboxes(image.exam);
				if (debug_mode) {
					//cv::imshow("Base Interest area", image.exam.base_with_area);
					//cv::imshow("Base Interest area", image.exam.exam_with_suggest_fix);
					//cv::imshow("Exam After fix", image.exam.exam_after_per_fix_crop);
					//cv::imshow("Exam Clean", image.exam.cleanExam);
					//cv::imshow("Exam blur", image.exam.proc.blur);
					//cv::imshow("Exam thres_blur", image.exam.proc.thres_blur);
					//cv::imshow("Exam thres_gray", image.exam.proc.thres_gray);
					//cv::imshow("Exam edges_thres_blur", image.exam.proc.edges_thres_blur);
					//cv::imshow("Exam edges_thres_gray", image.exam.proc.edges_thres_gray);
					cv::imshow("Exam masscenters", image.exam.proc.masscenters);
					cv::imshow("Exam traces", image.exam.proc.traces);


				}
			} else {
				mes_return = 3; // Error Qr Parse;
			}
		} else {
			mes_return = 2; // Error Extract Qr;
		}
	} else {
		mes_return = 1; // Error image;
	}
	cv::waitKey();
	std::cout << resmes[mes_return] << std::endl;
	SH_PAUSE;
	return 0;
}
