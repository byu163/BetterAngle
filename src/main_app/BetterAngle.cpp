#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <dwmapi.h>
#include <gdiplus.h>

#include "shared/Input.h"
#include "shared/Overlay.h"
#include "shared/Logic.h"
#include "shared/Detector.h"
#include "shared/Updater.h"
#include "shared/Profile.h"
#include "shared/Tray.h"
#include "shared/Remote.h"

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// Global State
HWND g_hWnd = NULL;
ULONG_PTR g_gdiplusToken;
std::atomic<bool> g_running(true);
AngleLogic g_logic(800, 6.5);
FovDetector g_detector;
std::string g_status = "Cloud Syncing...";

// Modern UI State
Profile g_currentProfile;
std::vector<Profile> g_allProfiles;
int g_selectedProfileIdx = 0;
bool g_isCursorVisible = false;
bool g_isSettingsMode = true; // Pro 4.5: Interactive by default

// Cloud Logic
void CloudProfileFetch() {
    std::string urlStr = "https://raw.githubusercontent.com/MahanYTT/BetterAngle/main/remote_profiles/Fortnite_Standard.json";
    std::wstring wUrl(urlStr.begin(), urlStr.end());
    std::string profilesJson = FetchRemoteString(wUrl);
    
    if (!profilesJson.empty()) {
        std::ofstream f("profiles/cloud_fn.json", std::ios::trunc);
        f << profilesJson;
        f.close();
        
        Profile p;
        if (p.Load(L"profiles/cloud_fn.json")) {
            g_allProfiles.clear();
            g_allProfiles.push_back(p);
            g_currentProfile = p;
            g_status = "Connected (v4.5 Pro)";
        }
    } else {
        g_status = "Offline Mode (Using Local)";
        g_allProfiles = GetProfiles(L"profiles");
        if (!g_allProfiles.empty()) {
             g_currentProfile = g_allProfiles[0];
        }
    }
}

// Toggle Interactivity
void ToggleHUDInteractivity(HWND hwnd, bool interactive) {
    long exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (interactive) {
        exStyle &= ~WS_EX_TRANSPARENT; // Can be clicked
    } else {
        exStyle |= WS_EX_TRANSPARENT; // Click-through
    }
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

// FOV Detector Thread
void DetectorThread() {
    while (g_running) {
        if (!g_allProfiles.empty()) {
            Profile& p = g_allProfiles[g_selectedProfileIdx];
            RoiConfig cfg = { p.roi_x, p.roi_y, p.roi_w, p.roi_h, p.target_color, p.tolerance };
            
            float ratio = g_detector.Scan(cfg);
            if (ratio > 0.05f) {
                g_logic.SetScale(p.scale_diving);
            } else {
                g_logic.SetScale(p.scale_normal);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
            RegisterRawMouse(hWnd);
            AddSystrayIcon(hWnd);
            RegisterHotKey(hWnd, 1, MOD_CONTROL, 'U'); // CTRL+U to toggle
            
            if (CheckForUpdates()) {
                g_status = "New Update Available!";
                StartUpdate(); 
            }
            return 0;

        case WM_HOTKEY:
            if (wParam == 1) {
                g_isSettingsMode = !g_isSettingsMode;
                ToggleHUDInteractivity(hWnd, g_isSettingsMode);
            }
            return 0;

        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP) {
                if (lParam == WM_RBUTTONUP) ShowTrayContextMenu(hWnd);
                else {
                    g_isSettingsMode = true;
                    ToggleHUDInteractivity(hWnd, true);
                    SetForegroundWindow(hWnd);
                }
            }
            return 0;

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_TRAY_EXIT) {
                SendMessage(hWnd, WM_CLOSE, 0, 0);
            }
            return 0;

        case WM_INPUT: {
            if (g_isCursorVisible || g_isSettingsMode) return 0;
            int dx = GetRawInputDeltaX(lParam);
            g_logic.Update(dx);
            return 0;
        }

        case WM_LBUTTONDOWN:
            if (g_isSettingsMode) PostMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            return 0;

        case WM_TIMER: {
            CURSORINFO ci = { sizeof(CURSORINFO) };
            if (GetCursorInfo(&ci)) {
                g_isCursorVisible = (ci.flags & CURSOR_SHOWING);
            }
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;
        }

        case WM_ACTIVATE:
            if (LOWORD(wParam) != WA_INACTIVE) {
                g_isSettingsMode = true;
                ToggleHUDInteractivity(hWnd, true);
            }
            return 0;

        case WM_PAINT:
            DrawOverlay(hWnd, g_logic.GetAngle(), g_status.c_str());
            return 0;

        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;

        case WM_DESTROY:
            RemoveSystrayIcon(hWnd);
            g_running = false;
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"BetterAngleV3";
    RegisterClass(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    // v4.5: WS_EX_APPWINDOW for Taskbar visibility, removed WS_EX_TOOLWINDOW
    g_hWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_APPWINDOW,
        L"BetterAngleV3", L"BetterAngle Pro",
        WS_POPUP,
        0, 0, screenW, screenH,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    SetTimer(g_hWnd, 1, 25, NULL);

    std::thread detThread(DetectorThread);
    std::thread cloudThread(CloudProfileFetch);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    g_running = false;
    if (detThread.joinable()) detThread.join();
    if (cloudThread.joinable()) cloudThread.join();
    GdiplusShutdown(g_gdiplusToken);
    return (int)msg.wParam;
}
