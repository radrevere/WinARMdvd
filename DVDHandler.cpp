#include "DVDHandler.h"
#include "OMDbInfo.h"
#include <string>

DVDHandler::DVDHandler()
{
	info = new OMDbInfo(set.strOMDbkey);
	LoadDrives();
}

void DVDHandler::LoadDrives()
{
	char volBuf[MAX_PATH+1] = {0};
	DWORD driveMask = GetLogicalDrives();
	char drive = 'A';
	while (driveMask)
	{
		if (driveMask & 1)
		{
			sprintf_s(volBuf, MAX_PATH, "%c:\\", drive);
			UINT ret = GetDriveTypeA(volBuf);
			if (ret == DRIVE_CDROM)
			{
				Worker* w = new Worker(drive, &set);
				mapWorkers[drive] = w;
			}
		}
		driveMask >>= 1;
		drive++;
	}
}

void DVDHandler::SetupOutput(HWND parent)
{
	std::map<char, Worker*>::iterator it;
	for (it = mapWorkers.begin(); it != mapWorkers.end(); it++)
	{
		Worker* w = it->second;
		HWND txtBox = CreateWindowEx(
			0, L"EDIT",   // predefined class 
			NULL,         // no window title 
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_READONLY |
			ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
			0, 0, 300, 300,   // set size in WM_SIZE message 
			parent,         // parent window 
			0,   // edit control ID 
			(HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE),
			NULL);
		w->SetTextWindow(txtBox);
	}
	UpdateTextPositions();
}

void DVDHandler::UpdateTextPositions()
{
	int wide = set.wndWide;
	if (mapWorkers.size() > 1)
	{
		wide = set.wndWide / (int)mapWorkers.size();
	}
	std::map<char, Worker*>::iterator it;
	int left = 0;
	for (it = mapWorkers.begin(); it != mapWorkers.end(); it++)
	{
		HWND hwnd = it->second->GetTextWindow();
		if (hwnd == NULL)
		{
			continue;
		}
		MoveWindow((HWND)(hwnd),
			left, 0,                  // starting x- and y-coordinates 
			wide,        // width of client area 
			set.wndHigh,        // height of client area 
			TRUE);                 // repaint window 
		left += wide;
	}
}

void DVDHandler::WindowChanged(int x, int y, int width, int height)
{
	
	set.xPos = x;
	set.yPos = y;
	set.wndWide = width;
	set.wndHigh = height;
	UpdateTextPositions();
	set.SaveSettings();
}

void DVDHandler::DiskLoaded(char drive)
{
	Worker* w = mapWorkers[drive];
	if (w == NULL)
	{
		Worker* w = new Worker(drive, &set);
		mapWorkers[drive] = w;
		w->Start();
	}
	else
	{
		w->Start();
	}
}

void DVDHandler::DiskEjected(char drive)
{
	Worker* w = mapWorkers[drive];
	if (w)
	{
		w->Stop();
		// cleanup
		//mapWorkers[drive] = NULL;
		//delete w;
	}
}