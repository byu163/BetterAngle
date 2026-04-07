#include "shared/Overlay.h"
#include <gdiplus.h>
#include <iostream>
#include <string>

using namespace Gdiplus;

void DrawOverlay(HWND hwnd, double angle, const char* status) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    
    // Double Buffering
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

    // Clear with Invisible
    graphics.Clear(Color(0, 0, 0));

    // 1. Draw Modern Glass Container (Rounded Rectangle)
    GraphicsPath path;
    int rw = 400; // Wider for tabs
    int rh = 280;
    int rx = 40;
    int ry = 40;
    int r = 20; 
    
    path.AddArc(rx, ry, r, r, 180, 90);
    path.AddArc(rx + rw - r, ry, r, r, 270, 90);
    path.AddArc(rx + rw - r, ry + rh - r, r, r, 0, 90);
    path.AddArc(rx, ry + rh - r, r, r, 90, 90);
    path.CloseFigure();

    // Background Gradient (Dark Glass)
    LinearGradientBrush brush(Point(rx, ry), Point(rx, ry + rh), 
                             Color(210, 20, 24, 30), Color(210, 10, 12, 16));
    graphics.FillPath(&brush, &path);

    // Subtle Border
    Pen borderPen(Color(100, 255, 255, 255), 1);
    graphics.DrawPath(&borderPen, &path);

    // 2. Draw Tabs Header
    FontFamily fontFamily(L"Segoe UI");
    Font tabFont(&fontFamily, 12, FontStyleBold, UnitPixel);
    SolidBrush activeTabBrush(Color(255, 255, 255, 255));
    SolidBrush inactiveTabBrush(Color(120, 150, 160, 170));
    
    // Simple Tab Indicator (Underline for active tab)
    graphics.DrawString(L"DASHBOARD", -1, &tabFont, PointF(rx + 30, ry + 20), &activeTabBrush);
    graphics.DrawString(L"STATISTICS", -1, &tabFont, PointF(rx + 150, ry + 20), &inactiveTabBrush);
    graphics.DrawString(L"CLOUD", -1, &tabFont, PointF(rx + 270, ry + 20), &inactiveTabBrush);
    
    Pen tabUnderline(Color(255, 0, 255, 127), 3);
    graphics.DrawLine(&tabUnderline, rx + 30, ry + 45, rx + 120, ry + 45); // Dashboard is active

    // 3. Draw Angle Text (Main Module)
    Font font(&fontFamily, 64, FontStyleBold, UnitPixel);
    SolidBrush textBrush(Color(255, 0, 255, 127)); 

    std::wstring angleStr = std::to_wstring(angle);
    angleStr = angleStr.substr(0, angleStr.find(L'.') + 2); // 0.0
    angleStr += L"\u00B0"; 

    graphics.DrawString(angleStr.c_str(), -1, &font, PointF(rx + 30, ry + 65), &textBrush);

    // 4. Status Indicator Circle (Green = Normal, Cyan = Diving)
    Color indicatorColor = Color(255, 0, 255, 127); 
    if (strstr(status, "Diving") || strstr(status, "High")) {
        indicatorColor = Color(255, 0, 255, 255); // Cyan for High FOV
    }
    
    SolidBrush indicatorBrush(indicatorColor);
    graphics.FillEllipse(&indicatorBrush, rx + 195, ry + 95, 14, 14); 
    
    Pen glowPen(Color(100, indicatorColor.GetR(), indicatorColor.GetG(), indicatorColor.GetB()), 4);
    graphics.DrawEllipse(&glowPen, rx + 195, ry + 95, 14, 14);

    // 5. Info Section
    Font subFont(&fontFamily, 14, FontStyleBold, UnitPixel);
    SolidBrush greyBrush(Color(255, 160, 170, 180));
    graphics.DrawString(L"CURRENT ANGLE (LIVE)", -1, &subFont, PointF(rx + 30, ry + 55), &greyBrush);

    Font detailFont(&fontFamily, 12, FontStyleRegular, UnitPixel);
    std::string statStr = std::string("Mode: ") + status;
    std::wstring wStat = std::wstring(statStr.begin(), statStr.end());
    graphics.DrawString(wStat.c_str(), -1, &detailFont, PointF(rx + 30, ry + 165), &greyBrush);

    graphics.DrawString(L"Performance: Super Low Latency (Win32)", -1, &detailFont, PointF(rx + 30, ry + 190), &greyBrush);
    
    std::string verStr = "Status: Connected (v4.5 Pro)";
    std::wstring wVer(verStr.begin(), verStr.end());
    graphics.DrawString(wVer.c_str(), -1, &detailFont, PointF(rx + 30, ry + 215), &greyBrush);

    Font hintFont(&fontFamily, 10, FontStyleItalic, UnitPixel);
    graphics.DrawString(L"Ctrl+U to lock/unlock for gaming", -1, &hintFont, PointF(rx + 30, ry + 245), &greyBrush);

    // 6. Accent Line
    Pen accentPen(Color(255, 0, 255, 127), 2);
    graphics.DrawLine(&accentPen, rx + 30, ry + 145, rx + 370, ry + 145);

    // Blit to Screen
    BitBlt(hdc, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);
    
    // Cleanup
    SelectObject(hdcMem, hOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    EndPaint(hwnd, &ps);
}
