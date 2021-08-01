#pragma once
#include <windows.h>
#include "Worker.h"
#include "Settings.h"
#include "OMDbInfo.h"
#include <map>
#define MAX_THREADS 4

class DVDHandler
{
private:
	std::map<char,Worker*> mapWorkers;
	Settings set;
	OMDbInfo *info = NULL;

	void LoadDrives();
	void UpdateTextPositions();
public:
	DVDHandler();
	void DiskLoaded(char drive);
	void DiskEjected(char drive);
	Settings* GetSettings()
	{
		return &set;
	}
	int GetWorkerCount() { return (int)mapWorkers.size(); }
	void SetupOutput(HWND hWnd);
	void WindowChanged(int x, int y, int width, int height);
};

