#pragma once
#include <Windows.h>
#include <string>
#include "Settings.h"
#include "OMDbInfo.h"
using namespace std;

#define STD_BUFFSIZE 4096

bool ejectDisk(char driveLetter);
std::string GetLastErrorAsString();
std::string FindPosterUrl(std::string json);
bool fileExists(std::string file);

class Worker
{
private:
	std::string driveLetter;
	std::string strTitle;
	// DVDInfo info;
	HANDLE hThread;
	DWORD threadId;
	bool run;
	Settings* set;
	OMDbInfo* omdb;
	HWND txtOut;

	static DWORD WINAPI WorkerThread(LPVOID lpParam);
	std::string RenameFile(std::string& folder);
	static bool RipDisk(Worker * wrkr);

public:
	Worker(char drive, Settings* settings);
	~Worker();
	void Stop();
	bool IsStarted();
	void Start();
	void SetTextWindow(HWND hwnd);// { txtOut = hwnd; }
	HWND GetTextWindow() { return txtOut; }
};

