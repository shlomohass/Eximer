/* The Qr Engine Header
   By Shlomo Hassid
 */

#ifndef Shqr_H
#define Shqr_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include "ShqrBase.h"
#include "Inc.h"

class Shqr : public ShqrBase 
{
	
	public:

		Shqr(int dbg);

		bool scaleDownWidth(cv::Mat& inputOutput, int dstSize);

		void proc_image(workimage& image);

		bool find_markers(workimage& image);

		bool saveQr(workimage& image);

		void setFinalBaseMarkers(imagecontainer& container);
		
	private:

};

#endif //Shqr_H