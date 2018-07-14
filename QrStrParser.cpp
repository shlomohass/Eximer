
/* The Qr Engine Header
By Shlomo Hassid
*/

#ifndef QrStrParser_CPP
#define QrStrParser_CPP


#include "QrStrParser.h"



QrStrParser::QrStrParser(int dbg) : ShqrBase(dbg)
{
	this->Id = -1;
	this->TotalPages = -1;
	this->PageNumber = -1;
	this->QuestionCount = -1;
	this->StudentId = -1;
}

int QrStrParser::parse(const std::string& _str) {
	//Split:
	std::vector<std::string> parsedQr = ShqrBase::splitStr(_str, SHQR_QR_STRING_TOKEN);
	std::vector<std::string> parsedQrParts;
	//Validte found:
	if (parsedQr.size() < 1) { return 1; }
	//Parse:
	parsedQrParts = ShqrBase::splitStr(parsedQr[0], SHQR_QR_PARTS_TOKEN);
	//Validate parts:
	if (parsedQrParts.size() != SHQR_QR_MINPARTS_COUNT) { return 2; }
	//Save:
	this->raw = parsedQr[0];
	this->Id = std::stoi(parsedQrParts[1]);
	this->TotalPages = std::stoi(parsedQrParts[2]);
	this->PageNumber = std::stoi(parsedQrParts[3]);
	this->QuestionCount = std::stoi(parsedQrParts[4]);
	this->AnswersCount = ShqrBase::splitInt(parsedQrParts[5]);
	this->StudentId = std::stoi(parsedQrParts[6]);
	//Validate Answers:
	if (QuestionCount != (int)AnswersCount.size()) {
		return 3;
	}
	return 0;
}
QrStrParser::~QrStrParser()
{
}

#endif //QrStrParser_CPP