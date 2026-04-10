#include "shared/Logic.h"
#include <cmath>
#include "shared/State.h"

bool IsFortniteFocused() {
    HWND hForeground = GetForegroundWindow();
    if (!hForeground) return false;

    wchar_t className[256];
    GetClassNameW(hForeground, className, 256);
    std::wstring cls(className);

    // Fortnite uses "UnrealWindow" or "Fortnite"
    if (cls == L"UnrealWindow" || cls == L"Fortnite") {
        return true;
    }

    wchar_t title[256];
    GetWindowTextW(hForeground, title, 256);
    std::wstring wTitle(title);

    return (wTitle.find(L"Fortnite") != std::wstring::npos);
}

AngleLogic::AngleLogic(double dpi, double sens)
    : m_dpi(dpi), m_sens(sens), m_divingMult(1.22), m_isDiving(false), m_accumDx(0), m_baseDx(0), m_baseAngle(0.0) {}

void AngleLogic::Update(int dx) {
    if (!g_debugMode) {
        if (!IsFortniteFocused() || g_isCursorVisible) return;
    }
    m_accumDx += dx;
}

double AngleLogic::GetAngle() const {
    double normalScale = 0.5573 / (m_dpi * m_sens);
    double scale = m_isDiving ? (normalScale * m_divingMult) : normalScale;
    
    if (scale == 0.0) scale = 0.0031415; // Bullet-proof fallback
    double angle = Norm360(m_baseAngle + (m_accumDx - m_baseDx) * scale);
    
    g_currentAngle = (float)angle;
    
    return angle;
}

void AngleLogic::SetZero() {
    m_baseAngle = 0.0;
    m_baseDx = m_accumDx;
}

void AngleLogic::LoadProfile(int dpi, double sens, double divingMult) {
    if (m_dpi == dpi && m_sens == sens && m_divingMult == divingMult) return;
    
    // Prevent the angle jump when swapping Sensitivity mid-air
    m_baseAngle = GetAngle();
    m_baseDx = m_accumDx;
    
    m_dpi = dpi;
    m_sens = sens;
    m_divingMult = divingMult;
}

void AngleLogic::SetDivingState(bool diving) {
    if (m_isDiving == diving) return;
    
    // Keep angle stationary during state transition
    m_baseAngle = GetAngle();
    m_baseDx = m_accumDx;
    
    m_isDiving = diving;
}

double AngleLogic::Norm360(double a) const {
    double res = fmod(a, 360.0);
    if (res < 0) res += 360.0;
    return res;
}
