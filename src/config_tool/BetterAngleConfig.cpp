#include <windows.h>
#include <iostream>
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
#include "shared/Profile.h"

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// State
int g_step = 1; 
long long g_dxA = 0, g_dxB = 0;
Profile g_result;
std::wstring g_wizardMsg = L"STEP 1: Point your camera exactly at 0 degrees and press 'ENTER'.";

// Window Procedure
LRESULT CALLBACK ConfigWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
            RegisterRawMouse(hWnd);
            return 0;
            
        case WM_KEYDOWN:
            if (wParam == VK_RETURN) {
                if (g_step == 1) {
                    // Capture Zero
                    g_step = 2;
                    g_wizardMsg = L"STEP 2: Turn exactly 120 degrees and press 'ENTER'.";
                } else if (g_step == 2) {
                    // Calculate Scale
                    g_step = 3;
                    double diff = (double)g_dxB - (double)g_dxA;
                    g_result.scale_normal = 120.0 / diff;
                    g_result.scale_diving = g_result.scale_normal * 1.5; // Default estimate
                    g_result.name = L"New Profile";
                    g_wizardMsg = L"STEP 3: Calibration Complete! Press 'S' to Save.";
                }
            } else if (wParam == 'S' && g_step == 3) {
                g_result.Save(L"profiles/new_profile.json");
                g_wizardMsg = L"DONE! Profile saved to 'profiles/new_profile.json'.";
            }
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;

        case WM_INPUT: {
            int dx = GetRawInputDeltaX(lParam);
            if (g_step == 1) g_dxA += dx;
            if (g_step == 2) g_dxB += dx;
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            Graphics graphics(hdc);
            graphics.Clear(Color(200, 30, 34, 40));

            FontFamily ff(L"Segoe UI");
            Font f(&ff, 24, FontStyleBold, UnitPixel);
            SolidBrush b(Color(255, 255, 255, 255));
            graphics.DrawString(L"BETTERANGLE CONFIG TOOL (V3.5 PRO)", -1, &f, PointF(50, 50), &b);
            
            Font f2(&ff, 18, FontStyleRegular, UnitPixel);
            graphics.DrawString(g_wizardMsg.c_str(), -1, &f2, PointF(50, 100), &b);

            std::wstring statusLine = L"Current Acc: " + std::to_wstring(g_dxA + g_dxB);
            graphics.DrawString(statusLine.c_str(), -1, &f2, PointF(50, 200), &b);

            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ULONG_PTR token;
    GdiplusStartupInput input;
    GdiplusStartup(&token, &input, NULL);

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = ConfigWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"BetterAngleConfig";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        L"BetterAngleConfig", L"BetterAngle Calibration Wizard",
        WS_POPUP,
        100, 100, 800, 400,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(token);
    return (int)msg.wParam;
}
