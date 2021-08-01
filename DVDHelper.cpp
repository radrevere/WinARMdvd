// DVDHelper.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "DVDHelper.h"
#include "dbt.h"
#include "DVDHandler.h"
#include "Worker.h"
#include "SettingsDialog.h"
//#include <WinUser.h>
#include <windef.h>
#include <fileapi.h>


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

DVDHandler dvdHandler;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    //ejectDisk('I');
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DVDHELPER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DVDHELPER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DVDHELPER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DVDHELPER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   Settings* set = dvdHandler.GetSettings();
   HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, set->yPos, set->wndWide, set->wndHigh, nullptr, nullptr, hInstance, nullptr);
   //CW_USEDEFAULT
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd); 
   //MoveWindow(hWnd, set->xPos, set->yPos, set->wndWide, set->wndHigh, false);
   //SetWindowPos(hWnd, NULL, set->xPos, set->yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
   //RECT rect;
   //GetWindowRect(hWnd, &rect);
   dvdHandler.SetupOutput(hWnd);
   return TRUE;
}


// gets drive letter based on the mask sent by the device change event
char FirstDriveFromMask(ULONG unitmask)
{
    char i;

    for (i = 0; i < 26; ++i)
    {
        if (unitmask & 0x1)
            break;
        unitmask = unitmask >> 1;
    }

    return(i + 'A');
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    
    PDEV_BROADCAST_HDR lpdb = NULL;
    switch (message)
    {
    case WM_CREATE:
        
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case ID_FILE_OPTIONS:
            {
                SettingsDialog setDlg;
                setDlg.Show(hWnd, dvdHandler.GetSettings());
            }
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DEVICECHANGE:
        // sent when cd/dvd gets inserted
        lpdb = (PDEV_BROADCAST_HDR)lParam;
        switch (wParam)
        {
        case DBT_DEVICEARRIVAL:
            if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                lpdb->dbch_size; // size of volume
                if (lpdbv->dbcv_flags & DBTF_MEDIA)
                {
                    //I have a valid drive
                    std::string drive = "C:\\";
                    char drv = FirstDriveFromMask(lpdbv->dbcv_unitmask);
                    drive[0] = drv;
                    // make sure we have a valid dvd disk
                    dvdHandler.DiskLoaded(drv);
                }
            }
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            // cd/dvd ejected
            if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                lpdb->dbch_size; // size of volume
                if (lpdbv->dbcv_flags & DBTF_MEDIA)
                {
                    //I have a valid drive
                    char drv = FirstDriveFromMask(lpdbv->dbcv_unitmask);
                    dvdHandler.DiskEjected(drv);
                }
            }
            break;
        case DBT_DEVNODES_CHANGED:
            break;
        default:
            break;
        }
        break;

    //case WM_MOVE:
    //    dvdHandler.SavePos(LOWORD(lParam), HIWORD(lParam));
    //    break;
    case WM_SIZE:
        break;
    case WM_EXITSIZEMOVE:
        RECT rect;
        if(GetWindowRect(hWnd, &rect))
        {
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;
            dvdHandler.WindowChanged(rect.left, rect.right, width, height);
        }
        // repaint window 
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
