#pragma once
#include <string>

struct CINFO
{

};
class MkvParseStream
{
public:
	MkvParseStream();
	std::string ParseMessage(char* lastRead, size_t len);
private:
	std::string strMsg; // MSG
	std::string strJob; // PRGT
	std::string strTask; // PRGC
	std::string strCurProg; // PRGV
	std::string strTotalProg;

	size_t ParseMSG(char* line);
	size_t ParsePRGT(char* line);
	size_t ParsePRGC(char* line);
	size_t ParsePRGV(char* line);
	size_t LineLen(char* line);
};