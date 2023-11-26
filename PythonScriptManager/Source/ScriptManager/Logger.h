#pragma once
#include <functional>
#include <iostream>
#include <sstream>
#include <utility>

namespace Scripting {
  class Logger {
  public:
    explicit Logger(std::function<void(const std::string&)> custom_logger = nullptr) : logger_(std::move(custom_logger)) {}

    // Set a custom logger function
    void set_logger(std::function<void(const std::string&)> custom_logger) {
      logger_ = std::move(custom_logger);
    }

    // Log a message
    template <typename... Args>
    void log_message(const Args&... args) {
      std::ostringstream oss;
      (oss << ... << args); // Concatenate arguments into a string
      const std::string message = oss.str();

      if (logger_) {
        logger_(message);
      }
      else {
        std::cerr << message << std::endl;
      }
    }

  private:
    std::function<void(const std::string&)> logger_;
  };
}