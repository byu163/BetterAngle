#include "shared/BetterAngleBackend.h"
#include "shared/Logic.h"
#include "shared/Profile.h"
#include "shared/State.h"
#include "shared/Updater.h"
#include <QGuiApplication>
#include <QTimer>
#include <shlobj.h>
#include <thread>
#include <windows.h>

extern std::vector<Profile> g_allProfiles;
extern int g_selectedProfileIdx;
extern AngleLogic g_logic;

BetterAngleBackend::BetterAngleBackend(QObject *parent) : QObject(parent) {
  // Emit profileChanged once the Qt event loop starts so QML fields
  // refresh with whatever sensitivityX is in g_allProfiles (set by setup or
  // loaded from disk).
  QTimer::singleShot(0, this, [this]() { emit profileChanged(); });

  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, [this]() {
    static bool lastChecking = false;
    static bool lastDownloading = false;
    static bool lastComplete = false;
    static bool lastChecked = false;
    if (g_hasCheckedForUpdates != lastChecked) {
      lastChecked = g_hasCheckedForUpdates;
      emit updateStatusChanged();
    }

    if (g_isCheckingForUpdates != lastChecking) {
      lastChecking = g_isCheckingForUpdates;
      emit updateStatusChanged();
    }
    if (g_isDownloadingUpdate != lastDownloading) {
      lastDownloading = g_isDownloadingUpdate;
      emit updateStatusChanged();
    }
    if (g_downloadComplete != lastComplete) {
      lastComplete = g_downloadComplete;
      emit updateStatusChanged();
    }
  });
  timer->start(100);

  // Auto-check for updates on startup
  QTimer::singleShot(2000, this, [this]() {
    if (!g_hasCheckedForUpdates && !g_isCheckingForUpdates) {
      checkForUpdates();
    }
  });

  // Force show Dashboard shortly after startup
  QTimer::singleShot(500, this, [this]() { requestShowControlPanel(); });
}

double BetterAngleBackend::sensX() const {
  if (g_allProfiles.empty())
    return 1.0;
  return g_allProfiles[g_selectedProfileIdx].sensitivityX;
}
void BetterAngleBackend::setSensX(double v) {
  if (g_allProfiles.empty())
    return;
  Profile &p = g_allProfiles[g_selectedProfileIdx];
  p.sensitivityX = (std::max)(0.001, v);
  g_logic.LoadProfile(p.sensitivityX);
  p.Save(GetProfilesPath() + p.name + L".json");
  SaveSettings();
  emit profileChanged();
}

double BetterAngleBackend::sensY() const {
  if (g_allProfiles.empty())
    return 1.0;
  return g_allProfiles[g_selectedProfileIdx].sensitivityY;
}
void BetterAngleBackend::setSensY(double v) {
  if (g_allProfiles.empty())
    return;
  Profile &p = g_allProfiles[g_selectedProfileIdx];
  p.sensitivityY = (std::max)(0.001, v);
  p.Save(GetProfilesPath() + p.name + L".json");
  SaveSettings();
  emit profileChanged();
}

int BetterAngleBackend::tolerance() const {
  if (g_allProfiles.empty())
    return 2;
  return g_allProfiles[g_selectedProfileIdx].tolerance;
}
void BetterAngleBackend::setTolerance(int v) {
  if (g_allProfiles.empty())
    return;
  Profile &p = g_allProfiles[g_selectedProfileIdx];
  p.tolerance = v;
  p.Save(GetProfilesPath() + p.name + L".json");
  SaveSettings();
  emit profileChanged();
}

bool BetterAngleBackend::crosshairOn() const { return g_showCrosshair; }
void BetterAngleBackend::setCrosshairOn(bool v) {
  g_showCrosshair = v;
  SaveSettings();
  emit crosshairChanged();
}

float BetterAngleBackend::crossThickness() const { return g_crossThickness; }
void BetterAngleBackend::setCrossThickness(float v) {
  g_crossThickness = v;
  SaveSettings();
  emit crosshairChanged();
}

float BetterAngleBackend::crossOffsetX() const { return g_crossOffsetX; }
void BetterAngleBackend::setCrossOffsetX(float v) {
  g_crossOffsetX = v;
  SaveSettings();
  emit crosshairChanged();
}

float BetterAngleBackend::crossOffsetY() const { return g_crossOffsetY; }
void BetterAngleBackend::setCrossOffsetY(float v) {
  g_crossOffsetY = v;
  SaveSettings();
  emit crosshairChanged();
}

bool BetterAngleBackend::crossPulse() const { return g_crossPulse; }
void BetterAngleBackend::setCrossPulse(bool v) {
  g_crossPulse = v;
  SaveSettings();
  emit crosshairChanged();
}

QColor BetterAngleBackend::crossColor() const {
  return QColor(GetRValue(g_crossColor), GetGValue(g_crossColor),
                GetBValue(g_crossColor));
}
void BetterAngleBackend::setCrossColor(const QColor &c) {
  g_crossColor = RGB(c.red(), c.green(), c.blue());
  SaveSettings();
  emit crosshairChanged();
}

bool BetterAngleBackend::debugMode() const { return g_debugMode; }
void BetterAngleBackend::setDebugMode(bool v) {
  g_debugMode = v;
  SaveSettings();
  emit debugChanged();
}

bool BetterAngleBackend::forceDiving() const { return g_forceDiving; }
void BetterAngleBackend::setForceDiving(bool v) {
  g_forceDiving = v;
  SaveSettings();
  emit debugChanged();
}

bool BetterAngleBackend::forceDetection() const { return g_forceDetection; }
void BetterAngleBackend::setForceDetection(bool v) {
  g_forceDetection = v;
  SaveSettings();
  emit debugChanged();
}

float BetterAngleBackend::glideThreshold() const { return g_glideThreshold; }
void BetterAngleBackend::setGlideThreshold(float v) {
  g_glideThreshold = v;
  SaveSettings();
  emit debugChanged();
}

float BetterAngleBackend::freefallThreshold() const {
  return g_freefallThreshold;
}
void BetterAngleBackend::setFreefallThreshold(float v) {
  g_freefallThreshold = v;
  SaveSettings();
  emit debugChanged();
}

QString BetterAngleBackend::versionStr() const {
  return QString::fromLatin1(VERSION_STR);
}
QString BetterAngleBackend::latestVersion() const {
  return QString::fromStdString(g_latestVersionOnline);
}
bool BetterAngleBackend::updateAvailable() const { return g_updateAvailable; }
bool BetterAngleBackend::isDownloading() const { return g_isDownloadingUpdate; }
bool BetterAngleBackend::isCheckingForUpdates() const {
  return g_isCheckingForUpdates;
}
bool BetterAngleBackend::downloadComplete() const { return g_downloadComplete; }
bool BetterAngleBackend::hasCheckedForUpdates() const {
  return g_hasCheckedForUpdates;
}
QString BetterAngleBackend::updateHistory() const {
  return QString::fromStdString(g_updateHistory);
}

QString BetterAngleBackend::updateStatus() const {
  if (g_isDownloadingUpdate)
    return "Downloading update...";
  if (g_downloadComplete)
    return "Download complete! Restart to apply.";
  if (g_isCheckingForUpdates)
    return "Checking for updates...";
  if (g_hasCheckedForUpdates) {
    if (g_updateAvailable)
      return "New update available!";
    return "Application is up to date.";
  }
  return "";
}

void BetterAngleBackend::terminateApp() {
  SaveSettings();
  if (g_hHUD) {
    PostMessage(g_hHUD, WM_CLOSE, 0, 0);
  }
  // Also terminate the Qt loop immediately to ensure everything shuts down
  QCoreApplication::quit();
}
void BetterAngleBackend::checkForUpdates() {
  g_hasCheckedForUpdates = false;
  g_updateAvailable = false;
  emit updateStatusChanged();
  std::thread([]() { CheckForUpdates(); }).detach();
}

void BetterAngleBackend::downloadUpdate() {
  if (g_downloadComplete) {
    ApplyUpdateAndRestart();
    return;
  }
  UpdateApp();
}

void BetterAngleBackend::saveThresholds() { SaveSettings(); }

void BetterAngleBackend::requestShowControlPanel() {
  emit showControlPanelRequested();
}

QStringList BetterAngleBackend::crosshairPresetNames() const {
  QStringList list;
  if (g_allProfiles.empty())
    return list;
  for (const auto &cp : g_allProfiles[g_selectedProfileIdx].crosshairPresets) {
    list << QString::fromStdWString(cp.name) + QString(" (%1, %2)")
                                                   .arg(cp.offsetX, 0, 'f', 1)
                                                   .arg(cp.offsetY, 0, 'f', 1);
  }
  return list;
}

void BetterAngleBackend::saveCrosshairPreset(const QString &name) {
  if (g_allProfiles.empty())
    return;
  Profile &p = g_allProfiles[g_selectedProfileIdx];
  CrosshairPreset cp;
  cp.name = name.toStdWString();
  cp.offsetX = g_crossOffsetX;
  cp.offsetY = g_crossOffsetY;
  cp.angle = g_crossAngle;
  // Replace existing with same name, otherwise append
  for (auto &existing : p.crosshairPresets) {
    if (existing.name == cp.name) {
      existing = cp;
      p.Save(GetProfilesPath() + p.name + L".json");
      emit crosshairPresetsChanged();
      return;
    }
  }
  p.crosshairPresets.push_back(cp);
  p.Save(GetProfilesPath() + p.name + L".json");

  emit crosshairPresetsChanged();
}

void BetterAngleBackend::loadCrosshairPreset(int index) {
  if (g_allProfiles.empty())
    return;
  Profile &p = g_allProfiles[g_selectedProfileIdx];
  if (index < 0 || index >= (int)p.crosshairPresets.size())
    return;
  const CrosshairPreset &cp = p.crosshairPresets[index];
  g_crossOffsetX = cp.offsetX;
  g_crossOffsetY = cp.offsetY;
  g_crossAngle = cp.angle;
  p.crossOffsetX = cp.offsetX;
  p.crossOffsetY = cp.offsetY;
  p.crossAngle = cp.angle;
  p.Save(GetProfilesPath() + p.name + L".json");
  emit crosshairChanged();
}

void BetterAngleBackend::deleteCrosshairPreset(int index) {
  if (g_allProfiles.empty())
    return;
  Profile &p = g_allProfiles[g_selectedProfileIdx];
  if (index < 0 || index >= (int)p.crosshairPresets.size())
    return;
  p.crosshairPresets.erase(p.crosshairPresets.begin() + index);
  p.Save(GetProfilesPath() + p.name + L".json");
  emit crosshairPresetsChanged();
}

// --- Hotkey Helpers ---
static QString s_hotkeyStatus =
    "Click a hotkey field, then press Ctrl / Shift / Alt plus a key.";

static bool isModifierVk(UINT vk) {
  return vk == 0 || vk == VK_CONTROL || vk == VK_SHIFT || vk == VK_MENU ||
         vk == VK_LCONTROL || vk == VK_RCONTROL || vk == VK_LSHIFT ||
         vk == VK_RSHIFT || vk == VK_LMENU || vk == VK_RMENU;
}

static UINT stringToVk(const QString &s) {
  QString upper = s.toUpper().trimmed();
  if (upper.isEmpty())
    return 0;

  if (upper.length() == 1) {
    char c = upper[0].toLatin1();
    if (c >= 'A' && c <= 'Z')
      return (UINT)c;
    if (c >= '0' && c <= '9')
      return (UINT)c;
  }

  if (upper == "F1")
    return VK_F1;
  if (upper == "F2")
    return VK_F2;
  if (upper == "F3")
    return VK_F3;
  if (upper == "F4")
    return VK_F4;
  if (upper == "F5")
    return VK_F5;
  if (upper == "F6")
    return VK_F6;
  if (upper == "F7")
    return VK_F7;
  if (upper == "F8")
    return VK_F8;
  if (upper == "F9")
    return VK_F9;
  if (upper == "F10")
    return VK_F10;
  if (upper == "F11")
    return VK_F11;
  if (upper == "F12")
    return VK_F12;
  if (upper == "TAB")
    return VK_TAB;
  if (upper == "SPACE")
    return VK_SPACE;
  if (upper == "ESC" || upper == "ESCAPE")
    return VK_ESCAPE;
  if (upper == "ENTER" || upper == "RETURN")
    return VK_RETURN;
  if (upper == "UP")
    return VK_UP;
  if (upper == "DOWN")
    return VK_DOWN;
  if (upper == "LEFT")
    return VK_LEFT;
  if (upper == "RIGHT")
    return VK_RIGHT;
  if (upper == "HOME")
    return VK_HOME;
  if (upper == "END")
    return VK_END;
  if (upper == "PGUP" || upper == "PAGEUP")
    return VK_PRIOR;
  if (upper == "PGDN" || upper == "PAGEDOWN")
    return VK_NEXT;
  if (upper == "INS" || upper == "INSERT")
    return VK_INSERT;
  if (upper == "DEL" || upper == "DELETE")
    return VK_DELETE;
  if (upper == "PLUS")
    return VK_OEM_PLUS;
  if (upper == "MINUS")
    return VK_OEM_MINUS;
  return 0;
}

static QString vkToString(UINT vk) {
  if (vk >= 'A' && vk <= 'Z')
    return QString((char)vk);
  if (vk >= '0' && vk <= '9')
    return QString((char)vk);
  if (vk == VK_F1)
    return "F1";
  if (vk == VK_F2)
    return "F2";
  if (vk == VK_F3)
    return "F3";
  if (vk == VK_F4)
    return "F4";
  if (vk == VK_F5)
    return "F5";
  if (vk == VK_F6)
    return "F6";
  if (vk == VK_F7)
    return "F7";
  if (vk == VK_F8)
    return "F8";
  if (vk == VK_F9)
    return "F9";
  if (vk == VK_F10)
    return "F10";
  if (vk == VK_F11)
    return "F11";
  if (vk == VK_F12)
    return "F12";
  if (vk == VK_TAB)
    return "Tab";
  if (vk == VK_SPACE)
    return "Space";
  if (vk == VK_ESCAPE)
    return "Esc";
  if (vk == VK_RETURN)
    return "Enter";
  if (vk == VK_UP)
    return "Up";
  if (vk == VK_DOWN)
    return "Down";
  if (vk == VK_LEFT)
    return "Left";
  if (vk == VK_RIGHT)
    return "Right";
  if (vk == VK_HOME)
    return "Home";
  if (vk == VK_END)
    return "End";
  if (vk == VK_PRIOR)
    return "PageUp";
  if (vk == VK_NEXT)
    return "PageDown";
  if (vk == VK_INSERT)
    return "Insert";
  if (vk == VK_DELETE)
    return "Delete";
  if (vk == VK_OEM_PLUS)
    return "Plus";
  if (vk == VK_OEM_MINUS)
    return "Minus";
  return "";
}

static QString fullKeyToString(UINT mod, UINT vk) {
  QStringList parts;
  if (mod & MOD_CONTROL)
    parts << "Ctrl";
  if (mod & MOD_SHIFT)
    parts << "Shift";
  if (mod & MOD_ALT)
    parts << "Alt";
  if (vk != 0)
    parts << vkToString(vk);
  return parts.join(" + ");
}

static bool parseFullKey(const QString &s, UINT &outMod, UINT &outKey) {
  outMod = 0;
  outKey = 0;

  QStringList tokens = s.split('+', Qt::SkipEmptyParts);
  QString keyPart;
  for (const QString &token : tokens) {
    QString upper = token.trimmed().toUpper();
    if (upper == "CTRL" || upper == "CONTROL")
      outMod |= MOD_CONTROL;
    else if (upper == "SHIFT")
      outMod |= MOD_SHIFT;
    else if (upper == "ALT")
      outMod |= MOD_ALT;
    else
      keyPart = token.trimmed();
  }

  if (keyPart.isEmpty())
    keyPart = s.trimmed();

  outKey = stringToVk(keyPart);
  return !isModifierVk(outKey);
}

static UINT qtKeyToVk(int key) {
  if (key >= Qt::Key_A && key <= Qt::Key_Z)
    return 'A' + (UINT)(key - Qt::Key_A);
  if (key >= Qt::Key_0 && key <= Qt::Key_9)
    return '0' + (UINT)(key - Qt::Key_0);
  if (key >= Qt::Key_F1 && key <= Qt::Key_F12)
    return VK_F1 + (UINT)(key - Qt::Key_F1);

  switch (key) {
  case Qt::Key_Tab:
    return VK_TAB;
  case Qt::Key_Space:
    return VK_SPACE;
  case Qt::Key_Escape:
    return VK_ESCAPE;
  case Qt::Key_Return:
  case Qt::Key_Enter:
    return VK_RETURN;
  case Qt::Key_Up:
    return VK_UP;
  case Qt::Key_Down:
    return VK_DOWN;
  case Qt::Key_Left:
    return VK_LEFT;
  case Qt::Key_Right:
    return VK_RIGHT;
  case Qt::Key_Home:
    return VK_HOME;
  case Qt::Key_End:
    return VK_END;
  case Qt::Key_PageUp:
    return VK_PRIOR;
  case Qt::Key_PageDown:
    return VK_NEXT;
  case Qt::Key_Insert:
    return VK_INSERT;
  case Qt::Key_Delete:
    return VK_DELETE;
  case Qt::Key_Equal:
  case Qt::Key_Plus:
    return VK_OEM_PLUS;
  case Qt::Key_Minus:
  case Qt::Key_Underscore:
    return VK_OEM_MINUS;
  default:
    return 0;
  }
}

static UINT qtModifiersToNative(int modifiers) {
  UINT native = 0;
  if (modifiers & Qt::ControlModifier)
    native |= MOD_CONTROL;
  if (modifiers & Qt::ShiftModifier)
    native |= MOD_SHIFT;
  if (modifiers & Qt::AltModifier)
    native |= MOD_ALT;
  return native;
}

static Keybinds *activeKeybinds() {
  if (g_allProfiles.empty() || g_selectedProfileIdx < 0 ||
      g_selectedProfileIdx >= (int)g_allProfiles.size())
    return nullptr;
  return &g_allProfiles[g_selectedProfileIdx].keybinds;
}

static QString actionLabel(const QString &action) {
  if (action == "toggle")
    return "Toggle Dashboard";
  if (action == "roi")
    return "Selection Overlay";
  if (action == "cross")
    return "Toggle Crosshair";
  if (action == "zero")
    return "Zero Counter";
  if (action == "debug")
    return "Debug Overlay";
  return "Unknown Action";
}

static bool assignActionKeybind(const QString &action, UINT mod, UINT key) {
  Keybinds *k = activeKeybinds();
  if (!k)
    return false;

  if (action == "toggle") {
    k->toggleMod = mod;
    k->toggleKey = key;
    return true;
  }
  if (action == "roi") {
    k->roiMod = mod;
    k->roiKey = key;
    return true;
  }
  if (action == "cross") {
    k->crossMod = mod;
    k->crossKey = key;
    return true;
  }
  if (action == "zero") {
    k->zeroMod = mod;
    k->zeroKey = key;
    return true;
  }
  if (action == "debug") {
    k->debugMod = mod;
    k->debugKey = key;
    return true;
  }
  return false;
}

QString BetterAngleBackend::keyToggle() const {
  if (g_allProfiles.empty())
    return "Ctrl + U";
  const auto &k = g_allProfiles[g_selectedProfileIdx].keybinds;
  return fullKeyToString(k.toggleMod, k.toggleKey);
}

void BetterAngleBackend::setKeyToggle(const QString &s) {
  Keybinds *k = activeKeybinds();
  UINT mod = 0, key = 0;
  if (!k || !parseFullKey(s, mod, key))
    return;
  k->toggleMod = mod;
  k->toggleKey = key;
  s_hotkeyStatus =
      "Pending save: Toggle Dashboard = " + fullKeyToString(mod, key);
  emit hotkeysChanged();
}

QString BetterAngleBackend::keyRoi() const {
  if (g_allProfiles.empty())
    return "Ctrl + R";
  const auto &k = g_allProfiles[g_selectedProfileIdx].keybinds;
  return fullKeyToString(k.roiMod, k.roiKey);
}

void BetterAngleBackend::setKeyRoi(const QString &s) {
  Keybinds *k = activeKeybinds();
  UINT mod = 0, key = 0;
  if (!k || !parseFullKey(s, mod, key))
    return;
  k->roiMod = mod;
  k->roiKey = key;
  s_hotkeyStatus =
      "Pending save: Selection Overlay = " + fullKeyToString(mod, key);
  emit hotkeysChanged();
}

QString BetterAngleBackend::keyCross() const {
  if (g_allProfiles.empty())
    return "F10";
  const auto &k = g_allProfiles[g_selectedProfileIdx].keybinds;
  return fullKeyToString(k.crossMod, k.crossKey);
}

void BetterAngleBackend::setKeyCross(const QString &s) {
  Keybinds *k = activeKeybinds();
  UINT mod = 0, key = 0;
  if (!k || !parseFullKey(s, mod, key))
    return;
  k->crossMod = mod;
  k->crossKey = key;
  s_hotkeyStatus =
      "Pending save: Toggle Crosshair = " + fullKeyToString(mod, key);
  emit hotkeysChanged();
}

QString BetterAngleBackend::keyZero() const {
  if (g_allProfiles.empty())
    return "Ctrl + G";
  const auto &k = g_allProfiles[g_selectedProfileIdx].keybinds;
  return fullKeyToString(k.zeroMod, k.zeroKey);
}

void BetterAngleBackend::setKeyZero(const QString &s) {
  Keybinds *k = activeKeybinds();
  UINT mod = 0, key = 0;
  if (!k || !parseFullKey(s, mod, key))
    return;
  k->zeroMod = mod;
  k->zeroKey = key;
  s_hotkeyStatus = "Pending save: Zero Counter = " + fullKeyToString(mod, key);
  emit hotkeysChanged();
}

QString BetterAngleBackend::keyDebug() const {
  if (g_allProfiles.empty())
    return "Ctrl + 9";
  const auto &k = g_allProfiles[g_selectedProfileIdx].keybinds;
  return fullKeyToString(k.debugMod, k.debugKey);
}

void BetterAngleBackend::setKeyDebug(const QString &s) {
  Keybinds *k = activeKeybinds();
  UINT mod = 0, key = 0;
  if (!k || !parseFullKey(s, mod, key))
    return;
  k->debugMod = mod;
  k->debugKey = key;
  s_hotkeyStatus = "Pending save: Debug Overlay = " + fullKeyToString(mod, key);
  emit hotkeysChanged();
}

QString BetterAngleBackend::hotkeyStatus() const { return s_hotkeyStatus; }

bool BetterAngleBackend::setCapturedKeybind(const QString &action, int key,
                                            int modifiers) {
  if (!activeKeybinds()) {
    s_hotkeyStatus = "No active profile is loaded.";
    emit hotkeysChanged();
    return false;
  }

  UINT nativeKey = qtKeyToVk(key);
  UINT nativeMod = qtModifiersToNative(modifiers);
  if (isModifierVk(nativeKey)) {
    s_hotkeyStatus =
        "Press a non-modifier key with optional Ctrl / Shift / Alt.";
    emit hotkeysChanged();
    return false;
  }

  if (!assignActionKeybind(action, nativeMod, nativeKey)) {
    s_hotkeyStatus = "Unknown hotkey action requested.";
    emit hotkeysChanged();
    return false;
  }

  s_hotkeyStatus = "Pending save: " + actionLabel(action) + " = " +
                   fullKeyToString(nativeMod, nativeKey);
  emit hotkeysChanged();
  return true;
}

void BetterAngleBackend::saveKeybinds() {
  if (g_allProfiles.empty()) {
    s_hotkeyStatus = "No active profile is loaded.";
    emit hotkeysChanged();
    return;
  }

  Profile &p = g_allProfiles[g_selectedProfileIdx];
  p.Save(GetProfilesPath() + p.name + L".json");
  bool applied = RefreshHotkeys(g_hHUD);
  s_hotkeyStatus =
      applied
          ? "Hotkeys saved and applied successfully."
          : "Hotkey registration failed. Use unique combos with one main key.";
  emit hotkeysChanged();
}
