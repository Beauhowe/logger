#include "logger/Logger.h"

#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <utility>

namespace logger {
namespace {

std::tm localTime(std::time_t time) {
  std::tm tmValue{};
#if defined(_WIN32)
  localtime_s(&tmValue, &time);
#else
  localtime_r(&time, &tmValue);
#endif
  return tmValue;
}

std::string sanitizeName(const std::string& name) {
  std::string result = name;
  for (char& ch : result) {
    if (ch == '/' || ch == '\\' || ch == ' ') {
      ch = '_';
    }
  }
  return result;
}

void createDirectories(const std::string& path) {
  if (path.empty()) {
    return;
  }

  std::string current;
  if (path.front() == '/') {
    current = "/";
  }

  std::stringstream stream(path);
  std::string part;
  while (std::getline(stream, part, '/')) {
    if (part.empty()) {
      continue;
    }
    if (!current.empty() && current.back() != '/') {
      current += "/";
    }
    current += part;
    if (::mkdir(current.c_str(), 0755) != 0 && errno != EEXIST) {
      throw std::runtime_error("failed to create log directory " + current + ": " + std::strerror(errno));
    }
  }
}

std::string runDirectoryName() {
  const char* runId = std::getenv("LOG_RUN_ID");
  if (runId != nullptr && runId[0] != '\0') {
    return sanitizeName(runId);
  }

  const auto now = std::chrono::system_clock::now();
  const std::time_t time = std::chrono::system_clock::to_time_t(now);
  const std::tm tmValue = localTime(time);
  std::ostringstream stream;
  stream << std::put_time(&tmValue, "%Y%m%d_%H%M%S");
  return stream.str();
}

std::string joinPath(const std::string& lhs, const std::string& rhs) {
  if (lhs.empty()) {
    return rhs;
  }
  if (lhs.back() == '/') {
    return lhs + rhs;
  }
  return lhs + "/" + rhs;
}

std::string csvEscape(const std::string& value) {
  bool needsQuotes = false;
  for (char ch : value) {
    if (ch == ',' || ch == '"' || ch == '\n' || ch == '\r') {
      needsQuotes = true;
      break;
    }
  }
  if (!needsQuotes) {
    return value;
  }

  std::string escaped;
  escaped.reserve(value.size() + 2);
  escaped += '"';
  for (char ch : value) {
    if (ch == '"') {
      escaped += "\"\"";
    } else {
      escaped += ch;
    }
  }
  escaped += '"';
  return escaped;
}

}  // namespace

Logger::Logger(const std::string& name) : Logger(name, defaultLogDir()) {}

Logger::Logger(const std::string& name, const std::string& logDir) : name_(name) {
  openFile(logDir);
}

void Logger::debug(const std::string& message) { log(Level::Debug, message); }

void Logger::info(const std::string& message) { log(Level::Info, message); }

void Logger::warn(const std::string& message) { log(Level::Warn, message); }

void Logger::error(const std::string& message) { log(Level::Error, message); }

void Logger::log(Level level, const std::string& message) {
  std::lock_guard<std::mutex> lock(mutex_);
  const auto line = timestampForLine() + "," + levelName(level) + "," + csvEscape(message);
  if (file_.is_open()) {
    file_ << line << std::endl;
    file_.flush();
  }
}

std::string Logger::defaultLogDir() {
  const char* overrideDir = std::getenv("LOG_DIR");
  if (overrideDir != nullptr && overrideDir[0] != '\0') {
    return overrideDir;
  }
  const char* home = std::getenv("HOME");
  if (home == nullptr || home[0] == '\0') {
    return "./logs";
  }
  return std::string(home) + "/.ros/log/logger";
}

std::string Logger::timestampForFile() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t time = std::chrono::system_clock::to_time_t(now);
  const std::tm tmValue = localTime(time);
  std::ostringstream stream;
  stream << std::put_time(&tmValue, "%Y%m%d_%H%M%S");
  return stream.str();
}

std::string Logger::timestampForLine() {
  const auto now = std::chrono::system_clock::now();
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
  const std::time_t time = std::chrono::system_clock::to_time_t(now);
  const std::tm tmValue = localTime(time);
  std::ostringstream stream;
  stream << std::put_time(&tmValue, "%Y-%m-%d %H:%M:%S") << '.' << std::setw(3) << std::setfill('0')
         << ms.count();
  return stream.str();
}

const char* Logger::levelName(Level level) {
  switch (level) {
    case Level::Debug:
      return "DEBUG";
    case Level::Info:
      return "INFO";
    case Level::Warn:
      return "WARN";
    case Level::Error:
      return "ERROR";
  }
  return "INFO";
}

void Logger::openFile(const std::string& logDir) {
  const auto runDir = joinPath(logDir, runDirectoryName());
  createDirectories(runDir);
  const auto safeName = sanitizeName(name_);
  filePath_ = joinPath(runDir, safeName + ".csv");
  file_.open(filePath_, std::ios::out | std::ios::app);
  if (!file_.is_open()) {
    throw std::runtime_error("failed to open log file: " + filePath_);
  }
  file_ << "timestamp,level,message" << std::endl;
  file_.flush();
}

}  // namespace logger
