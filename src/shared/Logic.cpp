#include "shared/Logic.h"
#include <cmath>
#include "shared/State.h"
#include <shlobj.h>
#include <fstream>
#include <string>
#include <algorithm>

double FetchFortniteSensitivity() {
    double fetchedSens = 0.05;
    wchar_t appdata[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appdata))) {
        std::wstring folders[] = { L"\\WindowsClient\\", L"\\WindowsNoEditor\\" };
        std::wstring basePath = std::wstring(appdata) + L"\\FortniteGame\\Saved\\Config";
        
        for (const auto& f : folders) {
            std::wstring pPath = basePath + f + L"GameUserSettings.ini";
            std::ifstream ifs(pPath.c_str());
            if (ifs.good()) {
                std::string line;
                bool found = false;
                while (std::getline(ifs, line)) {
                    // Search for both modern and legacy sensitivity keys
                    if (line.find("MouseSensitivityX=") != std::string::npos || 
                        line.find("MouseX=") != std::string::npos) {
                        size_t eq = line.find("=");
                        if (eq != std::string::npos) {
                            try {
                                fetchedSens = std::stod(line.substr(eq + 1));
                                found = true;
                                break;
                            } catch (...) {}
                        }
                    }
                }
                if (found) break; 
            }
        }
    }
    return (std::max)(fetchedSens, 0.0001);
}

bool IsFortniteFocused() {
    if (g_forceDetection) return true; // Safety fallback

    HWND hForeground = GetForegroundWindow();
    if (!hForeground) return false;

    wchar_t className[256];
    GetClassNameW(hForeground, className, 256);
    std::wstring cls(className);

    return (cls == L"UnrealWindow");
}

AngleLogic::AngleLogic(double sensX)
    : m_sensX(sensX), m_isDiving(false), m_accumDx(0), m_baseDx(0), m_baseAngle(0.0) {}

void AngleLogic::Update(int dx) {
    if (!g_debugMode) {
        if (!g_fortniteFocusedCache || g_isCursorVisible) return;
    }
    m_accumDx.fetch_add(dx, std::memory_order_relaxed);
}

double AngleLogic::GetAngle() const {
    double currentSens = m_sensX.load(std::memory_order_relaxed);
    double normalScale = 0.05555 * currentSens;
    double scale = m_isDiving.load(std::memory_order_relaxed) ? (normalScale * 1.0916) : normalScale;
    
    if (scale == 0.0) scale = 0.0031415; 
    
    long long accum = m_accumDx.load(std::memory_order_relaxed);
    long long base  = m_baseDx.load(std::memory_order_relaxed);
    double angle = Norm360(m_baseAngle.load(std::memory_order_relaxed) + (accum - base) * scale);
    
    g_currentAngle = (float)angle;
    
    return angle;
}

void AngleLogic::SetZero() {
    m_baseAngle.store(0.0);
    m_baseDx.store(m_accumDx.load());
}

void AngleLogic::LoadProfile(double sensX) {
    if (m_sensX.load() == sensX) return;
    
    // Prevent the angle jump when swapping Sensitivity mid-air
    m_baseAngle.store(GetAngle());
    m_baseDx.store(m_accumDx.load());
    
    m_sensX.store(sensX);
}

void AngleLogic::SetDivingState(bool diving) {
    if (m_isDiving.load() == diving) return;
    
    // Keep angle stationary during state transition
    m_baseAngle.store(GetAngle());
    m_baseDx.store(m_accumDx.load());
    
    m_isDiving.store(diving);
}

double AngleLogic::Norm360(double a) const {
    double res = fmod(a, 360.0);
    if (res < 0) res += 360.0;
    return res;
}
