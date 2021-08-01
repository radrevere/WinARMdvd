#pragma once
#include <Windows.h>
#include "Settings.h"
class SettingsDialog
{
private:
	Settings* set;
public:
	SettingsDialog();
	void Show(HWND parent, Settings *settings);
	void SaveSettings(HWND hwnd);
	void InitControls(HWND hwnd);
};

