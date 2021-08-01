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

	std::string tmp = ini.get("cmd").get("mkvpath");
	if (tmp != "")
	{
		strMkvExe = tmp;
	}
	// ensure the root path ends with a slash
		if (strMkvExe[strMkvExe.size() - 1] != '\\')
		{
			strMkvExe += "\\";
		}

	tmp = ini.get("cmd").get("outroot");
	if (tmp != "")
	{
		strOutRoot = tmp;
	}
	// ensure the root path ends with a slash
	if (strOutRoot[strOutRoot.size() - 1] != '\\')
	{
		strOutRoot += "\\";
	}

	tmp = ini.get("cmd").get("minlength");
	if (tmp != "")
	{
		minLength = atoi(tmp.c_str());
	}
	
	tmp = ini.get("cmd").get("cache");
	if (tmp != "")
	{
		cache = atoi(tmp.c_str());
	}

	tmp = ini.get("cmd").get("use64bit");
	use64bit = true;
	if (tmp == "0")
	{
		use64bit = false;
	}

	tmp = ini.get("cmd").get("robot");
	robot = tmp == "0" ? false : true;

	tmp = ini.get("cmd").get("decrypt");
	decrypt = tmp == "0" ? false : true;

	tmp = ini.get("cmd").get("directio");
	directio = tmp == "0" ? false : true;

	tmp = ini.get("cmd").get("eject");
	eject = tmp == "0" ? false : true;

	strOMDbkey = ini.get("OMDb").get("key");

	tmp = ini.get("multi").get("series");
	series = tmp=="0"?false:true;

	tmp = ini.get("window").get("width");
	if (tmp != "")
	{
		wndWide = atoi(tmp.c_str());
	}
	tmp = ini.get("window").get("height");
	if (tmp != "")
	{
		wndHigh = atoi(tmp.c_str());
	}
	tmp = ini.get("window").get("xpos");
	if (tmp != "")
	{
		xPos = atoi(tmp.c_str());
	}
	tmp = ini.get("window").get("ypos");
	if (tmp != "")
	{
		yPos = atoi(tmp.c_str());
	}
}