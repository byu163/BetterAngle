#include "shared/ControlPanel.h"
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QGuiApplication>
#include "shared/BetterAngleBackend.h"

QQmlApplicationEngine* g_qmlEngine = nullptr;
BetterAngleBackend* g_backend = nullptr;

void EnsureEngineInitialized() {
    if (!g_qmlEngine) {
        g_qmlEngine = new QQmlApplicationEngine();
        g_backend = new BetterAngleBackend(g_qmlEngine);
        g_qmlEngine->rootContext()->setContextProperty("backend", g_backend);
    }
}

HWND CreateControlPanel(HINSTANCE hInstance) {
    EnsureEngineInitialized();
    
    // Always attempt to load main.qml if it's not already loaded
    if (g_qmlEngine->rootObjects().isEmpty() || g_qmlEngine->rootObjects().size() < 2) {
        g_qmlEngine->load(QUrl(QStringLiteral("qrc:/src/gui/main.qml")));
        if (g_qmlEngine->rootObjects().isEmpty()) {
            MessageBoxW(NULL, L"CRITICAL: Failed to load UI (main.qml). The application will now close.", L"BetterAngle Error", MB_OK | MB_ICONERROR);
            exit(1);
        }
    }
    
    return (HWND)1; 
}

void ShowSplashScreen() {
    EnsureEngineInitialized();
    g_qmlEngine->load(QUrl(QStringLiteral("qrc:/src/gui/Splash.qml")));
}

void ShowControlPanel() {
    if (g_backend) {
        g_backend->requestToggleControlPanel();
    }
}