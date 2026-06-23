#ifndef LOGGER_LOGGER_H
#define LOGGER_LOGGER_H

#include <fstream>
#include <mutex>
#include <string>

namespace logger {

enum class Level { Debug, Info, Warn, Error };

class Logger {
 public:
  explicit Logger(const std::string& name);
  Logger(const std::string& name, const std::string& logDir);

  void debug(const std::string& message);
  void info(const std::string& message);
  void warn(const std::string& message);
  void error(const std::string& message);
  void log(Level level, const std::string& message);

  const std::string& filePath() const { return filePath_; }

 private:
  std::string name_;
  std::string filePath_;
  std::ofstream file_;
  std::mutex mutex_;

  static std::string defaultLogDir();
  static std::string timestampForFile();
  static std::string timestampForLine();
  static const char* levelName(Level level);

  void openFile(const std::string& logDir);
};

}  // namespace logger

#endif  // LOGGER_LOGGER_H
