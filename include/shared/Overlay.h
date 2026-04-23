#ifndef OVERLAY_H
#define OVERLAY_H

#include <windows.h>
#include <gdiplus.h>

void InitializeOverlay(HWND hwnd);
void CleanupOverlay();
void DrawOverlay(HWND hwnd, double angle, float detectionRatio, bool showCrosshair);
int GetMonitorCount();
bool GetMonitorRectByIndex(int index, RECT& outRect);

#endif // OVERLAY_H
