#include "shared/FirstTimeSetup.h"
#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <string>
#include <fstream>
#include <shlobj.h>
#include "shared/Profile.h"
#include "shared/State.h"

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

int g_setupState = 1;

std::wstring g_setupDPI = L"";
std::wstring g_setupSens = L"";
std::wstring g_setupFOV = L"";
std::wstring g_setupRes = L"";
std::wstring g_setupScale = L"";

bool g_sensManual = false;
bool g_fovManual = false;
bool g_resManual = false;
bool g_scaleManual = false;

int g_ptrSpeed = 10;
int g_ptrEnhance = 0;

void FinishSetup() {
    Profile p;
    p.name = L"Fallback_Default";
    p.roi_h = 60; p.roi_w = 400; p.roi_x = 760; p.roi_y = 650;
    p.target_color = RGB(150, 150, 150);
    p.tolerance = 25;

    double dpiVal = 800.0;
    try { dpiVal = std::stod(g_setupDPI); } catch(...) {}
    if (dpiVal <= 0) dpiVal = 800.0;

    double sensVal = 0.05;
    try { sensVal = std::stod(g_setupSens); } catch(...) {}
    if (sensVal <= 0) sensVal = 0.05;

    double normalScale = 0.5573 / (dpiVal * sensVal);
    p.scale_normal = normalScale;
    p.scale_diving = normalScale;
    p.scale_gliding = normalScale;

    p.fov = std::string(g_setupFOV.begin(), g_setupFOV.end());
    p.resolution = std::string(g_setupRes.begin(), g_setupRes.end());
    p.render_scale = std::string(g_setupScale.begin(), g_setupScale.end());

    extern std::vector<Profile> g_allProfiles;
    extern int g_selectedProfileIdx;
    
    if (g_allProfiles.empty()) {
        p.name = L"Fallback_Default";
        p.Save(GetAppStoragePath() + L"Fallback_Default.json");
        g_allProfiles.push_back(p);
    } else {
        p.name = g_allProfiles[g_selectedProfileIdx].name;
        // Keep existing ROI parameters
        p.roi_h = g_allProfiles[g_selectedProfileIdx].roi_h;
        p.roi_w = g_allProfiles[g_selectedProfileIdx].roi_w;
        p.roi_x = g_allProfiles[g_selectedProfileIdx].roi_x;
        p.roi_y = g_allProfiles[g_selectedProfileIdx].roi_y;
        p.target_color = g_allProfiles[g_selectedProfileIdx].target_color;
        p.tolerance = g_allProfiles[g_selectedProfileIdx].tolerance;
        g_allProfiles[g_selectedProfileIdx] = p;
        p.Save(GetAppStoragePath() + p.name + L".json");
    }
}

void DrawSetupButton(Graphics& g, Font* f, const wchar_t* text, int x, int y, int w, int h, Color fill) {
    SolidBrush brush(fill);
    g.FillRectangle(&brush, x, y, w, h);
    SolidBrush white(Color(255,255,255));
    g.DrawString(text, -1, f, PointF(x + 10, y + 10), &white);
}

LRESULT CALLBACK FirstTimeSetupProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_CREATE) {
        return 0;
    }
    if (message == WM_CHAR) {
        if (wParam == VK_BACK) {
            std::wstring* active = nullptr;
            if (g_setupState == 1) active = &g_setupDPI;
            else if (g_setupState == 2 && g_sensManual) active = &g_setupSens;
            else if (g_setupState == 3 && g_fovManual) active = &g_setupFOV;
            else if (g_setupState == 4 && g_resManual) active = &g_setupRes;
            else if (g_setupState == 5 && g_scaleManual) active = &g_setupScale;
            if (active && !active->empty()) active->pop_back();
        } else if (wParam >= 32 && wParam <= 126) {
            std::wstring* active = nullptr;
            if (g_setupState == 1) active = &g_setupDPI;
            else if (g_setupState == 2 && g_sensManual) active = &g_setupSens;
            else if (g_setupState == 3 && g_fovManual) active = &g_setupFOV;
            else if (g_setupState == 4 && g_resManual) active = &g_setupRes;
            else if (g_setupState == 5 && g_scaleManual) active = &g_setupScale;
            if (active) active->push_back((wchar_t)wParam);
        }
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }
    if (message == WM_LBUTTONDOWN) {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        
        auto nextState = [&]() {
            g_setupState++;
            if (g_setupState == 6 && g_ptrSpeed == 10) g_setupState++;
            if (g_setupState == 7 && g_ptrEnhance == 0) g_setupState++;
            if (g_setupState > 7) {
                FinishSetup();
                DestroyWindow(hWnd);
            }
            InvalidateRect(hWnd, NULL, FALSE);
        };
        
        if (g_setupState == 1) {
            if (y > 150 && y < 190 && x > 20 && x < 200) nextState();
        } else if (g_setupState >= 2 && g_setupState <= 5) {
            bool isManual = false;
            if (g_setupState == 2) isManual = g_sensManual;
            else if (g_setupState == 3) isManual = g_fovManual;
            else if (g_setupState == 4) isManual = g_resManual;
            else if (g_setupState == 5) isManual = g_scaleManual;
            
            if (!isManual) {
                if (y > 150 && y < 190) {
                    if (x > 20 && x < 120) nextState(); // YES
                    else if (x > 140 && x < 240) { // NO
                        if (g_setupState == 2) g_sensManual = true;
                        else if (g_setupState == 3) g_fovManual = true;
                        else if (g_setupState == 4) g_resManual = true;
                        else if (g_setupState == 5) g_scaleManual = true;
                        InvalidateRect(hWnd, NULL, FALSE);
                    }
                }
            } else {
                if (y > 220 && y < 260 && x > 20 && x < 200) nextState();
            }
        } else if (g_setupState == 6 || g_setupState == 7) {
            if (y > 220 && y < 260) {
                if (x > 20 && x < 250) {
                    ShellExecuteW(NULL, L"open", L"control", L"main.cpl", NULL, SW_SHOWNORMAL);
                } else if (x > 270 && x < 450) {
                    nextState();
                }
            }
        }
        return 0;
    }
    if (message == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        Graphics g(hdc);
        g.Clear(Color(255, 20, 24, 30));

        FontFamily ff(L"Segoe UI");
        Font title(&ff, 20, FontStyleBold, UnitPixel);
        Font text(&ff, 15, FontStyleRegular, UnitPixel);
        SolidBrush white(Color(255, 255, 255));
        SolidBrush gray(Color(180, 180, 180));
        SolidBrush inputBg(Color(40, 45, 50));

        g.DrawString(L"First Time Integration Setup", -1, &title, PointF(20, 20), &white);

        if (g_setupState == 1) {
            g.DrawString(L"Step 1: Mouse DPI", -1, &text, PointF(20, 60), &gray);
            g.DrawString(L"Enter your mouse DPI (check your mouse software):", -1, &text, PointF(20, 85), &white);
            g.FillRectangle(&inputBg, 20, 110, 300, 30);
            std::wstring disp = g_setupDPI + L"_";
            g.DrawString(disp.c_str(), -1, &text, PointF(25, 115), &white);
            DrawSetupButton(g, &text, L"Confirm", 20, 150, 180, 40, Color(0, 120, 215));
        } else if (g_setupState >= 2 && g_setupState <= 5) {
            std::wstring label, activeVal;
            bool manual = false;
            if (g_setupState == 2) { label = L"Step 2: Fortnite Parameter"; activeVal = g_setupSens; manual = g_sensManual; }
            if (g_setupState == 3) { label = L"Step 3: Fortnite FOV"; activeVal = g_setupFOV; manual = g_fovManual; }
            if (g_setupState == 4) { label = L"Step 4: Monitor Resolution"; activeVal = g_setupRes; manual = g_resManual; }
            if (g_setupState == 5) { label = L"Step 5: Render Scale"; activeVal = g_setupScale; manual = g_scaleManual; }
            
            g.DrawString(label.c_str(), -1, &text, PointF(20, 60), &gray);
            
            if (!manual) {
                std::wstring q = L"We detected: " + activeVal + L" \nIs this correct?";
                g.DrawString(q.c_str(), -1, &text, PointF(20, 90), &white);
                DrawSetupButton(g, &text, L"YES", 20, 150, 100, 40, Color(0, 160, 50));
                DrawSetupButton(g, &text, L"NO", 140, 150, 100, 40, Color(200, 50, 50));
            } else {
                g.DrawString(L"Please enter the value manually:", -1, &text, PointF(20, 90), &white);
                g.FillRectangle(&inputBg, 20, 140, 300, 30);
                std::wstring disp = activeVal + L"_";
                g.DrawString(disp.c_str(), -1, &text, PointF(25, 145), &white);
                DrawSetupButton(g, &text, L"Confirm Manual", 20, 220, 180, 40, Color(0, 120, 215));
            }
        } else if (g_setupState == 6 || g_setupState == 7) {
            std::wstring label = (g_setupState == 6) ? L"Step 6: Windows Pointer Speed" : L"Step 7: Pointer Precision";
            g.DrawString(label.c_str(), -1, &text, PointF(20, 60), &gray);
            
            std::wstring warn;
            if (g_setupState == 6) warn = L"Your Windows pointer speed is not at the default (6/11).\nThis strongly affects accuracy.\nWe recommend setting it to the 6th notch.";
            else warn = L"Enhance Pointer Precision is currently enabled!\nThis literally breaks angle tracking math.\nYou must turn it off in Windows settings.";
            
            g.DrawString(warn.c_str(), -1, &text, PointF(20, 90), &white);
            DrawSetupButton(g, &text, L"Open Windows Mouse Settings", 20, 220, 230, 40, Color(200, 100, 20));
            DrawSetupButton(g, &text, L"Continue Anyway", 270, 220, 180, 40, Color(80, 80, 80));
        }

        EndPaint(hWnd, &ps);
        return 0;
    }
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void StartModalSetupLoop(HWND hwnd) {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (!IsWindow(hwnd)) break;
    }
}

void ShowFirstTimeSetup(HINSTANCE hInstance) {
    g_setupState = 1;
    g_setupDPI = L""; g_setupSens = L""; g_setupFOV = L""; g_setupRes = L""; g_setupScale = L"";
    g_sensManual = false; g_fovManual = false; g_resManual = false; g_scaleManual = false;

    // Parse Phase
    std::string iniContent = "";
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
        std::wstring p = std::wstring(path) + L"\\FortniteGame\\Saved\\Config\\WindowsClient\\GameUserSettings.ini";
        std::ifstream ifs(p);
        if (ifs.good()) {
            iniContent.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        }
    }
    auto extractValue = [&](const std::string& key) -> std::wstring {
        size_t pos = iniContent.find(key + "=");
        if (pos != std::string::npos) {
            size_t end = iniContent.find_first_of("\r\n", pos);
            std::string val = iniContent.substr(pos + key.length() + 1, end - (pos + key.length() + 1));
            return std::wstring(val.begin(), val.end());
        }
        return L"";
    };

    g_setupSens = extractValue("MouseX");
    g_setupFOV = extractValue("FOV");
    if (g_setupFOV.empty()) {
        std::wstring w = extractValue("DesiredScreenWidth");
        std::wstring h = extractValue("DesiredScreenHeight");
        if (!w.empty() && !h.empty()) g_setupFOV = L"Aspect (" + w + L"x" + h + L")";
    }
    if (g_setupSens.empty()) g_setupSens = L"0.05";
    if (g_setupFOV.empty()) g_setupFOV = L"Unknown";

    g_setupRes = std::to_wstring(GetSystemMetrics(SM_CXSCREEN)) + L"x" + std::to_wstring(GetSystemMetrics(SM_CYSCREEN));
    g_setupScale = extractValue("ResolutionQuality");
    if (g_setupScale.empty()) g_setupScale = L"100.0";

    SystemParametersInfo(SPI_GETMOUSESPEED, 0, &g_ptrSpeed, 0);
    int mouseParams[3];
    SystemParametersInfo(SPI_GETMOUSE, 0, mouseParams, 0);
    g_ptrEnhance = mouseParams[2];

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = FirstTimeSetupProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"FTSWindowClass";
    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(
        WS_EX_TOPMOST, L"FTSWindowClass", L"BetterAngle Setup", WS_POPUP,
        GetSystemMetrics(SM_CXSCREEN) / 2 - 250, GetSystemMetrics(SM_CYSCREEN) / 2 - 175,
        500, 350, NULL, NULL, hInstance, NULL
    );

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    StartModalSetupLoop(hWnd);
}
