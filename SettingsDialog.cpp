#include "SettingsDialog.h"
#include "resource.h"
#include <ShlObj_core.h>

int arCache[6] = { 0,64,256,512,768,1024 };

static SettingsDialog* self;
HWND parent = NULL;
void ShowFolderDialog(bool makeMkvPath)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
        COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileOpen;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
        if (SUCCEEDED(hr))
        {
            pFileOpen->SetOptions(FOS_PICKFOLDERS);
            // Show the Open dialog box.
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Display the file name to the user.
                    if (SUCCEEDED(hr))
                    {
                        if (makeMkvPath)
                        {
                            SetDlgItemText(parent, IDC_EDT_MKV_PATH, pszFilePath);
                            
                        }
                        else
                        {
                            SetDlgItemText(parent, IDC_EDT_OUT_ROOT, pszFilePath);
                        }
                        //MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
}


INT_PTR CALLBACK DialogMessageProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    if (self == NULL)
    {
        return FALSE;
    }
    switch (Message)
    {
    case WM_INITDIALOG:
        self->InitControls(hwnd);
        parent = hwnd;
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_MKV_PATH:
            ShowFolderDialog(true);
            break;
        case IDC_BTN_OUT_PATH:
            ShowFolderDialog(false);
            break;
        case IDOK:
            self->SaveSettings(parent);
            EndDialog(hwnd, IDOK);
            break;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

SettingsDialog::SettingsDialog()
{
    set = NULL;
	self = this;
}

void SettingsDialog::Show(HWND parent, Settings* settings)
{
    set = settings;
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DLG_SETTINGS), parent, DialogMessageProc);
}

void SettingsDialog::SaveSettings(HWND hwnd)
{
    if (set == NULL)
    {
        return;
    }
    // gather data from controls
    char buf[MAX_PATH] = { 0 };
    GetDlgItemTextA(hwnd, IDC_EDT_MKV_PATH, buf, MAX_PATH);
    set->strMkvExe = buf;
    if (set->strMkvExe.size() > 0 && set->strMkvExe.at(set->strMkvExe.size() - 1) != '\\')
    {
        set->strMkvExe += "\\";
    }
    GetDlgItemTextA(hwnd, IDC_EDT_OUT_ROOT, buf, MAX_PATH);
    set->strOutRoot = buf;
    if (set->strOutRoot.size() > 0 && set->strOutRoot.at(set->strOutRoot.size() - 1) != '\\')
    {
        set->strOutRoot += "\\";
    }
    GetDlgItemTextA(hwnd, IDC_EDT_OMDB, buf, MAX_PATH);
    set->strOMDbkey = buf;
    GetDlgItemTextA(hwnd, IDC_EDT_MIN_LENGTH, buf, MAX_PATH);
    set->minLength = atoi(buf);
    HWND ctrl = GetDlgItem(hwnd, IDC_CBO_CACHE);
    int selection = 0;
    sprintf_s(buf, "%s", "Auto");
    int idx = (int)SendMessageA(ctrl, CB_GETCURSEL, 0, 0);
    if (idx > 0 && idx < 6)
    {
        set->cache = arCache[idx];
    }

    ctrl = GetDlgItem(hwnd, IDC_CHK_64BIT);
    set->use64bit = false;
    if (SendMessageA(ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        set->use64bit = true;
    }
    ctrl = GetDlgItem(hwnd, IDC_CHK_ROBOT);
    set->robot = false;
    if (SendMessageA(ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        set->robot = true;
    }
    ctrl = GetDlgItem(hwnd, IDC_CHK_DECRYPT);
    set->decrypt = false;
    if (SendMessageA(ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        set->decrypt = true;
    }
    ctrl = GetDlgItem(hwnd, IDC_CHK_DIRECTIO);
    set->directio = false;
    if (SendMessageA(ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        set->directio = true;
    }
    ctrl = GetDlgItem(hwnd, IDC_CHK_EJECT);
    set->eject = false;
    if (SendMessageA(ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        set->eject = true;
    }
    ctrl = GetDlgItem(hwnd, IDC_CHK_SERIES);
    set->series = false;
    if (SendMessageA(ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        set->series = true;
    }

    // save
    set->SaveSettings();
}

void SettingsDialog::InitControls(HWND hwnd)
{
    if (set == NULL)
    {
        return;
    }
    // move from settings to controls
    char buf[MAX_PATH] = { 0 };
    SetDlgItemTextA(hwnd, IDC_EDT_MKV_PATH, set->strMkvExe.c_str());
    SetDlgItemTextA(hwnd, IDC_EDT_OUT_ROOT, set->strOutRoot.c_str());
    SetDlgItemTextA(hwnd, IDC_EDT_OMDB, set->strOMDbkey.c_str());
    _itoa_s(set->minLength, buf, 10);
    SetDlgItemTextA(hwnd, IDC_EDT_MIN_LENGTH, buf);
    HWND ctrl = GetDlgItem(hwnd, IDC_CBO_CACHE);
    int selection = 0;
    sprintf_s(buf, "%s", "Auto");
    SendMessageA(ctrl, CB_ADDSTRING, 0, (LPARAM)buf);
    for (int i = 1; i < 6; i++)
    {
        sprintf_s(buf, "%d", arCache[i]);
        SendMessageA(ctrl, CB_ADDSTRING, 0, (LPARAM)buf);
        if (arCache[i] == set->cache)
        {
            selection = i;
        }
    }
    SendMessageA(ctrl, CB_SETCURSEL, selection, 0);

    ctrl = GetDlgItem(hwnd, IDC_CHK_64BIT);
    SendMessageA(ctrl, BM_SETCHECK, set->use64bit, 0);
    ctrl = GetDlgItem(hwnd, IDC_CHK_ROBOT);
    SendMessageA(ctrl, BM_SETCHECK, set->robot, 0);
    ctrl = GetDlgItem(hwnd, IDC_CHK_DECRYPT);
    SendMessageA(ctrl, BM_SETCHECK, set->decrypt, 0);
    ctrl = GetDlgItem(hwnd, IDC_CHK_DIRECTIO);
    SendMessageA(ctrl, BM_SETCHECK, set->directio, 0);
    ctrl = GetDlgItem(hwnd, IDC_CHK_EJECT);
    SendMessageA(ctrl, BM_SETCHECK, set->eject, 0);
    ctrl = GetDlgItem(hwnd, IDC_CHK_SERIES);
    SendMessageA(ctrl, BM_SETCHECK, set->series, 0);
}