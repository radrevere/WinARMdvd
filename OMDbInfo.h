#pragma once
#include <string>

#define BASE_URL_FMT "/?apikey=%s&t=%s"

class OMDbInfo
{
private:
	std::string apiKey = "a49b7d28";

	std::string GetWebInfo(std::wstring server, std::wstring url, std::string outFile);
public:
	OMDbInfo(std::string key);

	std::string GetOmdbInfo(std::string title);
	std::string DownloadFile(std::string url, std::string fileName);
};

