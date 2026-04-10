#include "shared/Logic.h"
#include "shared/State.h"
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <shlobj.h>

double FetchFortniteSensitivity() {
    wchar_t appdata[MAX_PATH];
    if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appdata)))
        return -1.0;

    std::wstring pPath = std::wstring(appdata) +
        L"\\FortniteGame\\Saved\\Config\\WindowsClient\\GameUserSettings.ini";

    std::ifstream ifs(pPath.c_str());
    if (!ifs.is_open() || !ifs.good())
        return -1.0; 

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.find("MouseSensitivityX=") != std::string::npos) {
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                try {
                    double val = std::stod(line.substr(eqPos + 1));
                    return (std::max)(val, 0.0001); 
                } catch (...) { }
            }
            break;
        }
    }
    return -1.0; 
}

bool IsFortniteFocused() {
    HWND fg = GetForegroundWindow();
    if (!fg) return false;
    wchar_t cls[256] = { 0 };
    GetClassNameW(fg, cls, 256);
    if (wcscmp(cls, L"UnrealWindow") != 0) return false;
    wchar_t title[256] = { 0 };
    GetWindowTextW(fg, title, 256);
    return wcsstr(title, L"Fortnite") != nullptr;
}

AngleLogic::AngleLogic(double sensX) 
    : m_sensX(sensX), m_isDiving(false), m_accumDx(0), m_baseDx(0), m_baseAngle(0.0) {}

void AngleLogic::Update(int dx) {
    m_accumDx += dx;
}

double AngleLogic::GetAngle() const {
    double currentSens = m_sensX.load();
    // 0.00555555 deg/tick * sens is the true Fortnite pitch/yaw scale
    double scale = 0.00555555 * currentSens;
    if (m_isDiving.load()) {
        scale *= 1.0916; // Diving multiplier
    }

    double delta = (double)(m_accumDx.load() - m_baseDx.load());
    return m_baseAngle.load() + (delta * scale);
}

void AngleLogic::SetZero() {
    m_accumDx = 0;
    m_baseDx = 0;
    m_baseAngle = 0.0;
}

void AngleLogic::LoadProfile(double sensX) {
    // Before updating sensitivity, bake in the current angle to prevent jumping
    m_baseAngle = GetAngle();
    m_baseDx = m_accumDx.load();
    m_sensX = sensX;
}

void AngleLogic::SetDivingState(bool diving) {
    if (diving == m_isDiving.load()) return;

    // Bake in the current angle before switching scales
    m_baseAngle = GetAngle();
    m_baseDx = m_accumDx.load();
    m_isDiving = diving;
}

double AngleLogic::Norm360(double a) const {
    while (a >= 360.0) a -= 360.0;
    while (a < 0.0) a += 360.0;
    return a;
}
