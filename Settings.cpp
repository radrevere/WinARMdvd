#include "Settings.h"
#include "ini.h"

#define INI_FILE "dvdhelper.ini"

Settings::Settings()
{
	strMkvExe = "C:\\Program Files (x86)\\MakeMKV\\makemkvcon64.exe";
	strOutRoot = "g:\\Movies\\";
	minLength = 1000;
	cache = 1024;
	robot = true;
	decrypt = true;
	directio = true;
	eject = true;
	LoadSettings();
}
Settings::~Settings()
{
	
}
#define CMD_BUF_SIZE 128
std::string Settings::GetMkvCommand(char disk, std::string dvdName)
{
	//C:\Program Files (x86)\MakeMKV\makemkvcon64.exe --minlength = 300 - r --decrypt --directio = true mkv dev:H:\ all g:\Movies
	std::string out = strMkvExe;
	char buf[CMD_BUF_SIZE] = { 0 };
	if (minLength > 0)
	{
		sprintf_s(buf, CMD_BUF_SIZE, " --minlength=%d", minLength);
		out += buf;
	}
	if (robot)
	{
		out += " -r";
	}
	if (cache > 0)
	{
		sprintf_s(buf, CMD_BUF_SIZE, " --cache=%d", cache);
		out += buf;
	}
	if (decrypt)
	{
		out += " --decrypt";
	}
	if (directio)
	{
		out += " --directio=true";
	}
	/*else
	{
		out == " --directio=false";
	}*/
	sprintf_s(buf, CMD_BUF_SIZE, " --progress=-stdout mkv dev:%c:\\ all ", disk);
	out += buf;
	out += strOutRoot + dvdName;
	
	return out;
}

void Settings::LoadSettings()
{
	mINI::INIFile file(INI_FILE);
	mINI::INIStructure ini;
	if (!file.read(ini))
	{
		// exit with no changes to the defaults
		return;
	}

	strMkvExe = ini["cmd"]["mkvpath"];
	strOutRoot = ini["cmd"]["outroot"];

	// ensure the root path ends with a slash
	if (strOutRoot[strOutRoot.size() - 1] != '\\')
	{
		strOutRoot += "\\";
	}

	std::string& tmp = ini["cmd"]["minlength"];
	minLength = atoi(tmp.c_str());

	tmp = ini["cmd"]["cache"];
	cache = atoi(tmp.c_str());

	tmp = ini["cmd"]["robot"];
	robot = true;
	if (tmp == "0")
	{
		robot = false;
	}

	tmp = ini["cmd"]["decrypt"];
	decrypt = true;
	if (tmp == "0")
	{
		decrypt = false;
	}

	tmp = ini["cmd"]["directio"];
	directio = true;
	if (tmp == "0")
	{
		directio = false;
	}

	tmp = ini["cmd"]["eject"];
	eject = true;
	if (tmp == "0")
	{
		eject = false;
	}

	strOMDbkey = ini["OMDb"]["key"];
}