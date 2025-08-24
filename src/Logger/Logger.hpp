/*
 ▄            ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄
▐░▌          ▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌
▐░▌          ▐░█▀▀▀▀▀▀▀█░▌▐░█▀▀▀▀▀▀▀▀▀ ▐░█▀▀▀▀▀▀▀▀▀ ▐░█▀▀▀▀▀▀▀▀▀ ▐░█▀▀▀▀▀▀▀█░▌
▐░▌          ▐░▌       ▐░▌▐░▌          ▐░▌          ▐░▌          ▐░▌       ▐░▌
▐░▌          ▐░▌       ▐░▌▐░▌ ▄▄▄▄▄▄▄▄ ▐░▌ ▄▄▄▄▄▄▄▄ ▐░█▄▄▄▄▄▄▄▄▄ ▐░█▄▄▄▄▄▄▄█░▌
▐░▌          ▐░▌       ▐░▌▐░▌▐░░░░░░░░▌▐░▌▐░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌
▐░▌          ▐░▌       ▐░▌▐░▌ ▀▀▀▀▀▀█░▌▐░▌ ▀▀▀▀▀▀█░▌▐░█▀▀▀▀▀▀▀▀▀ ▐░█▀▀▀▀█░█▀▀
▐░▌          ▐░▌       ▐░▌▐░▌       ▐░▌▐░▌       ▐░▌▐░▌          ▐░▌     ▐░▌
▐░█▄▄▄▄▄▄▄▄▄ ▐░█▄▄▄▄▄▄▄█░▌▐░█▄▄▄▄▄▄▄█░▌▐░█▄▄▄▄▄▄▄█░▌▐░█▄▄▄▄▄▄▄▄▄ ▐░▌      ▐░▌
▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░▌       ▐░▌
 ▀▀▀▀▀▀▀▀▀▀▀  ▀▀▀▀▀▀▀▀▀▀▀  ▀▀▀▀▀▀▀▀▀▀▀  ▀▀▀▀▀▀▀▀▀▀▀  ▀▀▀▀▀▀▀▀▀▀▀  ▀         ▀

*/

// Structure for time formatting utilities
#pragma once
#include <string>
#include <array> // Аnalogue of an array
#include <mutex> // the streams
#include <chrono>
#include <thread>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

// Structure for time formatting utilities
struct Format {
    // Gets current time in HH:MM:SS format
    static std::string GetCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        std::tm local_tm;
#ifdef _WIN32
        localtime_s(&local_tm, &time);
#else
        localtime_r(&time, &local_tm);
#endif
        ss << std::put_time(&local_tm, "%H:%M:%S");
        return ss.str();
    }
};

// Enum for different logging levels
enum Logger {
    DEBUG,      // Detailed information for debugging
    INFO,       // General information about program execution
    WARNING,    // Potentially harmful situations
    ERROR       // Error events that might still allow the program to continue
};

// Structure containing logger formatting and color settings
struct MyLogger {
    // ANSI color codes for console output
    const std::string RESET = "\033[0m";      // Reset to default color
    const std::string GREEN = "\033[32m";      // Green for Debug
    const std::string BLUE = "\033[35m";       // Blue for Info
    const std::string YELLOW = "\033[33m";     // Yellow for Warning
    const std::string RED = "\033[31m";        // Red for Error

    // Arrays for log level names
    const std::array<std::string, 4> levels = { "DEBUG", "INFO", "WARNING", "ERROR" };
    const std::array<std::string, 4> color_levels = {
        GREEN + "DEBUG" + RESET,
        BLUE + "INFO" + RESET,
        YELLOW + "WARNING" + RESET,
        RED + "ERROR" + RESET
    };
};

// Global log level configuration
#ifdef GLOBAL_LOG_LEVEL
const MyLogger myGlobalLevel = GLOBAL_LOG_LEVEL
#else
const Logger myGlobalLevel = DEBUG;
#endif

// Macro definitions for logging
#define LOG_MESSAGE(logger, level, message) \
    logger.logMessage(level, message, __FILE__, __LINE__);

// Convenience macros for different log levels
#define logDebug(logger, message) logger.logMessage(Logger::DEBUG, message, __FILE__, __LINE__)
#define logInfo(logger, message) logger.logMessage(Logger::INFO, message, __FILE__, __LINE__)
#define logWarning(logger, message) logger.logMessage(Logger::WARNING, message, __FILE__, __LINE__)
#define logError(logger, message) logger.logMessage(Logger::ERROR, message, __FILE__, __LINE__)

// Global logger macros
#define logMessageGlobal(level, message) \
    Logger::globalLogger.logMessage(level, message, __FILE__, __LINE__)
#define logDebugGlobal(message) Logger::globalLogger.logMessage(Logger::DEBUG, message, __FILE__, __LINE__)
#define logInfoGlobal(message) Logger::globalLogger.logMessage(Logger::INFO, message, __FILE__, __LINE__)
#define logWarningGlobal(message) Logger::globalLogger.logMessage(Logger::WARNING, message, __FILE__, __LINE__)
#define logErrorGlobal(message) Logger::globalLogger.logMessage(Logger::ERROR, message, __FILE__, __LINE__)

// String conversion function declaration
std::string to_String(Logger level, bool use_colors);

// Main logger class
class TheLogger {
public:
    // Constructor with name and output stream parameters
    explicit TheLogger(std::string name, std::ostream& out = std::cout);

    // Core logging functionality
    void logMessage(Logger level, const std::string& message, const char* file, int line_number);

    // Configuration methods
    void resetName(const std::string& name);           // Reset logger name (lvalue)
    void resetName(const std::string&& name);          // Reset logger name (rvalue)
    void setOutStream(std::ostream& out);             // Set output stream
    void setFormatString(const std::string& format);   // Set format string

private:
    // Format log message according to template
    std::string formatLog(Logger level, const std::string& message, const char* file, int line_number);

private:
    // Member variables
    bool use_colors = true;                           // Flag for color output
    std::mutex log_mutex;                            // Mutex for thread safety
    std::string name;                                // Logger name
    std::ostream* out;                               // Output stream pointer
    std::string format_string = "%L: %T [%N]: %M\n"; // Default format string
};

// Global logger access functions
TheLogger& MyglobalLogger();
extern TheLogger MyLogg;