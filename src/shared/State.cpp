#include "shared/State.h"

// Global Definitions
float g_detectionRatio = 0.0f;
bool g_showCrosshair = false;
bool g_isSelectionMode = false;
RECT g_selectionRect = { 0 };
POINT g_startPoint = { 0 };
std::string g_status = "Connected (v4.6.1 Pro)";
