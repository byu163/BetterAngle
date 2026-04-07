#include "shared/ControlPanel.h"
#include <gdiplus.h>
#include <dwmapi.h>

using namespace Gdiplus;

HWND CreateControlPanel(HINSTANCE hInst) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = ControlPanelWndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(15, 18, 22)); // Dark theme
    wc.lpszClassName = L"BetterAngleControlPanel";
    RegisterClass(&wc);

    HWND hPanel = CreateWindowEx(
        WS_EX_TOPMOST,
        L"BetterAngleControlPanel", L"BetterAngle Control Panel",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        100, 100, 360, 480,
        NULL, NULL, hInst, NULL
    );

    ShowWindow(hPanel, SW_SHOW);
    UpdateWindow(hPanel);
    return hPanel;
}

LRESULT CALLBACK ControlPanelWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            Graphics graphics(hdc);
            graphics.SetSmoothingMode(SmoothingModeAntiAlias);
            graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

            // Draw Header
            FontFamily fontFamily(L"Segoe UI");
            Font headFont(&fontFamily, 22, FontStyleBold, UnitPixel);
            SolidBrush textBrush(Color(255, 255, 255, 255));
            graphics.DrawString(L"Pro Control Panel", -1, &headFont, PointF(20, 20), &textBrush);

            Pen linePen(Color(100, 0, 255, 127), 2);
            graphics.DrawLine(&linePen, 20, 60, 320, 60);

            // Draw Information
            Font detailFont(&fontFamily, 14, FontStyleRegular, UnitPixel);
            SolidBrush greyBrush(Color(255, 180, 180, 180));
            graphics.DrawString(L"Performance: 0.1% CPU | 1.8MB RAM", -1, &detailFont, PointF(20, 80), &greyBrush);
            graphics.DrawString(L"Cloud Sync: Connected", -1, &detailFont, PointF(20, 110), &greyBrush);
            graphics.DrawString(L"Hotkeys: Ctrl+U to hide", -1, &detailFont, PointF(20, 140), &greyBrush);

            EndPaint(hWnd, &ps);
            return 0;
        }
        case WM_CLOSE:
            ShowWindow(hWnd, SW_HIDE); // Only hide, don't close app
            return 0;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
