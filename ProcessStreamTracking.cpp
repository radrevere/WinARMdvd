#include "ProcessStreamTracking.h"
ProcessStreamTracking::ProcessStreamTracking()
{
	memset(&procInfo, 0, sizeof(PROCESS_INFORMATION));
	stdout_read = NULL;
	stdout_write = NULL;
	stderr_read = NULL;
	stderr_write = NULL;
	memset(&sa, 0, sizeof(PROCESS_INFORMATION));
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	bReadStream = true;

	if (!CreatePipe(&stderr_read, &stderr_write, &sa, 0))
	{
		bReadStream = false;
	}
	if (!SetHandleInformation(stderr_read, HANDLE_FLAG_INHERIT, 0))
	{
		bReadStream = false;
	}
	if (!CreatePipe(&stdout_read, &stdout_write, &sa, 0))
	{
		bReadStream = false;
	}
	if (!SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0))
	{
		bReadStream = false;
	}
	if (bReadStream)
	{
		info.hStdError = stderr_write;
		info.hStdOutput = stdout_write;
		info.dwFlags |= STARTF_USESTDHANDLES;
	}
	else
	{
		CloseReadHandles();
		CloseWriteHandles();
	}
}
ProcessStreamTracking::~ProcessStreamTracking()
{
	CloseReadHandles();
	CloseWriteHandles();
	CloseHandle(procInfo.hThread);
	CloseHandle(procInfo.hProcess);
}
BOOL ProcessStreamTracking::StartProcess(const char* command)
{
	BOOL created = CreateProcessA(NULL, (char*)command, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &info, &procInfo);
	if (!created)
	{
		if (bReadStream)
		{
			CloseWriteHandles();
			CloseReadHandles();
		}
		return FALSE;
	}
	CloseWriteHandles();
	return TRUE;
}
int ProcessStreamTracking::ReadFromStream(char* buffer, size_t bufSize, DWORD& bytesRead, bool errStream)
{
	bytesRead = 0;
	if (!bReadStream)
	{
		return FAILED;
	}
	DWORD state = WaitForSingleObject(procInfo.hProcess, 0);
	if (state == WAIT_OBJECT_0)
	{
		// process is done
		return PROC_EXITED;
	}
	HANDLE hStream = (errStream ? stderr_read : stdout_read);
	DWORD bytesAvailable, bytesLeft, bytesReadOut;
	bytesReadOut = bytesAvailable = bytesLeft = 0;
	buffer[0] = NULL;
	int retVal = FAILED;
	// read from output stream
	if (bReadStream && PeekNamedPipe(hStream, buffer, bufSize, &bytesReadOut, &bytesAvailable, &bytesLeft))
	{
		if (bytesAvailable > 0)
		{
			BOOL bSuccess = ReadFile(hStream, buffer, bufSize, &bytesReadOut, NULL);
			if (bSuccess)
			{
				buffer[bytesReadOut] = 0;
				bytesRead = bytesReadOut;
				retVal = SUCCEEDED;
			}
		}
	}
	
	return retVal;
}
int ProcessStreamTracking::ReadFromOutStream(char* buffer, size_t bufSize, DWORD& bytesRead)
{
	return ReadFromStream(buffer, bufSize, bytesRead, false);
}
int ProcessStreamTracking::ReadFromErrStream(char* buffer, size_t bufSize, DWORD& bytesRead)
{
	return ReadFromStream(buffer, bufSize, bytesRead, true);
}
void ProcessStreamTracking::CloseWriteHandles()
{
	if (stdout_write != NULL)
	{
		CloseHandle(stdout_write);
		stdout_write = NULL;
	}
	if (stderr_write != NULL)
	{
		CloseHandle(stderr_write);
		stderr_write = NULL;
	}
}
void ProcessStreamTracking::CloseReadHandles()
{
	if (stdout_read != NULL)
	{
		CloseHandle(stdout_read);
		stdout_read = NULL;
	}
	if (stderr_read != NULL)
	{
		CloseHandle(stderr_read);
		stderr_read = NULL;
	}

}