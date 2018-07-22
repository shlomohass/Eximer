/* The Json Response
By Shlomo Hassid
*/

#ifndef ResJson_CPP
#define ResJson_CPP

#include "ResJson.h"
#include <algorithm>
#include <string>


ResJson::ResJson(int dbg, const std::vector<std::string>& resmes) : ShqrBase(dbg)
{

	//Gen:
	this->mesContainer = resmes;
	this->unknown = SHQR_JSONRES_UNKNOWN_MES;

	//General Params:
	this->code		= 0;
	this->message	= "";

	//Exam params:
	this->EaxmId		= 0;
	this->TotalPages	= 0;
	this->PageNumber	= 0;
	this->QuestionCount = 0;
	this->StudentId		= 0;

}

void ResJson::SetCode(int _code)
{
	this->code = 0;
	if ((int)this->mesContainer.size() > this->code) {
		this->message = this->mesContainer[this->code];
	}
	else {
		this->message = this->unknown;
	}
}

std::string ResJson::getprint() {

	std::string jsonstr =
		"{\"resCode\" : @1, \"resMessage\" : \"@2\" }";

	//Set code:
	jsonstr = this->ReplaceAll(jsonstr, "@1", std::to_string(this->code));

	if (this->code >= 0) {
		//Error code:
		if (this->code < (int)this->mesContainer.size()) {
			jsonstr = this->ReplaceAll(jsonstr, "@2", this->mesContainer[this->code]);
		} else {
			jsonstr = this->ReplaceAll(jsonstr, "@2", this->unknown);
		}
	}

	//Print
	return jsonstr;
}


ResJson::~ResJson()
{
}

#endif

