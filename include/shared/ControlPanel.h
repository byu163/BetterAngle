#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <windows.h>
#include <string>

HWND CreateControlPanel(HINSTANCE hInst);
LRESULT CALLBACK ControlPanelWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#endif
