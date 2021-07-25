#include "OMDbInfo.h"
#include <Windows.h>
#include <tchar.h>
#include <winhttp.h>

OMDbInfo::OMDbInfo(std::string key)
{
	apiKey = key;
}
std::string OMDbInfo::GetOmdbInfo(std::string title)
{
    char url[256] = { 0 };
    sprintf_s(url, BASE_URL_FMT, apiKey.c_str(), title.c_str());
    title = url;
    std::wstring strUrl(title.begin(), title.end());
    return GetWebInfo(L"www.omdbapi.com",strUrl, "");
}

std::string OMDbInfo::DownloadFile(std::string url, std::string fileName)
{
    std::wstring str(url.begin(), url.end());
    size_t pos = str.find(L"/", 0) + 2;
    size_t pos2 = str.find(L"/", pos);

    std::wstring server = str.substr(pos, pos2-pos);
    std::wstring qry = str.substr(pos2 + 1);
    return GetWebInfo(server, qry, fileName);
}

// poster.jpg for covers and posters
std::string OMDbInfo::GetWebInfo(std::wstring server, std::wstring obj, std::string outFile)
{
	std::string out;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer;
    BOOL  bResults = FALSE;
    HINTERNET  hSession = NULL,
        hConnect = NULL,
        hRequest = NULL;

    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen(L"WinHTTP RWBService/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    // Specify an HTTP server.
    if (hSession)
        hConnect = WinHttpConnect(hSession, server.c_str(),
            INTERNET_DEFAULT_HTTPS_PORT, 0);

    // Create an HTTP request handle.
    if (hConnect)
        hRequest = WinHttpOpenRequest(hConnect, L"GET", obj.c_str(),
            NULL, WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE);

    // Send a request.
    if (hRequest)
        bResults = WinHttpSendRequest(hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0,
            0, 0);


    // End the request.
    if (bResults)
        bResults = WinHttpReceiveResponse(hRequest, NULL);

    // Keep checking for data until there is nothing left.
    if (bResults)
    {
        FILE* f = NULL;
        if (outFile != "")
        {
            if (fopen_s(&f, outFile.c_str(), "wb") != 0)
            {
                f = NULL;
            }
        }
        do
        {
            // Check for available data.
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                printf("Error %u in WinHttpQueryDataAvailable.\n",
                    GetLastError());

            // Allocate space for the buffer.
            pszOutBuffer = new char[dwSize + 1];
            if (!pszOutBuffer)
            {
                printf("Out of memory\n");
                dwSize = 0;
            }
            else
            {
                // Read the data.
                ZeroMemory(pszOutBuffer, dwSize + 1);

                if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
                    dwSize, &dwDownloaded))
                {
                    printf("Error %u in WinHttpReadData.\n", GetLastError());
                }
                else
                {
                    //printf("%s", pszOutBuffer);
                    if (f)
                    {
                        fwrite(pszOutBuffer, 1, dwSize, f);
                        out += "1";
                    }
                    else
                    {
                        out += pszOutBuffer;
                    }
                }

                // Free the memory allocated to the buffer.
                
                delete[] pszOutBuffer;
            }
        } while (dwSize > 0);
        if (f)
        {
            fclose(f);
        }
    }


    // Report any errors.
    if (!bResults)
    {
        //char buf[512] = { 0 };
        //sprintf_s(buf, "Error %d has occurred.\n", GetLastError());
        //MessageBoxA(NULL, buf, "Web Error", MB_OK);
        
    }
    // Close any open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

	return out;
}