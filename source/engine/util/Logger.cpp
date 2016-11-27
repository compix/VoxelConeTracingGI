#include "Logger.h"

Logger::Logger() {}

void Logger::log(const std::string& msg) noexcept { stream() << msg << std::endl; }
