#include "Worker.h"
#include <mmsystem.h>
#include <list>
//#include "dvdread/ifo_types.h"
#define STD_BUFFSIZE 4096

bool ejectDisk(char driveLetter)
{
    char drive[32] = { 0 };
    sprintf_s(drive, "\\\\.\\%c:", driveLetter);
    HANDLE cdRom = CreateFileA(drive, GENERIC_READ | GENERIC_WRITE , 0, NULL, OPEN_EXISTING, 0, NULL);
    int retries = 0;
    while (cdRom == INVALID_HANDLE_VALUE)
    {
        Sleep(1000); // give up after about 10 seconds
        if (retries == 10)
        {
            return false;
        }
        retries++;
        cdRom = CreateFileA(drive, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    }
    DWORD bytes = 0;
    DeviceIoControl(cdRom, IOCTL_STORAGE_EJECT_MEDIA, NULL, 0, NULL, 0, &bytes, NULL);

    CloseHandle(cdRom);
    return true;
}

bool fileExists(std::string file)
{
    WIN32_FIND_DATAA FindFileData;
    HANDLE handle = FindFirstFileA(file.c_str(), &FindFileData);
    bool found = false;
    if (handle != INVALID_HANDLE_VALUE)
    {
        found = true;
        FindClose(handle);
    }
    return found;
}

std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

std::string FindPosterUrl(std::string json)
{
    std::string strFind = "\"Poster\":";
    size_t pos1 = json.find(strFind);
    if (pos1 == std::string::npos)
    {
        return "";
    }
    pos1 = json.find('\"', pos1 + strFind.size()) + 1;
    size_t pos2 = json.find('\"', pos1);
    return json.substr(pos1, pos2 - pos1);
}

std::string GetProgress(char* buf)
{
    std::string percent = " ";
    if (strncmp("PRGV", buf, 4) == 0)
    {
        // I do have a progress output
        char curNum[32] = { 0 };
        char* curPos = curNum;
        int curIdx = 0;
        char max[32] = { 0 };
        char* cur = buf + 5;
        while (*cur != '\n' && *cur != '\r' && curIdx < 64)
        {
            if (*cur == ',')
            {
                curPos = max;
            }
            else
            {
                *curPos = *cur;
                curPos++;
                *curPos = NULL; // ensure null termination
            }
            cur++;
        }
        int prog = atoi(curNum);
        int total = atoi(max);
        if (total != 0)
        {
            total = (int)(((float)prog / (float)total) * 100);
            _itoa_s(total, curNum, 10);
            percent += curNum;
            percent += "%\n";
        }
    }
    return percent;
}

Worker::Worker(char drive, Settings * settings)
{
    set = settings;
    omdb = NULL;
    if (set->strOMDbkey != "")
    {
        omdb = new OMDbInfo(set->strOMDbkey);
    }
    //dInfo = set->mapDrives[drive];
    //std::string strDrive = "";
    driveLetter = drive;
    driveLetter += ":\\";
    hThread = NULL;
    threadId = NULL;
	//driveLetter = drive;
    txtOut = NULL;
    //Start();
}

Worker::~Worker()
{
    Stop();
    if (omdb)
    {
        delete omdb;
        omdb = NULL;
    }
}

void Worker::Start()
{
    if (hThread != NULL)
    {
        Stop();
    }
    hThread = NULL;
    threadId = NULL;
    run = true;
    std::string str = driveLetter + " Loading Disk Info...";
    SendMessageA(txtOut, WM_SETTEXT, 0, (LPARAM)str.c_str());
    hThread = CreateThread(NULL, 0, WorkerThread, this, 0, &threadId);
}

void Worker::Stop()
{
    if (hThread)
    {
        run = false;
        WaitForSingleObject(hThread, 5000);
        // thread function will close itself out
        CloseHandle(hThread);
        threadId = -1;
        hThread = NULL;
    }
}

void Worker::SetTextWindow(HWND hwnd)
{
    txtOut = hwnd;
    SendMessageA(txtOut, WM_SETTEXT, 0, (LPARAM)driveLetter.c_str());
}

bool Worker::IsStarted()
{
    return hThread > 0;
}

std::string Worker::RenameFile(std::string& folder)
{
    // I don't know the name of the file makeMKV created
    // nor if it happend to RIP more than one file
    WIN32_FIND_DATAA ffd;
    char szTmp[MAX_PATH];
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;
    std::string errOut = "";
    std::string filename = "";

    std::list<std::string> lstFiles;

    sprintf_s(szTmp, "%s*", folder.c_str());

    if (strTitle == "")
    {
        strTitle = "unknown";
    }

    //************************************
    // TODO: rename all .mkv output
    // fmt = [title] - pt1.mkv
    //       [title] - pt2.mkv, etc...
    //************************************
    hFind = FindFirstFileA(szTmp, &ffd);
    do
    {
        // skip directories; it will find ./ and ../
        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            std::string tmpStr = ffd.cFileName;
            if (tmpStr.find(".mkv") > 0)
            {
                lstFiles.push_back(tmpStr);
            }
        }
    } while (FindNextFileA(hFind, &ffd) != 0);

    if (lstFiles.size() == 0)
    {
        errOut = "Unable to find mkv file for rename.";
    }
    else
    {
        bool many = lstFiles.size() > 1;
        std::list<std::string>::iterator itr;
        int part = 1;
        for (itr = lstFiles.begin(); itr != lstFiles.end(); itr++)
        {
            filename = folder + *itr;
            std::string tofile = folder + strTitle;
            
            if (many)
            {
                if (set->series)
                {
                    sprintf_s(szTmp, MAX_PATH, " - s01e%02d", part);
                }
                else
                {
                    sprintf_s(szTmp, MAX_PATH, " - part%d", part);
                }
                tofile += szTmp;
                part++;
            }
            tofile += ".mkv";
            if (MoveFileA(filename.c_str(), tofile.c_str()) == FALSE)
            {
                errOut += "Unable to rename file " + filename;
            }
        }
    }

    FindClose(hFind);
    return errOut;
}

DWORD WINAPI Worker::WorkerThread(LPVOID lpParam)
{
    Worker* self = (Worker*)lpParam;
    std::string strOutToUi = "";
    if (fileExists(self->driveLetter + "VIDEO_TS\\VIDEO_TS.IFO") == false)
    {
        strOutToUi = self->driveLetter + " Not a DVD...could not find VIDEO_TS\\VIDEO_TS.IFO ";
        SendMessageA(self->txtOut, WM_SETTEXT, 0, (LPARAM)strOutToUi.c_str());
        self->run = false;
        return 0;
    }
    char szName[MAX_PATH] = { 0 };
    if (GetVolumeInformationA(self->driveLetter.c_str(), szName, MAX_PATH, NULL, 0, 0, NULL, 0))
    {
        self->strTitle = szName;
        std::string outDir = self->set->strOutRoot + self->strTitle;
        CreateDirectoryA(outDir.c_str(), NULL);
        strOutToUi = "Starting ";
        strOutToUi += self->strTitle;
        SendMessageA(self->txtOut, WM_SETTEXT, 0, (LPARAM)strOutToUi.c_str());
    }
    else
    {
        strOutToUi = self->driveLetter + " Unable to get volume information: ";
        strOutToUi += GetLastErrorAsString();
        SendMessageA(self->txtOut, WM_SETTEXT, 0, (LPARAM)strOutToUi.c_str());
        self->run = false;
        return 0;
    }

    std::string cmd = self->set->GetMkvCommand(self->driveLetter[0], self->strTitle);

    STARTUPINFOA info = { sizeof(info) };
    PROCESS_INFORMATION procInfo;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    bool bReadStream = true;
    HANDLE stdout_read = NULL;
    HANDLE stdout_write = NULL;
    HANDLE stderr_read = NULL;
    HANDLE stderr_write = NULL;
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
    info.hStdError = stderr_write;
    info.hStdOutput = stdout_write;
    info.dwFlags |= STARTF_USESTDHANDLES;
    BOOL created = CreateProcessA(NULL, (char*)cmd.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &info, &procInfo);
    if (!created)
    {
        if (bReadStream)
        {
            CloseHandle(stdout_read);
            CloseHandle(stderr_read);
            CloseHandle(stdout_write);
            CloseHandle(stderr_write);
        }
        CloseHandle(self->hThread);
        self->threadId = -1;
        self->hThread = NULL;
        return 1;
    }
    CloseHandle(stdout_write);
    CloseHandle(stderr_write);
    
    char stdBuf[STD_BUFFSIZE+1] = {0};
    bool bSuccess = false;
    DWORD bytesRead = 0;
    DWORD bytesAvailable = 0;
    DWORD bytesLeft = 0;
    
    while (self->run)
    {
        Sleep(1000);
        DWORD state = WaitForSingleObject(procInfo.hProcess, 0);
        if (state == WAIT_OBJECT_0)
        {
            // process has terminated
            self->run = false;
            //continue;
        }
        strOutToUi = "";
        bytesRead = bytesAvailable = bytesLeft = 0;
        stdBuf[0] = NULL;
        // read from output stream
        if (bReadStream && PeekNamedPipe(stdout_read, stdBuf, STD_BUFFSIZE, &bytesRead, &bytesAvailable, &bytesLeft))
        {
            if (bytesAvailable > 0)
            {
                bSuccess = ReadFile(stdout_read, stdBuf, STD_BUFFSIZE, &bytesRead, NULL);
                if (bSuccess)
                {
                    stdBuf[bytesRead] = 0;
                    strOutToUi = self->driveLetter + " Title: " + self->strTitle + "\n";
                    std::string progress = GetProgress(stdBuf);
                    if (progress != "")
                    {
                        strOutToUi += progress;
                        SendMessageA(self->txtOut, WM_SETTEXT, 0, (LPARAM)strOutToUi.c_str());
                    }
                    else
                    {
                        strOutToUi += " LOADING... ";
                        SendMessageA(self->txtOut, WM_SETTEXT, 0, (LPARAM)strOutToUi.c_str());
                    }
                }
            }
        }
        stdBuf[0] = NULL;
        bytesRead = bytesAvailable = bytesLeft = 0;
        if (bReadStream && PeekNamedPipe(stderr_read, stdBuf, STD_BUFFSIZE, &bytesRead, &bytesAvailable, &bytesLeft))
        {
            if (bytesAvailable > 0)
            {
                bSuccess = ReadFile(stderr_read, stdBuf, STD_BUFFSIZE, &bytesRead, NULL);
                if (bSuccess)
                {
                    stdBuf[bytesRead] = 0;
                    SendMessageA(self->txtOut, WM_SETTEXT, 0, (LPARAM)stdBuf);
                }
            }
        }
    }

    CloseHandle(stdout_read);
    CloseHandle(stderr_read);
    CloseHandle(procInfo.hThread);
    CloseHandle(procInfo.hProcess);

    std::string fileRoot = self->set->strOutRoot + "\\" + self->strTitle + "\\";
    strOutToUi += self->RenameFile(fileRoot);

    if (self->omdb)
    {
        std::string json = self->omdb->GetOmdbInfo(self->strTitle);
        if (json == "")
        {
            strOutToUi += " Unable to get Information from OMDB\n";
        }
        else
        {
            std::string filePath = fileRoot + "info.json";
            FILE *f = NULL;
            if (fopen_s(&f, filePath.c_str(), "w") == 0)
            {
                fwrite(json.c_str(), 1, json.size(), f);
                fclose(f);
            }
            else {
                strOutToUi += " Failed open file for writing: " + filePath + "\n";
            }
            std::string posterUrl = FindPosterUrl(json);
            if (posterUrl != "")
            {
                size_t pos = posterUrl.rfind('.');
                filePath = fileRoot + "poster" + posterUrl.substr(pos);
                json = self->omdb->DownloadFile(posterUrl, filePath);
                if (json == "")
                {
                    strOutToUi += " Unable to download poster image\n";
                }
            }
        }
    }
    
    self->run = false;
    CloseHandle(self->hThread);
    self->threadId = -1;
    self->hThread = NULL;
    if (self->set->eject)
    {
        if (!ejectDisk(self->driveLetter[0]))
        {
            strOutToUi += " \nUnable to eject disc: ";
            strOutToUi += GetLastErrorAsString();
        }
    }
    strOutToUi += self->driveLetter + " - ";
    strOutToUi += self->strTitle;
    strOutToUi += " is DONE!";
    SendMessageA(self->txtOut, WM_SETTEXT, 0, (LPARAM)strOutToUi.c_str());
	return 0;
}
