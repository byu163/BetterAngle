#include "shared/State.h"

// Global Definitions
float g_detectionRatio = 0.0f;
bool g_showCrosshair = false;
bool g_isSelectionMode = false;
int g_selectionStep = 0;
COLORREF g_pickedColor = RGB(255, 0, 255);
float g_latestVersion = 4.81f;
std::wstring g_latestName = L"Pending Scan";
RECT g_selectionRect = { 0 };
POINT g_startPoint = { 0 };
std::string g_status = "Connected (v4.6.1 Pro)";
