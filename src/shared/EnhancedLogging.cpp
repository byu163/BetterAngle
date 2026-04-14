#include "shared/EnhancedLogging.h"

#include <filesystem>
#include <windows.h>

EnhancedLogger& EnhancedLogger::Instance() {
    static EnhancedLogger instance;
    return instance;
}

void EnhancedLogger::Initialize(const std::wstring& logPath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_initialized) {
        return;
    }

    std::filesystem::path path(logPath);
    if (path.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(), ec);
    }

    m_stream.open(logPath, std::ios::out | std::ios::app);
    m_initialized = m_stream.is_open();
}

void EnhancedLogger::Log(LogLevel level, const std::wstring& message) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_initialized || !m_stream.is_open()) {
        return;
    }

    m_stream << L"[" << TimestampNow() << L"] "
             << L"[" << LevelToString(level) << L"] "
             << message << L"\n";
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

std::wstring EnhancedLogger::LevelToString(LogLevel level) const {
    switch (level) {
    case LogLevel::Info:
        return L"INFO";
    case LogLevel::Warning:
        return L"WARNING";
    case LogLevel::Error:
        return L"ERROR";
    case LogLevel::Debug:
        return L"DEBUG";
    default:
        return L"UNKNOWN";
    }
}

std::wstring EnhancedLogger::TimestampNow() const {
    using namespace std::chrono;

    const auto now = system_clock::now();
    const auto time = system_clock::to_time_t(now);

    std::tm tmValue{};
    localtime_s(&tmValue, &time);

    std::wstringstream ss;
    ss << std::put_time(&tmValue, L"%Y-%m-%d %H:%M:%S");
    return ss.str();
}
