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

        // Register "backend" context property BEFORE any load() call
        g_qmlEngine->rootContext()->setContextProperty("backend", g_backend);

        // Register the src/gui/ directory as a QML import path so that
        // Dashboard.qml and FirstTimeSetup.qml are resolvable as types
        // via the qmldir file embedded in the QRC.
        g_qmlEngine->addImportPath("qrc:/src/gui");
    }
}

HWND CreateControlPanel(HINSTANCE hInstance) {
    EnsureEngineInitialized();

    // Track whether main.qml has been loaded by using a static flag,
    // not by inspecting rootObjects().size() (which is unreliable because
    // Splash.qml is already root object #1).
    static bool mainLoaded = false;
    if (!mainLoaded) {
        mainLoaded = true;
        g_qmlEngine->load(QUrl(QStringLiteral("qrc:/src/gui/main.qml")));
        if (g_qmlEngine->rootObjects().size() < 2) {
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