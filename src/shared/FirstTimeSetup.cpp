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
std::wstring g_setupDPI  = L"";
std::wstring g_setupSensX = L"";
std::wstring g_setupSensY = L"";
bool g_extractedConfig = false;
int g_focusedInput = 1;

// ── Palette ────────────────────────────────────────────────────────────────
static const Color BG        (255,  13,  15,  20);
static const Color INPUT_BG  (255,  28,  31,  40);
static const Color ACCENT    (255,  59, 130, 246);
static const Color ACCENT_DIM(255,  37,  60, 110);
static const Color WHITE     (255, 255, 255, 255);
static const Color GRAY      (255, 110, 115, 135);
static const Color DIMGRAY   (255,  55,  60,  75);
static const Color CYAN      (255,   0, 210, 200);

// ── Rounded-rect helpers ───────────────────────────────────────────────────
static void FillRR(Graphics& g, Brush* br, int x, int y, int w, int h, int r) {
    GraphicsPath p;
    p.AddArc(x,         y,         r*2, r*2, 180, 90);
    p.AddArc(x+w-r*2,   y,         r*2, r*2, 270, 90);
    p.AddArc(x+w-r*2,   y+h-r*2,   r*2, r*2,   0, 90);
    p.AddArc(x,         y+h-r*2,   r*2, r*2,  90, 90);
    p.CloseFigure();
    g.FillPath(br, &p);
}
static void DrawRR(Graphics& g, Pen* pen, int x, int y, int w, int h, int r) {
    GraphicsPath p;
    p.AddArc(x,         y,         r*2, r*2, 180, 90);
    p.AddArc(x+w-r*2,   y,         r*2, r*2, 270, 90);
    p.AddArc(x+w-r*2,   y+h-r*2,   r*2, r*2,   0, 90);
    p.AddArc(x,         y+h-r*2,   r*2, r*2,  90, 90);
    p.CloseFigure();
    g.DrawPath(pen, &p);
}

// ── Centered draw helper ───────────────────────────────────────────────────
static void DrawC(Graphics& g, Font* f, const wchar_t* t, SolidBrush* br,
                  int winW, float y, float h = 40.0f) {
    StringFormat sf;
    sf.SetAlignment(StringAlignmentCenter);
    g.DrawString(t, -1, f, RectF(0.0f, y, (REAL)winW, h), &sf, br);
}

// ── Profile save ──────────────────────────────────────────────────────────
void FinishSetup() {
    Profile p;
    p.name          = L"Default";
    p.roi_h         = 60; p.roi_w = 400;
    p.roi_x         = 760; p.roi_y = 650;
    p.target_color  = RGB(150, 150, 150);
    p.tolerance     = 25;

    double dpiVal = 800.0;
    try { dpiVal = std::stod(g_setupDPI); } catch(...) {}
    if (dpiVal <= 0) dpiVal = 800.0;

    double sensX = 0.05, sensY = 0.05;
    try { sensX = std::stod(g_setupSensX); } catch(...) {}
    try { sensY = std::stod(g_setupSensY); } catch(...) {}
    if (sensX <= 0) sensX = 0.05;
    if (sensY <= 0) sensY = 0.05;

    p.dpi                    = (int)dpiVal;
    p.sensitivityX           = sensX;
    p.sensitivityY           = sensY;
    p.divingScaleMultiplier  = 1.22;
    p.fov                    = 80.0f;
    p.resolutionWidth        = GetSystemMetrics(SM_CXSCREEN);
    p.resolutionHeight       = GetSystemMetrics(SM_CYSCREEN);
    p.renderScale            = 100.0f;

    extern std::vector<Profile> g_allProfiles;
    extern int g_selectedProfileIdx;

    if (g_allProfiles.empty()) {
        p.Save(GetAppStoragePath() + L"Default.json");
        g_allProfiles.push_back(p);
    } else {
        Profile& e = g_allProfiles[g_selectedProfileIdx];
        p.name = e.name; p.roi_h = e.roi_h; p.roi_w = e.roi_w;
        p.roi_x = e.roi_x; p.roi_y = e.roi_y;
        p.target_color = e.target_color; p.tolerance = e.tolerance;
        g_allProfiles[g_selectedProfileIdx] = p;
        p.Save(GetAppStoragePath() + p.name + L".json");
    }
}

// ── Window Procedure ───────────────────────────────────────────────────────
LRESULT CALLBACK FirstTimeSetupProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    // ── Keep window visible when it loses focus (never hide/minimize) ──────
    if (msg == WM_ACTIVATE) {
        if (LOWORD(wParam) == WA_INACTIVE) {
            // Bring back to top without forcing foreground (avoids OS rejection)
            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        return 0;
    }
    if (msg == WM_NCACTIVATE) {
        // Return TRUE so the non-client area never draws as "inactive"
        return TRUE;
    }
    if (msg == WM_MOUSEACTIVATE) return MA_ACTIVATE;

    // ── Drag from top header ───────────────────────────────────────────────
    if (msg == WM_NCHITTEST) {
        LRESULT hit = DefWindowProc(hWnd, msg, wParam, lParam);
        if (hit == HTCLIENT) {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(hWnd, &pt);
            if (pt.y < 55) return HTCAPTION;
        }
        return hit;
    }

    // ── Tab key on step 2 switches fields ─────────────────────────────────
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_TAB && g_setupState == 2) {
            g_focusedInput = (g_focusedInput == 1) ? 2 : 1;
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;
        }
        // Escape cancels — but we just re-show the window (no close during setup)
        return 0;
    }

    // ── Text input ────────────────────────────────────────────────────────
    if (msg == WM_CHAR) {
        if (wParam == VK_RETURN) {
            if (g_setupState == 1 && !g_setupDPI.empty()) {
                g_setupState   = 2;
                g_focusedInput = 1;
            } else if (g_setupState == 2) {
                FinishSetup();
                DestroyWindow(hWnd);
            }
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;
        }
        std::wstring* cur = nullptr;
        if      (g_setupState == 1)              cur = &g_setupDPI;
        else if (g_focusedInput == 1)            cur = &g_setupSensX;
        else                                      cur = &g_setupSensY;

        if (wParam == VK_BACK) {
            if (cur && !cur->empty()) cur->pop_back();
        } else if (wParam >= 32 && wParam <= 126) {
            if (cur) cur->push_back((wchar_t)wParam);
        }
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }

    // ── Mouse clicks ───────────────────────────────────────────────────────
    if (msg == WM_LBUTTONDOWN) {
        int mx = GET_X_LPARAM(lParam), my = GET_Y_LPARAM(lParam);
        RECT rc; GetClientRect(hWnd, &rc);
        int W = rc.right, H = rc.bottom;

        if (g_setupState == 1) {
            int bx = (W-200)/2, by = H - 80;
            if (mx >= bx && mx <= bx+200 && my >= by && my <= by+44 && !g_setupDPI.empty()) {
                g_setupState = 2; g_focusedInput = 1;
                InvalidateRect(hWnd, NULL, FALSE);
            }
        } else {
            // Field hit-test
            int fieldY = 195, fieldH = 44;
            int fw2    = (W - 75) / 2;
            int fxA    = 30, fxB = fxA + fw2 + 15;
            if (my >= fieldY && my <= fieldY + fieldH) {
                if      (mx >= fxA && mx <= fxA + fw2) { g_focusedInput = 1; InvalidateRect(hWnd, NULL, FALSE); }
                else if (mx >= fxB && mx <= fxB + fw2) { g_focusedInput = 2; InvalidateRect(hWnd, NULL, FALSE); }
            }
            // Confirm button
            int bx = (W-200)/2, by = H - 80;
            if (mx >= bx && mx <= bx+200 && my >= by && my <= by+44) {
                FinishSetup();
                DestroyWindow(hWnd);
            }
        }
        SetFocus(hWnd);
        // fall through for default activation
    }

    // ── Paint ──────────────────────────────────────────────────────────────
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc; GetClientRect(hWnd, &rc);
        int W = rc.right, H = rc.bottom;

        // Double-buffer to eliminate flicker
        HDC     hdcBuf = CreateCompatibleDC(hdc);
        HBITMAP hbmBuf = CreateCompatibleBitmap(hdc, W, H);
        HGDIOBJ hOld   = SelectObject(hdcBuf, hbmBuf);
        {
            Graphics g(hdcBuf);
            g.SetSmoothingMode(SmoothingModeAntiAlias);
            g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

            g.Clear(BG);

            FontFamily ff(L"Segoe UI");
            Font fStep  (&ff, 11, FontStyleRegular, UnitPixel);
            Font fTitle (&ff, 28, FontStyleBold,    UnitPixel);
            Font fSub   (&ff, 12, FontStyleRegular, UnitPixel);
            Font fLabel (&ff, 11, FontStyleRegular, UnitPixel);
            Font fInput (&ff, 14, FontStyleRegular, UnitPixel);
            Font fBtn   (&ff, 13, FontStyleBold,    UnitPixel);
            Font fBrand (&ff, 10, FontStyleRegular, UnitPixel);

            SolidBrush brWhite  (WHITE);
            SolidBrush brGray   (GRAY);
            SolidBrush brDim    (DIMGRAY);
            SolidBrush brAccent (ACCENT);
            SolidBrush brDimAcc (ACCENT_DIM);
            SolidBrush brInput  (INPUT_BG);
            SolidBrush brCyan   (CYAN);

            Pen penAccent (ACCENT,  1.5f);
            Pen penDim    (DIMGRAY, 1.0f);

            StringFormat sfC, sfL;
            sfC.SetAlignment(StringAlignmentCenter);
            sfC.SetLineAlignment(StringAlignmentCenter);
            sfL.SetAlignment(StringAlignmentNear);
            sfL.SetLineAlignment(StringAlignmentCenter);

            // ── Step indicator ─────────────────────────────────────────
            const wchar_t* stepTxt = (g_setupState == 1) ? L"Step 1 of 2" : L"Step 2 of 2";
            DrawC(g, &fStep, stepTxt, &brGray, W, 28.0f, 20.0f);

            // ── Progress bar (two pills) ───────────────────────────────
            int pilW = 70, pilH = 3, pilY = 52;
            int pilX = (W - pilW*2 - 8) / 2;
            SolidBrush brP1(g_setupState >= 1 ? ACCENT : DIMGRAY);
            SolidBrush brP2(g_setupState >= 2 ? ACCENT : DIMGRAY);
            FillRR(g, &brP1, pilX,        pilY, pilW, pilH, 1);
            FillRR(g, &brP2, pilX+pilW+8, pilY, pilW, pilH, 1);

            // ── Heading ────────────────────────────────────────────────
            const wchar_t* heading = (g_setupState == 1) ? L"Mouse DPI" : L"Fortnite Sensitivity";
            DrawC(g, &fTitle, heading, &brWhite, W, 68.0f, 40.0f);

            // thin separator line
            Pen sepPen(DIMGRAY, 1.0f);
            g.DrawLine(&sepPen, 30, 116, W-30, 116);

            if (g_setupState == 1) {
                // ── Subtitle ───────────────────────────────────────────
                DrawC(g, &fSub, L"Check your mouse software or the label on your sensor", &brGray, W, 124.0f, 28.0f);

                // ── Input ──────────────────────────────────────────────
                int ix = 30, iw = W-60, iy = 162, ih = 46;
                FillRR(g, &brInput, ix, iy, iw, ih, 8);
                DrawRR(g, &penAccent, ix, iy, iw, ih, 8);

                std::wstring d = g_setupDPI.empty() ? L"e.g. 800" : g_setupDPI + L"│";
                SolidBrush* tb = g_setupDPI.empty() ? &brDim : &brWhite;
                g.DrawString(d.c_str(), -1, &fInput,
                             RectF((REAL)(ix+14),(REAL)iy,(REAL)(iw-28),(REAL)ih), &sfL, tb);

                // ── Next button ────────────────────────────────────────
                int bx = (W-200)/2, by = H - 80;
                bool ready = !g_setupDPI.empty();
                FillRR(g, ready ? &brAccent : &brDim, bx, by, 200, 44, 8);
                g.DrawString(L"Next  →", -1, &fBtn,
                             RectF((REAL)bx,(REAL)by,200.0f,44.0f), &sfC, &brWhite);

            } else {
                // ── Subtitle ───────────────────────────────────────────
                const wchar_t* sub = g_extractedConfig
                    ? L"Pre-filled from GameUserSettings.ini"
                    : L"Enter your in-game sensitivity values";
                SolidBrush* subBr = g_extractedConfig ? &brCyan : &brGray;
                DrawC(g, &fSub, sub, subBr, W, 124.0f, 28.0f);

                // ── Two input fields ───────────────────────────────────
                int fw2 = (W - 75) / 2;
                int fxA = 30, fxB = fxA + fw2 + 15;
                int fy  = 195, fh = 44;

                // Labels
                g.DrawString(L"Sensitivity X", -1, &fLabel, PointF((REAL)fxA, 178.0f), &brGray);
                g.DrawString(L"Sensitivity Y", -1, &fLabel, PointF((REAL)fxB, 178.0f), &brGray);

                // Field A
                FillRR(g, &brInput, fxA, fy, fw2, fh, 8);
                DrawRR(g, g_focusedInput==1 ? &penAccent : &penDim, fxA, fy, fw2, fh, 8);
                {
                    std::wstring dx = g_setupSensX.empty() ? L"0.05" : g_setupSensX + (g_focusedInput==1 ? L"│":L"");
                    SolidBrush* tb = g_setupSensX.empty() ? &brDim : &brWhite;
                    g.DrawString(dx.c_str(), -1, &fInput,
                                 RectF((REAL)(fxA+10),(REAL)fy,(REAL)(fw2-20),(REAL)fh), &sfL, tb);
                }

                // Field B
                FillRR(g, &brInput, fxB, fy, fw2, fh, 8);
                DrawRR(g, g_focusedInput==2 ? &penAccent : &penDim, fxB, fy, fw2, fh, 8);
                {
                    std::wstring dy = g_setupSensY.empty() ? L"0.05" : g_setupSensY + (g_focusedInput==2 ? L"│":L"");
                    SolidBrush* tb = g_setupSensY.empty() ? &brDim : &brWhite;
                    g.DrawString(dy.c_str(), -1, &fInput,
                                 RectF((REAL)(fxB+10),(REAL)fy,(REAL)(fw2-20),(REAL)fh), &sfL, tb);
                }

                // Tab hint
                g.DrawString(L"Tab to switch fields  ·  Enter to confirm",
                             -1, &fLabel, PointF(30.0f, 248.0f), &brDim);

                // ── Confirm button ─────────────────────────────────────
                int bx = (W-200)/2, by = H - 80;
                FillRR(g, &brAccent, bx, by, 200, 44, 8);
                g.DrawString(L"Confirm  ✓", -1, &fBtn,
                             RectF((REAL)bx,(REAL)by,200.0f,44.0f), &sfC, &brWhite);
            }

            // ── Brand + drag hint ──────────────────────────────────────
            g.DrawString(L"BetterAngle Pro", -1, &fBrand, PointF(16.0f,(REAL)(H-20)), &brCyan);
            g.DrawString(L"drag ↑", -1, &fBrand, PointF((REAL)(W-50), 8.0f), &brDim);
        }
        BitBlt(hdc, 0, 0, W, H, hdcBuf, 0, 0, SRCCOPY);
        SelectObject(hdcBuf, hOld);
        DeleteObject(hbmBuf);
        DeleteDC(hdcBuf);

        EndPaint(hWnd, &ps);
        return 0;
    }

    if (msg == WM_DESTROY) return 0; // Do NOT post quit — modal loop exits via IsWindow
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// ── Modal loop ────────────────────────────────────────────────────────────
void StartModalSetupLoop(HWND hwnd) {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (!IsWindow(hwnd)) break;
    }
}

// ── Entry point ───────────────────────────────────────────────────────────
void ShowFirstTimeSetup(HINSTANCE hInstance) {
    g_setupState      = 1;
    g_setupDPI        = L"";
    g_setupSensX      = L"";
    g_setupSensY      = L"";
    g_focusedInput    = 1;
    g_extractedConfig = false;

    // Try auto-detect from Fortnite config
    std::string ini;
    wchar_t appdata[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appdata))) {
        std::wstring p = std::wstring(appdata) +
            L"\\FortniteGame\\Saved\\Config\\WindowsClient\\GameUserSettings.ini";
        std::ifstream ifs(p.c_str());
        if (ifs.good())
            ini.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    }
    auto extract = [&](const std::string& key) -> std::wstring {
        size_t pos = ini.find(key + "=");
        if (pos == std::string::npos) return L"";
        size_t end = ini.find_first_of("\r\n", pos);
        std::string val = ini.substr(pos + key.size() + 1, end - (pos + key.size() + 1));
        return std::wstring(val.begin(), val.end());
    };
    g_setupSensX = extract("MouseX");
    g_setupSensY = extract("MouseY");
    if (!g_setupSensX.empty() || !g_setupSensY.empty())
        g_extractedConfig = true;

    // Register window class (safe to call multiple times — just fails silently)
    WNDCLASS wc     = { 0 };
    wc.style        = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc  = FirstTimeSetupProc;
    wc.hInstance    = hInstance;
    wc.hCursor      = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground= CreateSolidBrush(RGB(13, 15, 20));
    wc.lpszClassName= L"FTSWindowClass";
    RegisterClass(&wc); // safe to ignore return value on re-registration

    // Size: 520 wide, 360 tall — fits all content with breathing room
    int W = 520, H = 360;
    HWND hWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_APPWINDOW,
        L"FTSWindowClass", L"BetterAngle — Setup",
        WS_POPUP | WS_SYSMENU,
        GetSystemMetrics(SM_CXSCREEN)/2 - W/2,
        GetSystemMetrics(SM_CYSCREEN)/2 - H/2,
        W, H, NULL, NULL, hInstance, NULL
    );

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    SetForegroundWindow(hWnd);
    BringWindowToTop(hWnd);
    SetFocus(hWnd);

    StartModalSetupLoop(hWnd);
}
