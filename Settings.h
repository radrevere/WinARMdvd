#pragma once
#include <string>
#include <map>

typedef struct DriveInfo
{
	char letter;
	char num;
	void* txtOut;
	DriveInfo(char l, char n, void* txt)
	{
		letter = l;
		num = n;
		txtOut = txt;
	}
}DRIVE_INFO;
class Settings
{
	//C:\Program Files (x86)\MakeMKV\makemkvcon64.exe --minlength = 300 - r --decrypt --directio = true mkv disc : 0 all g : \Movies
public :
	std::string strMkvExe;
	std::string strOutRoot;
	std::string strOMDbkey;
	int minLength;
	int cache;
	bool robot;
	bool decrypt;
	bool directio;
	bool eject;

	Settings();
	~Settings();
	std::string GetMkvCommand(char disk, std::string dvdName);

private:
	void LoadSettings();
};

