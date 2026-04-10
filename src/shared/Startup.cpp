#include "shared/Startup.h"
#include <gdiplus.h>
#include <thread>
#include <chrono>

using namespace Gdiplus;

LRESULT CALLBACK SplashWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        Graphics graphics(hdc);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
        
        // Background
        SolidBrush bg(Color(255, 15, 18, 22));
        graphics.Clear(Color(255, 15, 18, 22));

        FontFamily fontFamily(L"Segoe UI");
        Font font(&fontFamily, 24, FontStyleBold, UnitPixel);
        Font subFont(&fontFamily, 14, FontStyleRegular, UnitPixel);
        SolidBrush brush(Color(255, 255, 255, 255));
        SolidBrush greenBrush(Color(255, 0, 255, 127));

        graphics.DrawString(L"BetterAngle Pro", -1, &font, PointF(40, 40), &brush);
        
        // Progress Text (State determined by timing)
        static int state = 0; // External state sync needed if real-time
        // For this visual loop, we'll draw based on the current system time or a global
        
        EndPaint(hWnd, &ps);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void ShowSplashLoader(HINSTANCE hInst) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = DefWindowProc; // Simple splash
    wc.hInstance = hInst;
    wc.hbrBackground = CreateSolidBrush(RGB(15, 18, 22));
    wc.lpszClassName = L"BetterAngleSplash";
    RegisterClass(&wc);

    int w = 400, h = 200;
    int sx = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int sy = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    HWND hSplash = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, L"BetterAngleSplash", L"Loading...", WS_POPUP, sx, sy, w, h, NULL, NULL, hInst, NULL);
    SetLayeredWindowAttributes(hSplash, 0, 255, LWA_ALPHA);
    ShowWindow(hSplash, SW_SHOW);
    UpdateWindow(hSplash);

    HDC hdc = GetDC(hSplash);
    Graphics g(hdc);
    FontFamily ff(L"Segoe UI");
    Font f(&ff, 20, FontStyleBold, UnitPixel);
    Font f2(&ff, 14, FontStyleRegular, UnitPixel);
    SolidBrush white(Color(255, 255, 255, 255));
    SolidBrush green(Color(255, 0, 255, 127));

    // Phase 1: Update Check (2s)
    g.Clear(Color(255, 15, 18, 22));
    g.DrawString(L"BetterAngle Pro Edition", -1, &f, PointF(80, 60), &white);
    g.DrawString(L"Checking for Updates...", -1, &f2, PointF(120, 100), &green);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    ReleaseDC(hSplash, hdc);
    DestroyWindow(hSplash);
}
