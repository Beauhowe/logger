#include "logger/Logger.h"

int main() {
  logger::Logger logger("logger_example");
  logger.debug("debug message");
  logger.info("info message");
  logger.warn("warn message");
  logger.error("error message");
  return 0;
}
