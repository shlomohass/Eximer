/* The Json Response
By Shlomo Hassid
*/

#ifndef ResJson_H
#define ResJson_H

#include "ShqrBase.h"
#include "Inc.h"
#include <iostream>

class ResJson : public ShqrBase
{
	std::vector<std::string> mesContainer;
	std::string unknown;
public:
	
	//General Params:
	int         code;
	std::string	message;
	
	//Exam params:
	int EaxmId;
	int TotalPages;
	int PageNumber;
	int QuestionCount;
	int StudentId;

	//Answers:
	std::vector<question> quest;
	
	//Methods:
	ResJson(int dbg, const std::vector<std::string>& resmes);
	void SetCode(int _code);
	void SetBaseInfo(int _EaxmId, int _TotalPages, int _PageNumber, int _QuestionCount, int _StudentId);
	std::string getprint();
	virtual ~ResJson();
};

#endif
