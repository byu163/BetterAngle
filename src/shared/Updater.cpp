#include "shared/Updater.h"
#include "shared/State.h"
#include <windows.h>
#include <wininet.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>

#pragma comment(lib, "wininet.lib")

// Placeholders for actual URLs — user should replace these in GitHub Actions or similar if needed
const wchar_t* VERSION_URL = L"https://raw.githubusercontent.com/MahanYTT/BetterAngle/main/version.txt";
const wchar_t* DOWNLOAD_URL = L"https://github.com/MahanYTT/BetterAngle/releases/latest/download/BetterAngle.exe";

bool DownloadFile(const std::wstring& url, const std::wstring& dest) {
    HINTERNET hInternet = InternetOpenW(L"BetterAngleUpdater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hUrl = InternetOpenUrlW(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        InternetCloseHandle(hInternet);
        return false;
    }

    std::ofstream ofs(dest, std::ios::binary);
    if (!ofs.is_open()) {
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return false;
    }

    char buffer[4096];
    DWORD bytesRead;
    while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        ofs.write(buffer, bytesRead);
    }

    ofs.close();
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    return true;
}

bool CheckForUpdates() {
    g_isCheckingForUpdates = true;
    
    std::wstring tempVer = GetAppRootPath() + L"latest_version.txt";
    if (DownloadFile(VERSION_URL, tempVer)) {
        std::ifstream ifs(tempVer);
        std::string latestVerStr;
        if (std::getline(ifs, latestVerStr)) {
            // Very simple version comparison (v1.2.3 vs v1.2.4)
            // Trim whitespace
            latestVerStr.erase(0, latestVerStr.find_first_not_of(" \t\r\n"));
            latestVerStr.erase(latestVerStr.find_last_not_of(" \t\r\n") + 1);

            g_latestVersionOnline = latestVerStr;
            
            if (!latestVerStr.empty() && latestVerStr != VERSION_STR) {
                g_updateAvailable = true;
                g_updateHistory = std::string(VERSION_STR) + " -> " + latestVerStr;
            } else {
                g_updateAvailable = false;
            }
        }
    }

    g_isCheckingForUpdates = false;
    g_hasCheckedForUpdates = true;
    return g_updateAvailable;
}

void UpdateApp() {
    if (g_isDownloadingUpdate || g_downloadComplete) return;

    g_isDownloadingUpdate = true;
    std::thread([]() {
        std::wstring dest = GetAppRootPath() + L"update_tmp.exe";
        if (DownloadFile(DOWNLOAD_URL, dest)) {
            g_downloadComplete = true;
        }
        g_isDownloadingUpdate = false;
    }).detach();
}

void CleanupUpdateJunk() {
    std::wstring root = GetAppRootPath();
    DeleteFileW((root + L"ba_update.bat").c_str());
    DeleteFileW((root + L"update_tmp.exe").c_str());
    DeleteFileW((root + L"latest_version.txt").c_str());
}

void ApplyUpdateAndRestart() {
    std::wstring root = GetAppRootPath();
    std::wstring exePath = root + L"update_tmp.exe";
    
    wchar_t currentExe[MAX_PATH];
    GetModuleFileNameW(NULL, currentExe, MAX_PATH);

    // Create a batch file to swap EXEs and restart
    std::wstring batPath = root + L"ba_update.bat";
    std::wofstream ofs(batPath);
    ofs << L"@echo off\n";
    ofs << L"timeout /t 2 /nobreak > nul\n";
    ofs << L"move /y \"" << exePath << L"\" \"" << currentExe << L"\"\n";
    ofs << L"start \"\" \"" << currentExe << L"\"\n";
    ofs << L"del \"%~f0\"\n";
    ofs.close();

    ShellExecuteW(NULL, L"open", batPath.c_str(), NULL, NULL, SW_HIDE);
    exit(0);
}
