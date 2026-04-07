#ifndef UPDATER_H
#define UPDATER_H

#include <windows.h>
#include <string>

bool CheckForUpdates();
void StartUpdate();
bool DownloadUpdate(const std::wstring& url, const std::wstring& dest);
void ApplyUpdateAndRestart();

#endif // UPDATER_H
