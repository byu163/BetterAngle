#include "shared/FirstTimeSetup.h"
#include <windows.h>
#include <windowsx.h>
#include <string>
#include <fstream>
#include <shlobj.h>
#include <vector>
#include <algorithm>
#include <cwctype>
#include "shared/Profile.h"
#include "shared/State.h"

// ─── Pure GDI colours (no GDI+, zero GPU cost) ───────────────────────────
static const COLORREF C_BG      = RGB( 10,  12,  18);
static const COLORREF C_INPUT   = RGB( 22,  24,  32);
static const COLORREF C_ACCENT  = RGB( 59, 130, 246);
static const COLORREF C_DIM     = RGB( 45,  50,  65);
static const COLORREF C_GRAY    = RGB(100, 105, 125);
static const COLORREF C_WHITE   = RGB(255, 255, 255);
static const COLORREF C_CYAN    = RGB( 34, 211, 238);
static const COLORREF C_BTNOFF  = RGB( 35,  38,  48);

static int g_setupState    = 2;
static std::wstring g_setupSensX = L"";
static std::wstring g_setupSensY = L"";
static bool g_extractedConfig    = false;
static int  g_focusedInput       = 1;

extern std::vector<Profile> g_allProfiles;
extern int g_selectedProfileIdx;

// ── GDI helpers ───────────────────────────────────────────────────────────
static void FillR(HDC hdc, int x, int y, int w, int h, COLORREF c) {
    RECT r = { x, y, x+w, y+h };
    HBRUSH br = CreateSolidBrush(c);
    FillRect(hdc, &r, br);
    DeleteObject(br);
}

static void DrawT(HDC hdc, const wchar_t* t, int x, int y, int w, int h,
                  COLORREF c, int sz, bool bold = false, UINT fmt = DT_CENTER|DT_VCENTER|DT_SINGLELINE) {
    HFONT f = CreateFontW(-sz, 0, 0, 0,
                          bold ? FW_BOLD : FW_NORMAL,
                          0, 0, 0, DEFAULT_CHARSET,
                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          CLEARTYPE_QUALITY, DEFAULT_PITCH,
                          L"Segoe UI");
    HFONT old = (HFONT)SelectObject(hdc, f);
    SetTextColor(hdc, c);
    SetBkMode(hdc, TRANSPARENT);
    RECT r = { x, y, x+w, y+h };
    DrawTextW(hdc, t, -1, &r, fmt);
    SelectObject(hdc, old);
    DeleteObject(f);
}

// ── Profile save ─────────────────────────────────────────────────────────
void FinishSetup() {
    Profile p;
    // Set sensible defaults first
    p.name = L"Default"; 
    p.tolerance = 25;
    p.roi_x = 760; p.roi_y = 640; p.roi_w = 400; p.roi_h = 70;
    p.target_color = RGB(150, 150, 150);
    p.fov = 80.0f;
    p.resolutionWidth  = GetSystemMetrics(SM_CXSCREEN);
    p.resolutionHeight = GetSystemMetrics(SM_CYSCREEN);
    p.renderScale = 100.0f;

    // Parse numeric values safely
    double sensX = 0.05, sensY = 0.05;
    try { if (!g_setupSensX.empty()) sensX  = std::stod(g_setupSensX); } catch(...) {}
    try { if (!g_setupSensY.empty()) sensY  = std::stod(g_setupSensY); } catch(...) {}
    
    p.sensitivityX = (std::max)(0.0001, sensX);
    p.sensitivityY = (std::max)(0.0001, sensY);

    if (g_allProfiles.empty()) {
        p.Save(GetAppStoragePath() + L"Default.json");
        g_allProfiles.push_back(p);
        g_selectedProfileIdx = 0;
    } else {
        // Update existing selected profile safely
        if (g_selectedProfileIdx < 0 || g_selectedProfileIdx >= (int)g_allProfiles.size()) {
            g_selectedProfileIdx = 0;
        }
        Profile& e = g_allProfiles[g_selectedProfileIdx];
        e.sensitivityX = p.sensitivityX;
        e.sensitivityY = p.sensitivityY;
        e.Save(GetAppStoragePath() + e.name + L".json");
    }
}

// ── Paint ────────────────────────────────────────────────────────────────
static void PaintSetup(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    RECT rc; GetClientRect(hWnd, &rc);
    int W = rc.right, H = rc.bottom;

    HDC     buf = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, W, H);
    HGDIOBJ old = SelectObject(buf, bmp);

    FillR(buf, 0, 0, W, H, C_BG);

    // Header drag area hint
    FillR(buf, 0, 0, W, 48, RGB(18, 20, 26));

    // Progress
    COLORREF p1 = (g_setupState >= 1) ? C_ACCENT : C_DIM;
    COLORREF p2 = (g_setupState >= 2) ? C_ACCENT : C_DIM;
    int pilX = (W - 148) / 2;
    FillR(buf, pilX,      22, 70, 2, p1);
    FillR(buf, pilX + 78, 22, 70, 2, p2);

    DrawT(buf, L"CALIBRATING SENSITIVITY",
          0, 36, W, 18, C_ACCENT, 9, true);

    const wchar_t* hd = L"In-Game Sens";
    DrawT(buf, hd, 0, 64, W, 34, C_WHITE, 28, true);

    {
        const wchar_t* sub = g_extractedConfig
            ? L"Retrieved automatically from your game files."
            : L"Enter your X and Y sensitivity from game settings.";
        DrawT(buf, sub, 30, 108, W-60, 20, g_extractedConfig ? C_CYAN : C_GRAY, 11);

        int fw = (W - 90) / 2;
        int fxA = 40, fxB = fxA + fw + 10;
        int fy = 166, fh = 44;

        auto drawField = [&](int fx, int focusIdx, const std::wstring& label, const std::wstring& val) {
            DrawT(buf, label.c_str(), fx, fy-22, fw, 20, C_GRAY, 10, false, DT_LEFT|DT_SINGLELINE);
            FillR(buf, fx, fy, fw, fh, C_INPUT);
            HPEN pen = CreatePen(PS_SOLID, 1, (g_focusedInput == focusIdx) ? C_ACCENT : C_DIM);
            HPEN op  = (HPEN)SelectObject(buf, pen);
            HBRUSH ob = (HBRUSH)SelectObject(buf, GetStockObject(NULL_BRUSH));
            Rectangle(buf, fx, fy, fx+fw, fy+fh);
            SelectObject(buf, op); SelectObject(buf, ob); DeleteObject(pen);
            std::wstring d = val.empty() ? L"0.00" : val + (g_focusedInput==focusIdx ? L"_" : L"");
            DrawT(buf, d.c_str(), fx+14, fy, fw-28, fh, val.empty() ? C_DIM : C_WHITE, 15, false, DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        };
        drawField(fxA, 1, L"SENSITIVITY X", g_setupSensX);
        drawField(fxB, 2, L"SENSITIVITY Y", g_setupSensY);

        int bx = (W-200)/2, by = H-74, bw = 200, bh = 42;
        FillR(buf, bx, by, bw, bh, C_ACCENT);
        DrawT(buf, L"FINALIZE SETUP  ✓", bx, by, bw, bh, C_WHITE, 12, true);
    }

    DrawT(buf, L"PRO VERSION ACTIVE", 20, H-22, 160, 16, C_ACCENT, 9, true, DT_LEFT|DT_SINGLELINE);
    DrawT(buf, L"DRAG TO MOVE", W-100, H-22, 80, 16, C_DIM, 9, false, DT_RIGHT|DT_SINGLELINE);

    BitBlt(hdc, 0, 0, W, H, buf, 0, 0, SRCCOPY);
    SelectObject(buf, old); DeleteObject(bmp); DeleteDC(buf);
    EndPaint(hWnd, &ps);
}

// ── Window Procedure ──────────────────────────────────────────────────────
LRESULT CALLBACK FirstTimeSetupProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCHITTEST: {
        LRESULT hit = DefWindowProc(hWnd, msg, wParam, lParam);
        if (hit == HTCLIENT) {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(hWnd, &pt);
            if (pt.y < 50) return HTCAPTION;
        }
        return hit;
    }
    case WM_ERASEBKGND: return 1;

    case WM_KEYDOWN:
        if (wParam == VK_TAB && g_setupState == 2) {
            g_focusedInput = (g_focusedInput == 1) ? 2 : 1;
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;
        }
        if (wParam == VK_RETURN) {
            if (g_setupState == 2) {
                FinishSetup(); DestroyWindow(hWnd);
            }
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;
        }
        return 0;

    case WM_CHAR: {
        std::wstring* cur = (g_focusedInput == 1) ? &g_setupSensX : &g_setupSensY;
        if (wParam == VK_BACK) {
            if (!cur->empty()) cur->pop_back();
        } else if (iswdigit((wchar_t)wParam) || (wchar_t)wParam == L'.') {
            // Basic validation: only allow one dot
            if ((wchar_t)wParam == L'.' && cur->find(L'.') != std::wstring::npos) return 0;
            if (cur->length() < 10) cur->push_back((wchar_t)wParam);
        }
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int mx = GET_X_LPARAM(lParam), my = GET_Y_LPARAM(lParam);
        RECT rc; GetClientRect(hWnd, &rc);
        int W = rc.right, H = rc.bottom;

        {
            int fw = (W-90)/2, fxA = 40, fxB = fxA+fw+10, fy = 166, fh = 44;
            if (my >= fy && my <= fy+fh) {
                if      (mx>=fxA && mx<=fxA+fw) g_focusedInput=1;
                else if (mx>=fxB && mx<=fxB+fw) g_focusedInput=2;
            }
            int bx=(W-200)/2, by=H-74;
            if (mx>=bx && mx<=bx+200 && my>=by && my<=by+42) {
                FinishSetup(); DestroyWindow(hWnd);
            }
        }
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }

    case WM_PAINT: PaintSetup(hWnd); return 0;
    case WM_CLOSE: DestroyWindow(hWnd); return 0;
    case WM_DESTROY: return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void ShowFirstTimeSetup(HINSTANCE hInstance) {
    // Reset state locally
    g_setupState = 2; g_setupSensX = L""; g_setupSensY = L"";
    g_focusedInput = 1; g_extractedConfig = false;

    // Fast GameUserSettings.ini extraction
    wchar_t appdata[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appdata))) {
        std::wstring p = std::wstring(appdata) + L"\\FortniteGame\\Saved\\Config\\WindowsClient\\GameUserSettings.ini";
        std::ifstream ifs(p.c_str());
        if (ifs.good()) {
            std::string line;
            while (std::getline(ifs, line)) {
                if (line.find("MouseSensitivityX=") != std::string::npos) {
                    std::string val = line.substr(line.find("=") + 1);
                    g_setupSensX = std::wstring(val.begin(), val.end());
                } else if (line.find("MouseSensitivityY=") != std::string::npos) {
                    std::string val = line.substr(line.find("=") + 1);
                    g_setupSensY = std::wstring(val.begin(), val.end());
                }
            }
            if (!g_setupSensX.empty() || !g_setupSensY.empty()) g_extractedConfig = true;
        }
    }

    WNDCLASS wc = { 0 };
    if (!GetClassInfo(hInstance, L"FTSWindowClass", &wc)) {
        wc.lpfnWndProc = FirstTimeSetupProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = L"FTSWindowClass";
        RegisterClass(&wc);
    }

    int W = 500, H = 310;
    HWND hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_APPWINDOW, L"FTSWindowClass", L"BetterAngle Setup", WS_POPUP | WS_SYSMENU,
        GetSystemMetrics(SM_CXSCREEN)/2 - W/2, GetSystemMetrics(SM_CYSCREEN)/2 - H/2, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (!IsWindow(hWnd)) break;
    }
}
