#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <sstream>
#include <chrono>
#include <iomanip>

enum class LogLevel {
    Info,
    Warning,
    Error,
    Debug
};

class EnhancedLogger {
public:
    static EnhancedLogger& Instance();

    void Initialize(const std::wstring& logPath);
    void Log(LogLevel level, const std::wstring& message);
    void Flush();

private:
    EnhancedLogger() = default;
    ~EnhancedLogger();

    std::wofstream m_stream;
    std::mutex m_mutex;
    bool m_initialized = false;

    std::wstring LevelToString(LogLevel level) const;
    std::wstring TimestampNow() const;
};