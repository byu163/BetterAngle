#include <windows.h>
#include <string>
#include "shared/State.h"
#include <gdiplus.h>

extern float g_detectionRatio;

int g_wizStep = 0;
HWND g_hWiz = NULL;

LRESULT CALLBACK ThresholdWizProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_CREATE) {
        SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
        return 0;
    }
    if (message == WM_HOTKEY) {
        if (wParam == 99) { // F10 hotkey
            if (g_wizStep == 0) {
                g_freefallThreshold = g_detectionRatio;
                g_wizStep = 1;
                InvalidateRect(hWnd, NULL, FALSE);
            } else if (g_wizStep == 1) {
                g_glideThreshold = g_detectionRatio;
                void SaveSettings();
                SaveSettings();
                DestroyWindow(hWnd);
            }
        }
        return 0;
    }
    if (message == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        Gdiplus::Graphics graphics(hdc);
        graphics.Clear(Gdiplus::Color(255, 30, 30, 30));

        Gdiplus::FontFamily ff(L"Segoe UI");
        Gdiplus::Font f(&ff, 16, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
        Gdiplus::Font f2(&ff, 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush white(Gdiplus::Color(255, 255, 255, 255));
        Gdiplus::SolidBrush amber(Gdiplus::Color(255, 200, 150, 0));

        graphics.DrawString(L"FOV THRESHOLD CALIBRATOR", -1, &f, Gdiplus::PointF(10, 10), &white);

        if (g_wizStep == 0) {
            graphics.DrawString(L"1. Jump from bus and FREEFALL.", -1, &f2, Gdiplus::PointF(10, 40), &amber);
            graphics.DrawString(L"2. Press F10.", -1, &f2, Gdiplus::PointF(10, 60), &white);
        } else if (g_wizStep == 1) {
            graphics.DrawString(L"1. Deploy your GLIDER.", -1, &f2, Gdiplus::PointF(10, 40), &amber);
            graphics.DrawString(L"2. Press F10.", -1, &f2, Gdiplus::PointF(10, 60), &white);
        }

        EndPaint(hWnd, &ps);
        return 0;
    }
    if (message == WM_DESTROY) {
        UnregisterHotKey(hWnd, 99);
        g_hWiz = NULL;
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void StartThresholdWizard(HINSTANCE hInstance) {
    if (g_hWiz) return;
    g_wizStep = 0;
    
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = ThresholdWizProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"ThresholdWizardWnd";
    RegisterClass(&wc);

    int sw = GetSystemMetrics(SM_CXSCREEN);
    
    g_hWiz = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        L"ThresholdWizardWnd", L"Threshold Wizard",
        WS_POPUP,
        sw / 2 - 150, 50, 300, 100,
        NULL, NULL, hInstance, NULL
    );

    RegisterHotKey(g_hWiz, 99, 0, VK_F10);
    ShowWindow(g_hWiz, SW_SHOW);
    UpdateWindow(g_hWiz);
}
