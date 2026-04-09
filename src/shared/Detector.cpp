#include "shared/Detector.h"
#include <cmath>
#include <iostream>
#include <vector>

FovDetector::FovDetector()
    : m_hdcScreen(NULL), m_hdcMem(NULL), m_hbm(NULL), m_hOld(NULL), m_curW(0),
      m_curH(0) {
  m_hdcScreen = GetDC(NULL);
  if (!m_hdcScreen) {
    throw std::runtime_error("Failed to get screen DC");
  }
}

FovDetector::~FovDetector() {
  if (m_hdcMem) {
    if (m_hOld) {
      SelectObject(m_hdcMem, m_hOld);
    }
    DeleteDC(m_hdcMem);
  }
  if (m_hbm) {
    DeleteObject(m_hbm);
  }
  if (m_hdcScreen) {
    ReleaseDC(NULL, m_hdcScreen);
  }
}

void FovDetector::EnsureResources(int w, int h) {
  if (w == m_curW && h == m_curH && m_hdcMem)
    return;

  if (m_hdcMem) {
    if (m_hOld) {
      SelectObject(m_hdcMem, m_hOld);
    }
    DeleteDC(m_hdcMem);
  }
  if (m_hbm) {
    DeleteObject(m_hbm);
  }

  m_hdcMem = CreateCompatibleDC(m_hdcScreen);
  if (!m_hdcMem)
    return;

  m_hbm = CreateCompatibleBitmap(m_hdcScreen, w, h);
  if (!m_hbm)
    return;

  m_hOld = SelectObject(m_hdcMem, m_hbm);
  m_curW = w;
  m_curH = h;
}

float FovDetector::Scan(const RoiConfig &cfg) {
  if (cfg.w <= 0 || cfg.h <= 0)
    return 0.0f;

  EnsureResources(cfg.w, cfg.h);
  if (!m_hdcMem || !m_hbm)
    return 0.0f;

  if (!BitBlt(m_hdcMem, 0, 0, cfg.w, cfg.h, m_hdcScreen, cfg.x, cfg.y,
              SRCCOPY)) {
    return 0.0f;
  }

  BITMAPINFO bmi = {0};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = cfg.w;
  bmi.bmiHeader.biHeight = -cfg.h; // Top-down
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  std::vector<DWORD> pixels(cfg.w * cfg.h);
  if (GetDIBits(m_hdcMem, m_hbm, 0, cfg.h, pixels.data(), &bmi,
                DIB_RGB_COLORS) == 0) {
    return 0.0f;
  }

  int match = 0;
  BYTE tr = GetRValue(cfg.target);
  BYTE tg = GetGValue(cfg.target);
  BYTE tb = GetBValue(cfg.target);

  for (DWORD p : pixels) {
    BYTE b = p & 0xFF;
    BYTE g = (p >> 8) & 0xFF;
    BYTE r = (p >> 16) & 0xFF;

    if (abs(r - tr) <= cfg.tolerance && abs(g - tg) <= cfg.tolerance &&
        abs(b - tb) <= cfg.tolerance) {
      match++;
    }
  }

  return (float)match / (float)(cfg.w * cfg.h);
}
