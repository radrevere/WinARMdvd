#include "MkvParseStream.h"

MkvParseStream::MkvParseStream()
{

}
std::string MkvParseStream::ParseMessage(char* buffer, size_t len)
{
	char* endPos = buffer + len;
	for (char *curPos = buffer; curPos < endPos; curPos++)
	{
		switch (*curPos)
		{
		case 'M':
			curPos += ParseMSG(curPos);
			break;
		case'P':
			switch (curPos[3])
			{
			case 'T':
				curPos += ParsePRGT(curPos);
				break;
			case 'C':
				curPos += ParsePRGC(curPos);
				break;
			case 'V':
				curPos += ParsePRGV(curPos);
				break;
			default:
				curPos += LineLen(curPos);
				break;
			}
			break;
		default:
			curPos += LineLen(curPos);
			break;
		}
	}
	std::string out = strMsg + "\r\n\r\n";
	out += strJob + " : " + strTotalProg + "\r\n";
	out += strTask + " : " + strCurProg + "\r\n";

	return out;
}

size_t MkvParseStream::ParseMSG(char* line)
{
	char* curPos = line + 4;
	int curItem = 0;
	bool done = false;
	strMsg = "";
	while (*curPos != '\n')
	{
		if (*curPos == '"' || *curPos == '\r')
		{
			curPos++;
			continue;
		}
		if (*curPos == ',')
		{
			curItem++;
			curPos++;
			continue;
		}
		switch (curItem)
		{
		case 0: // message id
			break;
		case 1: // message code
			break;
		case 2: // how many format args
			break;
		case 3: // actual message
			strMsg += *curPos;
			done = true;
			break;
		case 4:// format version
			break; 
		case 5: // 5 and up are the arguments
			break;
		}
		curPos++;
		if (*curPos == NULL)
		{
			break;
		}
	}
	return curPos - line;
}
size_t MkvParseStream::ParsePRGT(char* line)
{
	char* curPos = line + 5;
	int curItem = 0;
	bool done = false;
	strJob = "";
	while (*curPos != '\n')
	{
		if (*curPos == '"' || *curPos == '\r')
		{
			curPos++;
			continue;
		}
		if (*curPos == ',')
		{
			curItem++;
			curPos++;
			continue;
		}
		switch (curItem)
		{
		case 0: // task id
			break;
		case 1: // task code
			break;
		case 2: // how many format args
			strJob += *curPos;
			done = true;
			break;
		}
		curPos++;
		if (*curPos == NULL)
		{
			break;
		}
	}
	return curPos - line;
}
size_t MkvParseStream::ParsePRGC(char* line)
{
	char* curPos = line + 5;
	int curItem = 0;
	bool done = false;
	strTask = "";
	while (*curPos != '\n')
	{
		if (*curPos == '"' || *curPos == '\r')
		{
			curPos++;
			continue;
		}
		if (*curPos == ',')
		{
			curItem++;
			curPos++;
			continue;
		}
		switch (curItem)
		{
		case 0: // task id
			break;
		case 1: // task code
			break;
		case 2: // how many format args
			strTask += *curPos;
			done = true;
			break;
		}
		curPos++;
		if (*curPos == NULL)
		{
			break;
		}
	}
	return curPos - line;
}
size_t MkvParseStream::ParsePRGV(char* line)
{
	char* curPos = line + 5;
	int curItem = 0;
	std::string current = "";
	std::string total = "";
	std::string max = "";
	bool done = false;
	strCurProg = "";
	strTotalProg = "";
	while (*curPos != '\n')
	{
		if (*curPos == '\r')
		{
			curPos++;
			continue;
		}
		if (*curPos == ',')
		{
			curItem++;
			curPos++;
			continue;
		}
		switch (curItem)
		{
		case 0: // task id
			current += *curPos;
				break;
		case 1: // task code
			total += *curPos;
			break;
		case 2: // how many format args
			max += *curPos;
			break;
		}
		curPos++;
		if (*curPos == NULL)
		{
			break;
		}
	}
	float cur = (float)atoi(current.c_str());
	float curTotal = (float)atoi(total.c_str());
	float maxNum = (float)atoi(max.c_str());
	if (maxNum != 0)
	{
		char tmp[32] = { 0 };
		int per = (int)((cur / maxNum) * 100.0);
		int perOverall = (int)((curTotal / maxNum) * 100.0);
		_itoa_s(per, tmp, 10);
		strCurProg = tmp;
		strCurProg += "%";
		_itoa_s(perOverall, tmp, 10);
		strTotalProg = tmp;
		strTotalProg += "%";
	}
	return curPos - line;
}

size_t MkvParseStream::LineLen(char* line)
{
	size_t total = 0;
	char* curPos = line;
	while (*curPos != '\n')
	{
		curPos++;
		total++;
		if (*curPos == NULL)
		{
			break;
		}
	}
	return total;
}