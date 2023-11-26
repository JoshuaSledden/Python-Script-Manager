#pragma once
#include <functional>
#include <iostream>
#include <sstream>
#include <utility>

namespace Scripting {
  enum LogType : unsigned int {
    INFO = 0,
    WARNING,
    ERROR,
    LOG_TYPE_COUNT
  };

  /// <summary>
  /// A simple logger that allows you to override it with custom logging functions.
  /// </summary>
  class Logger {
  public:
    using LoggerFunction = std::function<void(const std::string&)>;

    explicit Logger() = default;

    /// <summary>
    /// Assign a custom logger handler function for a log type
    /// </summary>
    /// <param name="log_type">A Log type category</param>
    /// <param name="custom_logger">A function to handle the logging event.</param>
    void set_logger(LogType log_type, std::function<void(const std::string&)> custom_logger) {
      logger_handlers_[log_type] = std::move(custom_logger);
    }

    /// <summary>
    /// Log a message type and specify the log type.
    /// If a logger handler is not setup for the provided type then default to normal logging.
    /// </summary>
    /// <typeparam name="...Args">Variadic arguments</typeparam>
    /// <param name="log_type">A Log type category</typeparam>
    /// <param name="args">List of parameters to pass in to the logger handler</param>
    template <typename... Args>
    void log_message(const LogType log_type, const Args&... args) {
      std::ostringstream oss;
      (oss << ... << args); // Concatenate arguments into a string
      const std::string message = oss.str();

      if (logger_handlers_[log_type]) {
        logger_handlers_[log_type](message);
      }
      else {
        std::cerr << message << std::endl;
      }
    }

  private:
    LoggerFunction logger_handlers_[LOG_TYPE_COUNT] = { nullptr };
  };
}