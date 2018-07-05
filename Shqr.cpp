/* The Qr Engine CPP
By Shlomo Hassid
*/

#ifndef Shqr_CPP
#define Shqr_CPP

#include "Shqr.h"


Shqr::Shqr(int dbg) : ShqrBase(dbg) 
{

};

bool Shqr::scaleDownWidth(cv::Mat& inputOutput, int dstSize)
{
	if (inputOutput.cols > dstSize) {

		//Calculate new sizes:
		double ratio = (double)dstSize / (double)inputOutput.cols;
		int newWidth = dstSize;
		double newHeight = (double)inputOutput.rows * ratio;

		cv::resize(inputOutput, inputOutput, cv::Size(newWidth, (int)newHeight));
		return true;
	}
	return false;
}

void Shqr::proc_image(workimage& image) {

	//Create base:
	image.gray		= cv::Mat(image.original.size(), CV_MAKETYPE(image.original.depth(), 1));
	image.edges		= cv::Mat(image.original.size(), CV_MAKETYPE(image.original.depth(), 1));
	image.traces	= cv::Mat(image.original.size(), CV_8UC3);
	image.thres		= cv::Mat(image.original.size(), CV_8UC1);

	image.Qr.qr_raw		= cv::Mat::zeros(SHQR_SAVED_RAW_QR_SIZE, SHQR_SAVED_RAW_QR_SIZE, CV_8UC3);
	image.Qr.qr			= cv::Mat::zeros(SHQR_SAVED_RAW_QR_SIZE, SHQR_SAVED_RAW_QR_SIZE, CV_8UC3);
	image.Qr.qr_gray	= cv::Mat::zeros(SHQR_SAVED_RAW_QR_SIZE + SHQR_SAVED_QR_BORDER_SIZE*2, SHQR_SAVED_RAW_QR_SIZE + SHQR_SAVED_QR_BORDER_SIZE * 2, CV_8UC1);
	image.Qr.qr_thres	= cv::Mat::zeros(SHQR_SAVED_RAW_QR_SIZE + SHQR_SAVED_QR_BORDER_SIZE * 2, SHQR_SAVED_RAW_QR_SIZE + SHQR_SAVED_QR_BORDER_SIZE * 2, CV_8UC1);

	//Use the filters:
	cv::cvtColor(image.original, image.gray, CV_RGB2GRAY);
	cv::threshold(image.gray, image.thres, 110, 255, CV_THRESH_BINARY);
	cv::Canny(image.gray, image.edges, 100, 200, 3);
	cv::cvtColor(image.edges, image.masscenters, CV_GRAY2RGB);
	

	//Find conturs in image:
	cv::findContours(image.edges, image.contours, image.hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	//Get mass centers and moments:
	image.mu = std::vector<cv::Moments>(image.contours.size());
	image.mc = std::vector<cv::Point2f>(image.contours.size());
	for (int i = 0; i < image.contours.size(); i++)
	{
		image.mu[i] = cv::moments(image.contours[i], false);
		image.mc[i] = cv::Point2f(image.mu[i].m10 / image.mu[i].m00, image.mu[i].m01 / image.mu[i].m00);
		//Plot the mass centers: 
		cv::circle( image.masscenters,
			cv::Point((int)image.mc[i].x, (int)image.mc[i].y),
			1,CV_RGB(233, 255, 0),1
		);
	}
}

bool Shqr::find_markers(workimage& image) {
	// Start processing the contour data

	// Find Three repeatedly enclosed contours A,B,C
	// NOTE: 1. Contour enclosing other contours is assumed to be the three Alignment markings of the QR code.
	// 2. Alternately, the Ratio of areas of the "concentric" squares can also be used for identifying base Alignment markers.
	// The below demonstrates the first method

	int mark = 0, A, B, C; //Candid as contour markers will hold the contour index.
	int AB, BC, CA; //Dist between the markers;

	for (int i = 0; i < image.contours.size(); i++)
	{
		int k = i;
		int c = 0;

		while (image.hierarchy[k][2] != -1)
		{
			k = image.hierarchy[k][2];
			c = c + 1;
		}
		if (image.hierarchy[k][2] != -1)
			c = c + 1;

		if (c >= 5)
		{
			if (mark == 0)		A = i;
			else if (mark == 1)	B = i;		// i.e., A is already found, assign current contour to B
			else if (mark == 2)	C = i;		// i.e., A and B are already found, assign current contour to C
			mark = mark + 1;
		}
	}

	//We have the markers:
	if (mark >= 3)		// Ensure we have (atleast 3; namely A,B,C) 'Alignment Markers' discovered
	{
		// We have found the 3 markers for the QR code; Now we need to determine which of them are 'top', 'right' and 'bottom' markers
		// Determining the 'top' marker
		// Vertex of the triangle NOT involved in the longest side is the 'outlier'

		int outlier, median1, median2;
		int top, bottom, right;
		float dist, slope;
		int align;
		AB = this->cv_distance(image.mc[A], image.mc[B]); //Measure the distance between the mass centers of those contuours.
		BC = this->cv_distance(image.mc[B], image.mc[C]);
		CA = this->cv_distance(image.mc[C], image.mc[A]);

		if (AB > BC && AB > CA)
		{
			outlier = C; median1 = A; median2 = B;
		}
		else if (CA > AB && CA > BC)
		{
			outlier = B; median1 = A; median2 = C;
		}
		else if (BC > AB && BC > CA)
		{
			outlier = A;  median1 = B; median2 = C;
		}

		top = outlier;							// The obvious choice

		dist = this->cv_lineEquation(image.mc[median1], image.mc[median2], image.mc[outlier]);	// Get the Perpendicular distance of the outlier from the longest side			
		slope = this->cv_lineSlope(image.mc[median1], image.mc[median2], align);					// Also calculate the slope of the longest side

		// Now that we have the orientation of the line formed median1 & median2 and we also have the position of the outlier w.r.t. the line
		// Determine the 'right' and 'bottom' markers

		if (align == 0)
		{
			bottom = median1;
			right = median2;
		}
		else if (slope < 0 && dist < 0)		// Orientation - North
		{
			bottom = median1;
			right = median2;
			image.Qr.markers.orientation = this->CV_QR_NORTH;
		}
		else if (slope > 0 && dist < 0)		// Orientation - East
		{
			right = median1;
			bottom = median2;
			image.Qr.markers.orientation = this->CV_QR_EAST;
		}
		else if (slope < 0 && dist > 0)		// Orientation - South			
		{
			right = median1;
			bottom = median2;
			image.Qr.markers.orientation = this->CV_QR_SOUTH;
		}

		else if (slope > 0 && dist > 0)		// Orientation - West
		{
			bottom = median1;
			right = median2;
			image.Qr.markers.orientation = this->CV_QR_WEST;
		}


		// To ensure any unintended values do not sneak up when QR code is not present
		float area_top, area_right, area_bottom;

		if (top < image.contours.size() && right < image.contours.size() && bottom < image.contours.size() && cv::contourArea(image.contours[top]) > 10 && cv::contourArea(image.contours[right]) > 10 && cv::contourArea(image.contours[bottom]) > 10)
		{
			std::vector<cv::Point2f> tempL, tempM, tempO;

			this->cv_getVertices(image.contours, top, slope, tempL);
			this->cv_getVertices(image.contours, right, slope, tempM);
			this->cv_getVertices(image.contours, bottom, slope, tempO);

			this->cv_updateCornerOr(image.Qr.markers.orientation, tempL, image.Qr.markers.L_TL); // Re-arrange marker corners w.r.t orientation of the QR code
			this->cv_updateCornerOr(image.Qr.markers.orientation, tempM, image.Qr.markers.M_TR); // Re-arrange marker corners w.r.t orientation of the QR code
			this->cv_updateCornerOr(image.Qr.markers.orientation, tempO, image.Qr.markers.O_BL); // Re-arrange marker corners w.r.t orientation of the QR code

			//Shold be Boolean??????
			bool iflag = this->getIntersectionPoint(image.Qr.markers.M_TR[1], image.Qr.markers.M_TR[2], image.Qr.markers.O_BL[3], image.Qr.markers.O_BL[2], image.Qr.markers.N_BR_CORNER);


			image.Qr.src_qr_border.push_back(image.Qr.markers.L_TL[0]);
			image.Qr.src_qr_border.push_back(image.Qr.markers.M_TR[1]);
			image.Qr.src_qr_border.push_back(image.Qr.markers.N_BR_CORNER);
			image.Qr.src_qr_border.push_back(image.Qr.markers.O_BL[3]);

			image.Qr.dst_qr_border.push_back(cv::Point2f(0, 0));
			image.Qr.dst_qr_border.push_back(cv::Point2f(image.Qr.qr.cols, 0));
			image.Qr.dst_qr_border.push_back(cv::Point2f(image.Qr.qr.cols, image.Qr.qr.rows));
			image.Qr.dst_qr_border.push_back(cv::Point2f(0, image.Qr.qr.rows));

			//Draw contours on the image
			cv::cvtColor(image.gray, image.withselectedconturs, CV_GRAY2RGB);
			drawContours(image.withselectedconturs, image.contours, top, cv::Scalar(255, 200, 0), 2, 8, image.hierarchy, 0);

			int boundingContur = top;
			for (int i = 0; i <= 2; i++) {
				if (image.hierarchy[boundingContur][2] != -1) {
					boundingContur = image.hierarchy[boundingContur][2];
					if (i == 2) {
						drawContours(image.withselectedconturs, image.contours, boundingContur, cv::Scalar(255, 200, 0), 2, 8, image.hierarchy, 0);
					}
				} else {
					boundingContur = -1;
					break;
				}
			}
			drawContours(image.withselectedconturs, image.contours, right, cv::Scalar(0, 0, 255), 2, 8, image.hierarchy, 0);
			drawContours(image.withselectedconturs, image.contours, bottom, cv::Scalar(255, 0, 100), 2, 8, image.hierarchy, 0);

			//Plot with qr conturs:
			this->drawVectorToLines(
				image.Qr.src_qr_border,
				image.withselectedconturs,
				CV_RGB(0, 51, 255),
				1
			);
		}
	}
	 else {
		 return false;
	}
	return true;
}

bool Shqr::saveQr(workimage& image) {
	
	if (image.Qr.src_qr_border.size() == 4 && image.Qr.dst_qr_border.size() == 4) // Failsafe for WarpMatrix Calculation to have only 4 Points with src and dst
	{
		cv::Mat warp_matrix = cv::getPerspectiveTransform(image.Qr.src_qr_border, image.Qr.dst_qr_border);
		cv::warpPerspective(
			image.gray, 
			image.Qr.qr_raw, 
			warp_matrix, 
			cv::Size(image.Qr.qr.cols, image.Qr.qr.rows)
		);
		cv::copyMakeBorder(
			image.Qr.qr_raw, 
			image.Qr.qr, 
			SHQR_SAVED_QR_BORDER_SIZE, SHQR_SAVED_QR_BORDER_SIZE, SHQR_SAVED_QR_BORDER_SIZE, SHQR_SAVED_QR_BORDER_SIZE,
			cv::BORDER_CONSTANT, 
			cv::Scalar(255, 255, 255)
		);

		//cv::cvtColor(image.qr, image.qr_gray, CV_RGB2GRAY);
		//hreshold(image.qr_gray, image.qr_thres, 127, 255, CV_THRESH_BINARY);

		//threshold(qr_gray, qr_thres, 0, 255, CV_THRESH_OTSU);
		//for( int d=0 ; d < 4 ; d++){	src.pop_back(); dst.pop_back(); }
		return true;
	}
	return false;
}

void Shqr::setFinalBaseMarkers(imagecontainer& container) {

	container.base_markers_top = container.top.Qr.markers;
	container.base_markers_bottom = container.bottom.Qr.markers;

	//Add the correct offset for the stiching:
	container.base_markers_bottom.L_TL[0].y		+= (float)container.top.original.rows;
	container.base_markers_bottom.L_TL[1].y		+= (float)container.top.original.rows;
	container.base_markers_bottom.L_TL[2].y		+= (float)container.top.original.rows;
	container.base_markers_bottom.L_TL[3].y		+= (float)container.top.original.rows;
	container.base_markers_bottom.M_TR[0].y		+= (float)container.top.original.rows;
	container.base_markers_bottom.M_TR[1].y		+= (float)container.top.original.rows;
	container.base_markers_bottom.M_TR[2].y		+= (float)container.top.original.rows;
	container.base_markers_bottom.M_TR[3].y		+= (float)container.top.original.rows;
	container.base_markers_bottom.O_BL[0].y		+= (float)container.top.original.rows;
	container.base_markers_bottom.O_BL[1].y		+= (float)container.top.original.rows;
	container.base_markers_bottom.O_BL[2].y		+= (float)container.top.original.rows;
	container.base_markers_bottom.O_BL[3].y		+= (float)container.top.original.rows;
	container.base_markers_bottom.N_BR_CORNER.y += (float)container.top.original.rows;
}

#endif //Shqr_CPP