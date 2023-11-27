#pragma once
#include <filesystem>
#include <pybind11\embed.h>
#include <pybind11\functional.h>
#include <pybind11\gil.h>
namespace py = pybind11;

#include "Logger.h"
#include "Models\ScriptModule.h"
using namespace Scripting::Models;

namespace Scripting {
  /// <summary>
  /// Simple examples of logger overrides.
  /// </summary>
  inline void log_debug(const std::string& message) {
    std::cout << "[INFO]: " << message.c_str() << std::endl;
  }

  inline void log_warning(const std::string& message) {
    std::cout << "[WARNING]: " << message.c_str() << std::endl;
  }

  inline void log_error(const std::string& message) {
    std::cout << "[ERROR]: " << message.c_str() << std::endl;
  }

  /// <summary>
  /// Handle the lifecycle and execution of python scripts.
  /// This class will handle messages between c++ and python to create a fluent scripting system.
  /// </summary>
  class ScriptManager {
    ScriptManager() {
      logger_ptr_ = std::make_shared<Logger>();
      if (logger_ptr_) {
        logger_ptr_->set_logger(LogType::LOG_INFO, &log_debug);
        logger_ptr_->set_logger(LogType::LOG_WARNING, &log_warning);
        logger_ptr_->set_logger(LogType::LOG_ERROR, &log_error);
      }
    }
    ~ScriptManager() = default;

  public:
    ScriptManager(const ScriptManager&) = delete;
    ScriptManager& operator=(const ScriptManager&) = delete;

    /// <summary>
    /// Get the singleton instance of the manager.
    /// </summary>
    /// <returns>A ScriptManager singleton instance.</returns>
    static ScriptManager& instance() {
      static ScriptManager instance;
      return instance;
    }

    /// <summary>
    /// Accessor for the logger.
    /// </summary>
    /// <returns>A shared pointer representing the logger</returns>
    std::shared_ptr<Logger> get_logger() {
      return logger_ptr_;
    }

    /// <summary>
    /// Set the path for modules to be loaded from.
    /// </summary>
    /// <param name="path">modules path</param>
    void set_module_path(const std::string& path) {
      module_path_ = path;
    }

    /// <summary>
    /// Load an individual python module in to memory.
    /// </summary>
    /// <param name="module_path">Module path of the python script.</param>
    void load_script(const std::filesystem::path& module_path, std::function<void(const std::string&, const std::shared_ptr<ScriptModule>)> callback_on_load) {
      py::gil_scoped_acquire acquire;

      // Use an absolute path to ensure consistency
      const auto absolute_path = std::filesystem::absolute(module_path);
      const auto relative_path = std::filesystem::relative(absolute_path, std::filesystem::current_path());
      const auto absolute_dir = std::filesystem::absolute(module_path.parent_path());
      const auto module_name = relative_path.stem().string();

      try {
        // Add the path to the python system.
        const auto sys_path = py::module::import("sys").attr("path");
        sys_path.attr("append")(absolute_dir.string());

        // Load the python module
        const auto module = py::module::import(module_name.c_str());

        // Store the loaded script in memory so we can interact with it throughout the server lifecycle.
        const auto script = std::make_shared<ScriptModule>(module_name, std::make_shared<py::module_>(module), absolute_path, relative_path);
        loaded_modules_[module_name] = script;

        // Check if any of the cached functions are held within this module and add it to those cache entries.
        for (const auto& function_name : module_function_cache_) {
          if (py::hasattr(*module, function_name.first.c_str())) {
            module_function_cache_[function_name.first].push_back(script);
          }
        }

        callback_on_load(module_name, script);
        logger_ptr_->log_message(LogType::LOG_INFO, "ScriptManager::load_script - Script Module Loaded: ", module_name);
      }
      catch (const py::error_already_set& e) {
        // An exception occurred, print the error message and traceback
        PyErr_Print();
        logger_ptr_->log_message(LogType::LOG_ERROR, "ScriptManager::load_script - Script Error.\n", e.what());

        // Access the Python traceback
        PyObject* type, * value, * traceback;
        PyErr_Fetch(&type, &value, &traceback);

        // Print the traceback
        if (traceback) {
          py::object print_tb = py::module::import("traceback").attr("print_tb");
          logger_ptr_->log_message(LogType::LOG_ERROR, "Trace:\n", traceback);
        }
      }
    }

    /// <summary>
    /// Reload an already loaded python module
    /// </summary>
    /// <param name="module_name">The name of a module to reload</param>
    void reload_script(const std::string& module_name) {
      py::gil_scoped_acquire acquire;

      // Check if the script is already loaded.
      const auto it = loaded_modules_.find(module_name);

      if (it == loaded_modules_.end()) {
        logger_ptr_->log_message(LogType::LOG_ERROR, "ScriptManager::reload_script - Error: Script not loaded.");
        return;
      }

      // Reload the module.
      const auto script = it->second;

      // Remove the module from cache so reloading is effective.
      for (auto& [function_name, modules] : module_function_cache_) {
        auto found_module = std::find_if(modules.begin(), modules.end(), [&script](const std::shared_ptr<ScriptModule>& module) {
          return module->name() == script->name();
          });

        if (found_module != modules.end()) {
          modules.erase(found_module);
        }
      }

      try {
        script->script_module()->reload();

        // Add the reloaded module in to any function cache where applicable.
        for (const auto& function_name : module_function_cache_) {
          if (py::hasattr(*script->script_module(), function_name.first.c_str())) {
            module_function_cache_[function_name.first].push_back(script);
          }
        }

        logger_ptr_->log_message(LogType::LOG_INFO, "ScriptManager::reload_script - Reloaded Module: ", module_name);
      }
      catch (const py::error_already_set& e) {
        // An exception occurred, print the error message and traceback
        PyErr_Print();
        logger_ptr_->log_message(LogType::LOG_ERROR, "ScriptManager::reload_script - Error.\n", e.what());

        // Access the Python traceback
        PyObject* type, * value, * traceback;
        PyErr_Fetch(&type, &value, &traceback);

        // Print the traceback
        if (traceback) {
          py::object print_tb = py::module::import("traceback").attr("print_tb");
          logger_ptr_->log_message(LogType::LOG_ERROR, "Trace:\n", traceback);
        }
      }
    }

    /// <summary>
    /// Load all scripts from a given path.
    /// </summary>
    /// <param name="path">The path housing the python scripts.</param>
    void load_scripts(const std::filesystem::path& path = std::filesystem::path(), std::function<void(const std::string&, const std::shared_ptr<ScriptModule>)> callback_on_load = {}) {
      const std::filesystem::path current_path = std::filesystem::current_path();
      const auto module_path = path.empty() ? (current_path / module_path_) : (current_path / path);

      // Check if the directory exists
      if (!std::filesystem::exists(module_path) || !std::filesystem::is_directory(module_path)) {
        logger_ptr_->log_message(LogType::LOG_ERROR, "ScriptManager::load_scripts: directory not found - ", module_path);
        return;
      }

      for (const auto& module : std::filesystem::recursive_directory_iterator(module_path)) {
        if (!module.is_regular_file()) {
          continue;
        }

        if (module.path().extension() != ".py") {
          continue;
        }

        load_script(module.path().string(), callback_on_load);
      }
    }

    /// <summary>
    /// Sends an event to a single loaded script for them to handle
    /// </summary>
    /// <param name="module_name">name of the module we want python to handle</param>
    /// <param name="event_key_name">name of the event function we want python to handle</param>
    /// <param name="...args">Argument list to pass to the python handlers</param>
    template <typename... Args>
    void send_event_to_single_module(const std::string& module_name, const std::string& event_key_name, Args&&... args) {
      const auto it = loaded_modules_.find(module_name);
      if (it == loaded_modules_.end())
      {
        logger_ptr_->log_message(LogType::LOG_ERROR, __FUNCTION__" Could not find module: ", module_name);
        return;
      }

      send_event_to_single_module(*it, event_key_name, args);
    }

    /// <summary>
    /// Sends an event to a single loaded script for them to handle
    /// </summary>
    /// <param name="module_name">The module we want python to handle</param>
    /// <param name="event_key_name">name of the event function we want python to handle</param>
    /// <param name="...args">Argument list to pass to the python handlers</param>
    template <typename... Args>
    void send_event_to_single_module(std::shared_ptr<ScriptModule> script_module, const std::string& event_key_name, Args&&... args) {
      py::gil_scoped_acquire acquire;
      const auto module_ = script_module->script_module().get();

      try {
        if (py::hasattr(*script_module->script_module(), event_key_name.c_str())) {
          logger_ptr_->log_message(LogType::LOG_INFO, "ScriptManager::dispatch_event - Dispatching cached event: ", event_key_name);

          // Call the specified Python function variadically
          py::object result = module_->attr(event_key_name.c_str())(
            std::forward<Args>(args)...);
        }
      }
      catch (const py::error_already_set& e) {
        // An exception occurred, print the error message and traceback
        PyErr_Print();
        logger_ptr_->log_message(LogType::LOG_ERROR, "ScriptManager::send_event_to_single_module - Script Error.\n",
          e.what());

        // Access the Python traceback
        PyObject* type, * value, * traceback;
        PyErr_Fetch(&type, &value, &traceback);

        // Print the traceback
        if (traceback) {
          py::object print_tb = py::module::import("traceback").attr("print_tb");
          logger_ptr_->log_message(LogType::LOG_ERROR, "Trace:\n", traceback);
        }
      }
    }

    /// <summary>
    /// Dispatch an event to any loaded scripts for them to handle
    /// </summary>
    /// <param name="event_key_name">name of the event function we want python to handle</param>
    /// <param name="...args">Argument list to pass to the python handlers</param>
    template <typename... Args>
    void dispatch_event(const std::string& event_key_name, Args&&... args) {
      py::gil_scoped_acquire acquire;

      // Is this function already in the cache?
      const auto it = module_function_cache_.find(event_key_name);

      // If it was found in cache, instead of iterating over all loaded modules, iterate over each module in the cache associated with this function.
      if (it != module_function_cache_.end()) {
        const auto cached_function_modules = it->second;

        for (const auto& cached_module : cached_function_modules) {
          auto script_module = cached_module->script_module();
          try {
            if (py::hasattr(*script_module, event_key_name.c_str())) {
              logger_ptr_->log_message(LogType::LOG_INFO, "ScriptManager::dispatch_event - Dispatching cached event: ", event_key_name);

              // Call the specified Python function variadically
              py::object result = script_module->attr(event_key_name.c_str())(
                std::forward<Args>(args)...);
            }
          }
          catch (const py::error_already_set& e) {
            // An exception occurred, print the error message and traceback
            PyErr_Print();
            logger_ptr_->log_message(LogType::LOG_ERROR, "ScriptManager::dispatch_event - Script Error.\n",
              e.what());

            // Access the Python traceback
            PyObject* type, * value, * traceback;
            PyErr_Fetch(&type, &value, &traceback);

            // Print the traceback
            if (traceback) {
              py::object print_tb = py::module::import("traceback").attr("print_tb");
              logger_ptr_->log_message(LogType::LOG_ERROR, "Trace:\n", traceback);
            }
          }
        }
      }
      else {
        // The function was not found in cache so we will iterate over all loaded modules and if the event is active in any module we will add it to cache.
        std::vector<std::shared_ptr<ScriptModule>> valid_modules;

        // Iterate over all loaded scripts
        for (const auto& loaded_script : loaded_modules_) {
          const auto script = loaded_script.second;
          const auto module = script->script_module().get();

          // Check if the function exists in the script
          try {
            if (py::hasattr(*module, event_key_name.c_str())) {
              logger_ptr_->log_message(LogType::LOG_INFO, "ScriptManager::dispatch_event - Dispatching event: ", event_key_name);

              // Call the specified Python function variadically
              py::object result = module->attr(event_key_name.c_str())(
                std::forward<Args>(args)...);

              // Add the module so it gets added to the function cache.
              valid_modules.push_back(script);
            }
          }
          catch (const py::error_already_set& e) {
            // An exception occurred, print the error message and traceback
            PyErr_Print();
            logger_ptr_->log_message(LogType::LOG_ERROR, "ScriptManager::dispatch_event - Script Error.\n",
              e.what());

            // Access the Python traceback
            PyObject* type, * value, * traceback;
            PyErr_Fetch(&type, &value, &traceback);

            // Print the traceback
            if (traceback) {
              py::object print_tb = py::module::import("traceback").attr("print_tb");
              logger_ptr_->log_message(LogType::LOG_ERROR, "Trace:\n", traceback);
            }
          }
        }

        // If the function was found in any modules add it to the cache with the valid modules.
        if (!valid_modules.empty()) {
          logger_ptr_->log_message(LogType::LOG_INFO, "ScriptManager::dispatch_event - Creating function cache for event: ", event_key_name);
          module_function_cache_[event_key_name] = valid_modules;
        }
      }
    }

  private:
    // Logger to provide custom logging context.
    std::shared_ptr<Logger> logger_ptr_;

    // Directory housing the python scripts
    std::string module_path_;

    // List of all the loaded python script modules
    std::unordered_map<std::string, std::shared_ptr<ScriptModule>> loaded_modules_;

    // A cache that holds all functions a script holds (for performance purposes)
    std::unordered_map <std::string, std::vector<std::shared_ptr<ScriptModule>>> module_function_cache_;
  };

  /// <summary>
  /// A wrapper function to get the Script Manager's Logger.
  /// </summary>
  inline std::shared_ptr<Logger> get_logger() {
    return ScriptManager::instance().get_logger();
  }

  /// <summary>
  /// A wrapper function to dispatch events without having to call for the instance each time.
  /// </summary>
  /// <typeparam name="...Args">Args list type</typeparam>
  /// <param name="event_key_name">Name of the python function</param>
  /// <param name="args">Variadic arguments to pass to the python function</param>
  template <typename... Args>
  void dispatch_event(const std::string& event_key_name, Args&&... args) {
    ScriptManager::instance().dispatch_event(event_key_name, std::forward<Args>(args)...);
  }

  /// <summary>
  /// A wrapper function to send events to single modules without having to call for the instance each time.
  /// </summary>
  /// <typeparam name="...Args">Args list type</typeparam>
  /// <param name="module_name">Name of the python module</param>
  /// <param name="event_key_name">Name of the python function</param>
  /// <param name="args">Variadic arguments to pass to the python function</param>
  template <typename... Args>
  void send_event_to_single_module(const std::string& module_name, const std::string& event_key_name, Args&&... args) {
    ScriptManager::instance().send_event_to_single_module(module_name, event_key_name, std::forward<Args>(args)...);
  }

  /// <summary>
  /// A wrapper function to send events to single modules without having to call for the instance each time.
  /// </summary>
  /// <typeparam name="...Args">Args list type</typeparam>
  /// <param name="module">The python module</param>
  /// <param name="event_key_name">Name of the python function</param>
  /// <param name="args">Variadic arguments to pass to the python function</param>
  template <typename... Args>
  void send_event_to_single_module(std::shared_ptr<ScriptModule> script_module, const std::string& event_key_name, Args&&... args) {
    ScriptManager::instance().send_event_to_single_module(script_module, event_key_name, std::forward<Args>(args)...);
  }

  /// <summary>
  /// A wrapper function to load scripts without having to call for the instance each time.
  /// </summary>
  /// <param name="path">Path name housing the python modules</param>
  /// <param name="callback_on_load">Callback function that will be called when each script successfully loads</param>
  inline void load_scripts(const std::filesystem::path& path = std::filesystem::path(), std::function<void(const std::string&, const std::shared_ptr<ScriptModule>)> callback_on_load = {}) {
    ScriptManager::instance().load_scripts(path, callback_on_load);
  }

  /// <summary>
  /// A wrapper function to load a single script without having to call for the instance each time.
  /// </summary>
  /// <param name="module_path">Path name housing the python module</param>
  /// <param name="callback_on_load">Callback function that will be called when the script successfully loads</param>
  inline void load_script(const std::filesystem::path& module_path, std::function<void(const std::string&, const std::shared_ptr<ScriptModule>)> callback_on_load = {}) {
    ScriptManager::instance().load_script(module_path, callback_on_load);
  }

  /// <summary>
  /// A wrapper function to reload a single script without having to call for the instance each time.
  /// </summary>
  /// <param name="module_name">Name of the module being reloaded</param>
  inline void reload_script(const std::string& module_name) {
    ScriptManager::instance().reload_script(module_name);
  }
}
