#include "shared/EnhancedLogging.h"
#include <filesystem>
#include <windows.h>
#include <cstdarg>
#include <iostream>
#include "shared/State.h"

std::atomic<LogLevel> g_logLevel(LogLevel::Info);

void InitEnhancedLogging() {
    std::wstring logPath = GetAppRootPath() + L"logs\\debug.log";
    EnhancedLogger::Instance().Initialize(logPath);
}

void ShutdownEnhancedLogging() {
    EnhancedLogger::Instance().Flush();
}

void SetLogLevel(LogLevel level) {
    g_logLevel = level;
}

void LogStartup() {
    LOG_INFO("--- BetterAngle Pro Startup ---");
    LOG_INFO("Build Version: 4.27.116");
    
    // Log basic system info
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    LOG_INFO("Processor Architecture: %u", si.wProcessorArchitecture);
    LOG_INFO("Number of Processors: %u", si.dwNumberOfProcessors);
}

void LogWindowInfo(HWND hwnd) {
    if (!hwnd) return;
    char className[256];
    GetClassNameA(hwnd, className, sizeof(className));
    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));
    LOG_INFO("Window Info: HWND=%p, Class=%s, Title=%s", hwnd, className, title);
}

void LogMessage(LogLevel level, const char* file, int line, const char* format, ...) {
    if (level < g_logLevel) return;

    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    EnhancedLogger::Instance().Log(level, file, line, buffer);
}

// Class Implementation
EnhancedLogger& EnhancedLogger::Instance() {
    static EnhancedLogger instance;
    return instance;
}

void EnhancedLogger::Initialize(const std::wstring& logPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_initialized) return;

    std::filesystem::path path(logPath);
    if (path.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(), ec);
    }

    m_stream.open(logPath, std::ios::out | std::ios::app);
    m_initialized = m_stream.is_open();
}

void EnhancedLogger::Log(LogLevel level, const char* file, int line, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_initialized || !m_stream.is_open()) return;

    // Optional: Only log basename of file
    std::string filename = file;
    size_t lastSlash = filename.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        filename = filename.substr(lastSlash + 1);
    }

    m_stream << "[" << TimestampNow() << "] [" << LevelToString(level) << "] [" 
             << filename << ":" << line << "] " << message << "\n";
    m_stream.flush();
}

void EnhancedLogger::Flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_stream.is_open()) {
        m_stream.flush();
    }
}

EnhancedLogger::~EnhancedLogger() {
    Flush();
}

std::string EnhancedLogger::LevelToString(LogLevel level) const {
    switch (level) {
    case LogLevel::Trace:   return "TRACE";
    case LogLevel::Debug:   return "DEBUG";
    case LogLevel::Info:    return "INFO";
    case LogLevel::Warning: return "WARN";
    case LogLevel::Error:   return "ERROR";
    case LogLevel::Fatal:   return "FATAL";
    default:                return "UNKNOWN";
    }
}

std::string EnhancedLogger::TimestampNow() const {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto time = system_clock::to_time_t(now);
    std::tm tmValue{};
    localtime_s(&tmValue, &time);
    std::stringstream ss;
    ss << std::put_time(&tmValue, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
