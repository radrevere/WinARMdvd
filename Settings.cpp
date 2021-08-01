#include "Settings.h"
#include "ini.h"

#define INI_FILE "dvdhelper.ini"

Settings::Settings()
{
	strMkvExe = "C:\\Program Files (x86)\\MakeMKV\\makemkvcon64.exe";
	strOutRoot = "g:\\Movies\\";
	minLength = 1000;
	cache = 1024;
	use64bit = true;
	robot = true;
	decrypt = true;
	directio = true;
	eject = true;
	series = false;
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
	if (out[out.size() - 1] != '\\')
	{
		out += "\\";
	}

	if (use64bit)
	{
		out += "makemkvcon64.exe";
	}
	else
	{
		out += "makemkvcon.exe";
	}
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

bool Settings::SaveSettings()
{
	mINI::INIFile file(INI_FILE);
	mINI::INIStructure ini;
	char tmpBuf[512] = { 0 };
	if (!file.read(ini))
	{
		return false;
	}
	ini["cmd"]["mkvpath"] = strMkvExe;
	ini["cmd"]["outroot"] = strOutRoot;
	_itoa_s(minLength, tmpBuf, 10);
	ini["cmd"]["minlength"] = tmpBuf;
	_itoa_s(cache, tmpBuf, 10);
	ini["cmd"]["cache"] = tmpBuf;
	ini["cmd"]["robot"] = robot ? "1":"0";
	ini["cmd"]["decrypt"] = decrypt ? "1" : "0";
	ini["cmd"]["directio"] = directio ? "1" : "0";
	ini["cmd"]["eject"] = eject? "1" : "0";
	ini["cmd"]["use64bit"] = use64bit ? "1" : "0";
	ini["OMDb"]["key"] = strOMDbkey;
	ini["multi"]["series"] = series?"1":"0";
	_itoa_s(wndWide, tmpBuf, 10);
	ini["window"]["width"] = tmpBuf;
	_itoa_s(wndHigh, tmpBuf, 10);
	ini["window"]["height"] = tmpBuf;
	_itoa_s(xPos, tmpBuf, 10);
	ini["window"]["xpos"] = tmpBuf;
	_itoa_s(yPos, tmpBuf, 10);
	ini["window"]["ypos"] = tmpBuf;
	return file.write(ini);
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

	tmp = ini["cmd"]["use64bit"];
	use64bit = true;
	if (tmp == "0")
	{
		use64bit = false;
	}

	tmp = ini["cmd"]["robot"];
	robot = tmp == "0" ? false : true;

	tmp = ini["cmd"]["decrypt"];
	decrypt = tmp == "0" ? false : true;

	tmp = ini["cmd"]["directio"];
	directio = tmp == "0" ? false : true;

	tmp = ini["cmd"]["eject"];
	eject = tmp == "0" ? false : true;

	strOMDbkey = ini["OMDb"]["key"];

	tmp = ini["multi"]["series"];
	series = tmp=="0"?false:true;

	tmp = ini["window"]["width"];
	wndWide = atoi(tmp.c_str());
	tmp = ini["window"]["height"];
	wndHigh = atoi(tmp.c_str());
	tmp = ini["window"]["xpos"];
	xPos = atoi(tmp.c_str());
	tmp = ini["window"]["ypos"];
	yPos = atoi(tmp.c_str());
}