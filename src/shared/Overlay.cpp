#include "shared/Overlay.h"
#include <gdiplus.h>
#include <iostream>
#include <string>

using namespace Gdiplus;

void DrawOverlay(HWND hwnd, double angle, const char* status) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    
    RECT rect;
    GetClientRect(hwnd, &rect);
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdc, w, h);
    HGDIOBJ hOld = SelectObject(hdcMem, hbmMem);
    
    Graphics graphics(hdcMem);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
    graphics.Clear(Color(0, 0, 0));

    // 1. Draw Clean Glass HUD (Minimalists)
    int rx = 40, ry = 40, rw = 320, rh = 160;
    GraphicsPath path;
    path.AddArc(rx, ry, 20, 20, 180, 90);
    path.AddArc(rx + rw - 20, ry, 20, 20, 270, 90);
    path.AddArc(rx + rw - 20, ry + rh - 20, 20, 20, 0, 90);
    path.AddArc(rx, ry + rh - 20, 20, 20, 90, 90);
    path.CloseFigure();

    LinearGradientBrush brush(Point(rx, ry), Point(rx, ry + rh), 
                             Color(180, 15, 18, 22), Color(180, 5, 6, 8));
    graphics.FillPath(&brush, &path);
    Pen borderPen(Color(80, 255, 255, 255), 1);
    graphics.DrawPath(&borderPen, &path);

    // 2. Draw Angle Text
    FontFamily fontFamily(L"Segoe UI");
    Font font(&fontFamily, 64, FontStyleBold, UnitPixel);
    SolidBrush textBrush(Color(255, 0, 255, 127)); 

    std::wstring angleStr = std::to_wstring(angle);
    angleStr = angleStr.substr(0, angleStr.find(L'.') + 2) + L"\u00B0"; 
    graphics.DrawString(angleStr.c_str(), -1, &font, PointF(rx + 30, ry + 45), &textBrush);

    // 3. Status Indicator Circle (Green/Cyan)
    Color indicatorColor = Color(255, 0, 255, 127); 
    if (strstr(status, "Diving") || strstr(status, "High")) {
        indicatorColor = Color(255, 0, 255, 255); 
    }
    graphics.FillEllipse(&SolidBrush(indicatorColor), rx + 205, ry + 75, 14, 14); 

    // 4. Subtitle
    Font subFont(&fontFamily, 14, FontStyleBold, UnitPixel);
    SolidBrush greyBrush(Color(255, 160, 170, 180));
    graphics.DrawString(L"CURRENT ANGLE (LIVE)", -1, &subFont, PointF(rx + 30, ry + 25), &greyBrush);

    // 5. Version Info
    Font miniFont(&fontFamily, 10, FontStyleRegular, UnitPixel);
    graphics.DrawString(L"v4.5.1 Pro Edition", -1, &miniFont, PointF(rx + 30, ry + 130), &greyBrush);

    BitBlt(hdc, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);
    SelectObject(hdcMem, hOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    EndPaint(hwnd, &ps);
}
