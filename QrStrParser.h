
/* The Qr Engine Header
By Shlomo Hassid
*/

#ifndef QrStrParser_H
#define QrStrParser_H

#include "ShqrBase.h"
#include "Inc.h"
#include <iostream>
#include <vector>

class QrStrParser : public ShqrBase
{


	// 000000099:01:01:06:444422:[00000000|00000095]
	// - ID				-> Int 9
	// - Total Pages	-> Int 2
	// - Page Number	-> Int 2
	// - Question Count -> Int 2
	// - Answers groups -> Int 1 one digit chained (0 is open question)
	// - Student Id     -> int 8 (all zeros is unknown)

public:

	std::string raw;
	int Id;
	int TotalPages;
	int PageNumber;
	int QuestionCount;
	std::vector<int> AnswersCount;
	int StudentId;

	QrStrParser(int dbg);

	int parse(const std::string&  _str);

	~QrStrParser();

};


#endif //QrStrParser_H