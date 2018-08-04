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

//Sets the resuts code:
void ResJson::SetCode(int _code)
{
	this->code = _code;
	if ((int)this->mesContainer.size() > this->code) {
		this->message = this->mesContainer[this->code];
	}
	else {
		this->message = this->unknown;
	}
}

//Sets basic information -> parsed from QR;
void ResJson::SetBaseInfo(int _EaxmId, int _TotalPages, int _PageNumber, int _QuestionCount, int _StudentId) {
	this->EaxmId		= _EaxmId;
	this->TotalPages	= _TotalPages;
	this->PageNumber	= _PageNumber;
	this->QuestionCount = _QuestionCount;
	this->StudentId		= _StudentId;
}

//Output result;
std::string ResJson::getprint() {

	/*
	{
	  "code"      : 0,
	  "message"       : "shlomi",
	  "result" : {
		  "id"              : 23,
		  "pagesCount"      : 2,
		  "currentPage"     : 1,
		  "questionsCount"  : 6,
		  "studentId"       : 23,
		  "parsed"  : [
			  { "questNum" : 1 , "type" : 0, "mark" : 2 },
			  { "questNum" : 2 , "type" : 1, "mark" : 4 },
			  { "questNum" : 3 , "type" : 0, "mark" : 3 }
		  ]
	  }
	}
	*/
	std::string jsonstr =
		"{\"code\" : @1, \"message\" : \"@2\" @3 }";
	std::string infostr =  ", \"result\" : { ";
				infostr += "\"id\" : @4, ";
				infostr += "\"pagesCount\" : @5, ";
				infostr += "\"currentPage\" : @6, ";
				infostr += "\"questionsCount\" : @7, ";
				infostr += "\"studentId\" : @8, ";
				infostr += "\"parsed\" : [@9] }";
				std::string queststr = "{ \"questNum\" : @1 , \"type\" : @2, \"mark\" : @3 }";
	std::vector<std::string> parsed;
	//Set code:
	jsonstr = this->ReplaceAll(jsonstr, "@1", std::to_string(this->code));

	if (this->code >= 0) {
		//Set code:
		if (this->code < (int)this->mesContainer.size()) {
			jsonstr = this->ReplaceAll(jsonstr, "@2", this->mesContainer[this->code]);
		} else {
			jsonstr = this->ReplaceAll(jsonstr, "@2", this->unknown);
		}

		//Set results:
		if (this->code > 0) {
			//An error:
			jsonstr = this->ReplaceAll(jsonstr, "@3", "");
		} else {
			//set id:
			infostr = this->ReplaceAll(infostr, "@4", std::to_string(this->EaxmId));
			//set total pages:
			infostr = this->ReplaceAll(infostr, "@5", std::to_string(this->TotalPages));
			//set currentPage:
			infostr = this->ReplaceAll(infostr, "@6", std::to_string(this->PageNumber));
			//set questionsCount:
			infostr = this->ReplaceAll(infostr, "@7", std::to_string(this->QuestionCount));
			//set studentId:
			infostr = this->ReplaceAll(infostr, "@8", std::to_string(this->StudentId));

			//Set questions:
			if (!this->quest.empty()) {
				for (int i = 0; i < (int)this->quest.size(); i++) {
					std::string qAdd = this->ReplaceAll(queststr, "@1", std::to_string(i + 1));
					qAdd = this->ReplaceAll(qAdd, "@2", std::to_string((int)this->quest[i].type));
					qAdd = this->ReplaceAll(qAdd, "@3", std::to_string(this->quest[i].mark ? 1 : 0));
					parsed.push_back(qAdd);
				}
				infostr = this->ReplaceAll(infostr, "@9", this->implode(parsed, ","));
			} else {
				infostr = this->ReplaceAll(infostr, "@9", "");
			}

			//Join all:
			jsonstr = this->ReplaceAll(jsonstr, "@3", infostr);
		}
		
	}

	//Print
	return jsonstr;
}


ResJson::~ResJson()
{
}

#endif

