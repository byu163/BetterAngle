#ifndef LOGIC_H
#define LOGIC_H

#include <windows.h>

class AngleLogic {
public:
    AngleLogic(double dpi, double sensX);
    void Update(int dx);
    double GetAngle() const;
    long long GetAccumDx() const { return m_accumDx; }
    void SetZero();
    void LoadProfile(int dpi, double sensX, double divingMult);
    void SetDivingState(bool diving);

private:
    double m_dpi;
    double m_sensX;
    double m_divingMult;
    bool m_isDiving;
    
    long long m_accumDx;
    long long m_baseDx;
    double m_baseAngle;
    
    double Norm360(double a) const;
};

#endif // LOGIC_H
