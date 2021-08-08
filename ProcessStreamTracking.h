#pragma once
#include <Windows.h>

class ProcessStreamTracking
{
private:
	STARTUPINFOA info = { sizeof(info) };
	PROCESS_INFORMATION procInfo;
	HANDLE stdout_read;
	HANDLE stdout_write;
	HANDLE stderr_read;
	HANDLE stderr_write;
	SECURITY_ATTRIBUTES sa;
	bool bReadStream;

	int ReadFromStream(char* buffer, size_t bufSize, DWORD& bytesRead, bool errStream);
	void CloseWriteHandles();
	void CloseReadHandles();

public:
	static const int SUCCEEDED = 1;
	static const int FAILED = 0;
	static const int PROC_EXITED = -1;

	ProcessStreamTracking();
	~ProcessStreamTracking();

	BOOL StartProcess(const char* command);
	int ReadFromOutStream(char* buffer, size_t bufSize, DWORD& bytesRead);
	int ReadFromErrStream(char* buffer, size_t bufSize, DWORD& bytesRead);
};

