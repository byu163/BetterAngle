#include "shared/State.h"
#include "shared/Logic.h"
#include <gdiplus.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>

using namespace Gdiplus;

bool IsFortniteFocused();

// Helper: format float to N decimal places
static std::wstring FmtFloat(double v, int decimals = 2) {
    std::wostringstream ss;
    ss << std::fixed << std::setprecision(decimals) << v;
    return ss.str();
}

// Static FPS tracking
static ULONGLONG s_lastFrameTime = 0;
static float     s_fps           = 0.0f;
static int       s_frameCount    = 0;
static ULONGLONG s_fpsTimer      = 0;

static void TickFPS() {
    ULONGLONG now = GetTickCount64();
    s_frameCount++;
    if (now - s_fpsTimer >= 500) {          // update every 0.5s
        s_fps = s_frameCount * 1000.0f / float(now - s_fpsTimer);
        s_frameCount = 0;
        s_fpsTimer   = now;
    }
}

void DrawOverlay(HWND hwnd, double angle, float detectionRatio, bool showCrosshair) {
    TickFPS();

    // Cache GDI+ Resources (Memory Leak Fix)
    static FontFamily ff(L"Segoe UI");
    static Font Font_Large(&ff, 66, FontStyleBold, UnitPixel); // Increased for better prominence
    static Font Font_Med(&ff, 28, FontStyleBold, UnitPixel);
    static Font Font_Small(&ff, 15, FontStyleRegular, UnitPixel);
    static Font Font_Tiny(&ff, 9, FontStyleRegular, UnitPixel);
    static Font Font_DbgTitle(&ff, 11, FontStyleBold, UnitPixel);
    static Font Font_DbgKey(&ff, 10, FontStyleBold, UnitPixel);
    static Font Font_DbgVal(&ff, 10, FontStyleRegular, UnitPixel);

    // Cached Brushes/Pens to prevent per-frame allocation leaks
    static SolidBrush dimBrush(Color(120, 0, 0, 0));
    static SolidBrush whiteBrush(Color(255, 255, 255, 255));
    static SolidBrush dimWhite(Color(180, 220, 220, 220));
    static SolidBrush greenBrush(Color(255, 0, 255, 127)); // Vibrant Neon Green
    
    static Pen shadowRingPen(Color(80, 0, 0, 0), 6.0f);

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rect;
    GetClientRect(hwnd, &rect);
    int sw = rect.right  - rect.left;
    int sh = rect.bottom - rect.top;

    HDC      hdcMem = CreateCompatibleDC(hdc);
    HBITMAP  hbmMem = CreateCompatibleBitmap(hdc, sw, sh);
    HGDIOBJ  hOld   = SelectObject(hdcMem, hbmMem);

    Graphics graphics(hdcMem);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    graphics.Clear(Color(0, 0, 0, 0));

    if (g_currentSelection != NONE) {
        graphics.FillRectangle(&dimBrush, 0, 0, sw, sh);
        // ... (ROI/Color picking simplified for brevity, kept essential)
    }

    if (showCrosshair) {
        float cx = sw / 2.0f + g_crossOffsetX, cy = sh / 2.0f + g_crossOffsetY;
        float a = 255.0f;
        if (g_crossPulse) {
            LARGE_INTEGER freq, cnt;
            QueryPerformanceFrequency(&freq);
            QueryPerformanceCounter(&cnt);
            float t = float(cnt.QuadPart % (freq.QuadPart * 2)) / float(freq.QuadPart);
            float opacity = 0.5f + 0.5f * sinf(t * 3.14159265f * 2.0f);
            if (opacity < 0.05f) opacity = 0.05f;
            a = opacity * 255.0f;
        }
        Color  crossC((BYTE)a, GetRValue(g_crossColor), GetGValue(g_crossColor), GetBValue(g_crossColor));
        Pen    crossPen(crossC, g_crossThickness);
        crossPen.SetLineJoin(LineJoinRound);
        float rad = g_crossAngle * (3.1415926535f / 180.0f), sinR = sinf(rad), cosR = cosf(rad);
        float l = (sw > sh ? sw : sh) * 3.0f;
        graphics.DrawLine(&crossPen, cx - sinR*l, cy + cosR*l, cx + sinR*l, cy - cosR*l);
        graphics.DrawLine(&crossPen, cx - cosR*l, cy - sinR*l, cx + cosR*l, cy + sinR*l);
    }

    const int rx = g_hudX, ry = g_hudY, rw = 160, rh = 80, RAD = 12;
    GraphicsPath path;
    path.AddArc(rx, ry, RAD, RAD, 180, 90);
    path.AddArc(rx + rw - RAD, ry, RAD, RAD, 270, 90);
    path.AddArc(rx + rw - RAD, ry + rh - RAD, RAD, RAD, 0, 90);
    path.AddArc(rx, ry + rh - RAD, RAD, RAD, 90, 90);
    path.CloseFigure();

    // Premium Slate Gradient Background
    LinearGradientBrush bgGrad(Rect(rx, ry, rw, rh), Color(255, 32, 35, 42), Color(255, 12, 14, 18), LinearGradientModeVertical);
    graphics.FillPath(&bgGrad, &path);

    Color borderCol = g_isDiving ? Color(255, 0, 255, 127) : Color(255, 60, 65, 75);
    Pen borderPen(borderCol, 1.5f);
    graphics.DrawPath(&borderPen, &path);

    std::wstring angleStr = FmtFloat(angle, 1) + L"°"; 
    StringFormat sf;
    sf.SetAlignment(StringAlignmentCenter); sf.SetLineAlignment(StringAlignmentCenter);
    
    // Draw Neon Green Angle
    graphics.DrawString(angleStr.c_str(), -1, &Font_Large, RectF((REAL)rx, (REAL)ry - 2, (REAL)rw, (REAL)rh), &sf, &greenBrush);

    if (g_debugMode) {
        const int DBG_ROWS = 14, ROW_H = 18;
        int dw = 370, dh = 28 + DBG_ROWS * ROW_H + 10;
        int dx = rx, dy = ry + rh + 10;
        if (dy + dh > sh) dy = ry - dh - 8;

        GraphicsPath dbgPath;
        dbgPath.AddRectangle(Rect(dx, dy, dw, dh));
        
        static SolidBrush colTitle(Color(255, 255, 255, 255));
        static SolidBrush colKey(Color(255, 180, 180, 180));
        static SolidBrush colVal(Color(255, 220, 230, 240));
        static SolidBrush colGood(Color(255, 0, 255, 127));
        static SolidBrush titleBarBrush(Color(120, 20, 22, 28));

        LinearGradientBrush dbgBg(Point(dx, dy), Point(dx, dy + dh), Color(235, 12, 14, 18), Color(235, 5, 6, 8));
        graphics.FillPath(&dbgBg, &dbgPath);
        graphics.DrawPath(&Pen(Color(120, 0, 255, 127), 1.0f), &dbgPath);
        graphics.FillRectangle(&titleBarBrush, dx, dy, dw, 22);
        graphics.DrawString(L"  DEBUG DASHBOARD", -1, &Font_DbgTitle, PointF(float(dx + 4), float(dy + 5)), &colTitle);

        int row = 0;
        auto DrawRow = [&](const wchar_t* key, const std::wstring& val, SolidBrush* valBrush) {
            float y = float(dy + 28 + row * ROW_H);
            graphics.DrawString(key, -1, &Font_DbgKey, PointF(float(dx + 8), y), &colKey);
            graphics.DrawString(val.c_str(), -1, &Font_DbgVal, PointF(float(dx + 175), y), valBrush);
            row++;
        };

        DrawRow(L"Angle (raw)", FmtFloat(angle, 2) + L"°", &colVal);
        DrawRow(L"Diving", g_isDiving ? L"YES" : L"NO", g_isDiving ? &colGood : &colVal);
        DrawRow(L"Fortnite Focused", IsFortniteFocused() ? L"YES" : L"NO", &colVal);
    }

    SolidBrush tinyBrush(Color(g_isDraggingHUD ? 180 : 80, 200, 210, 220));
    StringFormat sfBottom; sfBottom.SetAlignment(StringAlignmentCenter);
    graphics.DrawString(L"⠿ drag", -1, &Font_Tiny, RectF((REAL)rx, (REAL)ry + rh - 14, (REAL)rw, 14.0f), &sfBottom, &tinyBrush);

    BitBlt(hdc, 0, 0, sw, sh, hdcMem, 0, 0, SRCCOPY);
    SelectObject(hdcMem, hOld); DeleteObject(hbmMem); DeleteDC(hdcMem);
    EndPaint(hwnd, &ps);
}
