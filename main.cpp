
//______________________________________________________________________________________
// Program : Exam Parser QRcode Base + Checkboxes
// Author  : Shlomo Hassid
//__________________________________________________________________________________
// C:/Users/Shlomi/Desktop/Examer_Graphics/fullpage4.jpg
#define EXAMER_VERSION "0.1"
#define EXAMER_AUTHOR "Shlomi Hassid"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include "Inc.h"
#include "argvparser.h"
#include "Shqr.h"
#include "QrStrParser.h"
#include "ResJson.h"
#include "Exam.h"

#ifdef _WIN32

#endif
#ifndef _WIN32

#endif

namespace cm = CommandLineProcessing;

int main(int argc, char **argv)
{

	//Message Results:
	std::vector<std::string> resmes = {
		/* 0 -> Good */		"successful operation.",
		/* 1 -> Error */	"Unable to find image.",
		/* 2 -> Error */	"Can't extract Qr's in given input.",
		/* 3 -> Error */	"Can't parse Qr and get correct parts.",
		/* 4 -> Error */	"Can't Find enough checkboxes.",
		/* 5 -> Error */	"Arguments Error."
	};

	//Get settings:
	int debug_mode = 0;
	std::string pathToSwap = "D:\\Dev\\Examer\\swap\\";
	std::string pathToExec = "C:\\\"Program Files (x86)\"\\ZBar\\bin\\zbarimg";
	cv::String filename = "";
	cm::ArgvParser cmd;

	cmd.addErrorCode(0, "Success");
	cmd.addErrorCode(1, "Error");
	cmd.setIntroductoryDescription("Examer version: " + std::string(EXAMER_VERSION) + " - By: " + std::string(EXAMER_AUTHOR));
	cmd.setHelpOption("h", "help", "Print this help page");
	cmd.defineOption("p", "Parse a file -p file.jpg.", cm::ArgvParser::OptionRequiresValue);
	cmd.defineOption("debug", "enable debug.", cm::ArgvParser::NoOptionAttribute);
	cmd.defineOption("saveto", "save result to a path.", cm::ArgvParser::OptionRequiresValue);
	cmd.defineOption("execpath", "the path of the zbar exec.", cm::ArgvParser::OptionRequiresValue);

	int cmdresult = cmd.parse(argc, argv);

	//Parse and set Argu:
	if (cmdresult != cm::ArgvParser::NoParserError)
	{
		//Args has errors:
		if (cmdresult == cm::ArgvParser::ParserHelpRequested) {
			//Help page requested:
			std::cout << cmd.parseErrorDescription(cmdresult);
			exit(0);
		} else {
			cmdresult = 5;
		}
	} else {
		if (cmd.foundOption("p")) {
			filename = cmd.optionValue("p");
		}
		if (cmd.foundOption("debug")) {
			debug_mode = 1;
		}
		if (cmd.foundOption("saveto")) {
			pathToSwap = cmd.optionValue("saveto");
		}
		if (cmd.foundOption("execpath")) {
			pathToExec = cmd.optionValue("execpath");
		}
	}


	//Create Json Result Object:
	ResJson result(debug_mode, resmes);

	//Create ShQr;
	Shqr qrscanner(debug_mode);

	if (cmdresult == 0) {
		//Get target image to process:
		imagecontainer image;
		image.base = cv::imread(filename);
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

					//Set results with parsed qr:
					result.SetBaseInfo(
						parsedqr.Id,
						parsedqr.TotalPages,
						parsedqr.PageNumber,
						parsedqr.QuestionCount,
						parsedqr.StudentId
					);

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
					int totalNeededXheckboxes = 0;
					std::for_each(parsedqr.AnswersCount.begin(), parsedqr.AnswersCount.end(), [&](int n) {
						totalNeededXheckboxes += n;
					});

					//Check if enough checkboxes were fond
					if (totalNeededXheckboxes <= check_count) {

						//Create question groups:
						int quest_count = exam.createQuestionsQrInfo(image.exam, parsedqr);

						//Mark answers:
						int mark_count = exam.findMarkedCheckboxes(image.exam);

						//Save to results:
						for (int i = 0; i < image.exam.questions.size(); i++) {
							result.quest.push_back(image.exam.questions[i]);
						}

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

					}
					else {
						result.SetCode(4); // Error Not enought checkboxes
					}
				}
				else {
					result.SetCode(3); // Error Qr Parse;
				}
			}
			else {
				result.SetCode(2); // Error Extract Qr;
			}
		}
		else {
			result.SetCode(1); // Error image;
		}
	}
	else {
		result.SetCode(5); // Error args;
	}

	
	//Print out results:
	std::cout << result.getprint();
	
	//Halt until images released:
	if (debug_mode) {
		cv::waitKey();
		std::cout << std::endl;
		SH_PAUSE;
	}
	
	//Return:
	return 0;
}
