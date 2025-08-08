/*
 ▄▄▄▄▄▄▄▄▄▄▄  ▄▄        ▄  ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄  ▄            ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄ 
▐░░░░░░░░░░░▌▐░░▌      ▐░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░▌          ▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌
 ▀▀▀▀█░█▀▀▀▀ ▐░▌░▌     ▐░▌ ▀▀▀▀█░█▀▀▀▀  ▀▀▀▀█░█▀▀▀▀  ▀▀▀▀█░█▀▀▀▀ ▐░█▀▀▀▀▀▀▀█░▌▐░▌           ▀▀▀▀█░█▀▀▀▀  ▀▀▀▀▀▀▀▀▀█░▌▐░█▀▀▀▀▀▀▀▀▀ 
     ▐░▌     ▐░▌▐░▌    ▐░▌     ▐░▌          ▐░▌          ▐░▌     ▐░▌       ▐░▌▐░▌               ▐░▌               ▐░▌▐░▌          
     ▐░▌     ▐░▌ ▐░▌   ▐░▌     ▐░▌          ▐░▌          ▐░▌     ▐░█▄▄▄▄▄▄▄█░▌▐░▌               ▐░▌      ▄▄▄▄▄▄▄▄▄█░▌▐░█▄▄▄▄▄▄▄▄▄ 
     ▐░▌     ▐░▌  ▐░▌  ▐░▌     ▐░▌          ▐░▌          ▐░▌     ▐░░░░░░░░░░░▌▐░▌               ▐░▌     ▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌
     ▐░▌     ▐░▌   ▐░▌ ▐░▌     ▐░▌          ▐░▌          ▐░▌     ▐░█▀▀▀▀▀▀▀█░▌▐░▌               ▐░▌     ▐░█▀▀▀▀▀▀▀▀▀ ▐░█▀▀▀▀▀▀▀▀▀ 
     ▐░▌     ▐░▌    ▐░▌▐░▌     ▐░▌          ▐░▌          ▐░▌     ▐░▌       ▐░▌▐░▌               ▐░▌     ▐░▌          ▐░▌          
 ▄▄▄▄█░█▄▄▄▄ ▐░▌     ▐░▐░▌ ▄▄▄▄█░█▄▄▄▄      ▐░▌      ▄▄▄▄█░█▄▄▄▄ ▐░▌       ▐░▌▐░█▄▄▄▄▄▄▄▄▄  ▄▄▄▄█░█▄▄▄▄ ▐░█▄▄▄▄▄▄▄▄▄ ▐░█▄▄▄▄▄▄▄▄▄ 
▐░░░░░░░░░░░▌▐░▌      ▐░░▌▐░░░░░░░░░░░▌     ▐░▌     ▐░░░░░░░░░░░▌▐░▌       ▐░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌
 ▀▀▀▀▀▀▀▀▀▀▀  ▀        ▀▀  ▀▀▀▀▀▀▀▀▀▀▀       ▀       ▀▀▀▀▀▀▀▀▀▀▀  ▀         ▀  ▀▀▀▀▀▀▀▀▀▀▀  ▀▀▀▀▀▀▀▀▀▀▀  ▀▀▀▀▀▀▀▀▀▀▀  ▀▀▀▀▀▀▀▀▀▀▀                                                                                                                  
*/

#include "../../../src/Logger/Logger.hpp"
#pragma warning (once : 7595) // Warning for potential division by zero

// Global logger instance
MyLogger logger;

// Constructor: Initializes logger with name and output stream
TheLogger::TheLogger(std::string name, std::ostream& out)
    : name{ std::move(name) },
    out{ &out } {
    // Enable colors only for console output
    use_colors = &out == &std::cout;
}

// Convert log level to string with optional color formatting
std::string to_String(Logger level, bool use_colors) {
    return use_colors ? logger.color_levels[level] : logger.levels[level];
}

// Main logging function with thread safety and formatting
void TheLogger::logMessage(Logger level, const std::string& message, const char* file, int line_number) {
    // Check if message meets minimum log level requirement
    if (level < myGlobalLevel)
        return;

    // Format the log message
    const std::string res = this->formatLog(level, message, file, line_number);

    // Thread-safe writing to output stream
    std::lock_guard<std::mutex> lock(this->log_mutex);
    *this->out << res;

    // Reset colors if color output is enabled
    if (use_colors)
        *this->out << logger.RESET;
}

// Thread-safe method to reset logger name (lvalue version)
void TheLogger::resetName(const std::string& name) {
    std::lock_guard<std::mutex> lock(this->log_mutex);
    this->name = name;
}

// Thread-safe method to reset logger name (rvalue version)
void TheLogger::resetName(const std::string&& name) {
    std::lock_guard<std::mutex> lock(this->log_mutex);
    this->name = name;
}

// Thread-safe method to change output stream
void TheLogger::setOutStream(std::ostream& out) {
    std::lock_guard<std::mutex> lock(this->log_mutex);
    this->out = &out;
    // Enable colors only for console output
    use_colors = &out == &std::cout;
}

// Thread-safe method to set format string
void TheLogger::setFormatString(const std::string& format_string) {
    std::lock_guard<std::mutex> lock(this->log_mutex);
    this->format_string = format_string;
}

// Format log message according to format string
std::string TheLogger::formatLog(Logger level, const std::string& message, const char* file, int line_number) {
    std::string result;
    // Parse format string character by character
    for (size_t i = 0; i < this->format_string.size(); ++i) {
        if (this->format_string[i] == '%' && i + 1 < this->format_string.size()) {
            // Handle format specifiers
            switch (this->format_string[++i]) {
            case 'L':  // Log level
                result += to_String(level, use_colors);
                break;
            case 'T':  // Current time
                result += Format::GetCurrentTime();
                break;
            case 'N':  // Logger name
                result += this->name;
                break;
            case 'M':  // Log message
                result += message;
                break;
            case 'ID_T':  // Thread ID
                result += std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
                break;
            case 'P':  // Source file path
                result += file;
                break;
            case 'O':  // Line number
                result += std::to_string(line_number);
                break;
            default:  // Unknown format specifier
                result += '%';
                result += this->format_string[i];
                break;
            }
        }
        else {
            // Copy non-format characters as-is
            result += this->format_string[i];
        }
    }
    return result;
}

// Global logger accessor function (singleton pattern)
TheLogger& MyglobalLogger() {
    static TheLogger MyLogg("Logger");
    return MyLogg;
}