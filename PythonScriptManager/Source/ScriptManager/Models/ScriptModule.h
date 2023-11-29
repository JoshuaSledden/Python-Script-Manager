#pragma once
#include <filesystem>
namespace py = pybind11;

namespace scripting {
  namespace models {
    /// <summary>
    /// A class to hold python script module state.
    /// </summary>
    class ScriptModule {
    public:
      ScriptModule() = default;
      ScriptModule(const ScriptModule&) = delete;
      ScriptModule(std::string name, const std::shared_ptr<py::module_>& script_module, std::filesystem::path absolute_path, std::filesystem::path relative_path)
        : name_(std::move(name)), absolute_path_(std::move(absolute_path)), relative_path_(std::move(relative_path)),
        script_module_(script_module) {}
      ~ScriptModule() = default;

      /// <summary>
      /// A getter for the name of the module used for loading the script.
      /// </summary>
      /// <returns>A string representing the module name.</returns>
      std::string name() { return name_; }

      /// <summary>
      /// A getter for the absolute path where the python script module is located
      /// </summary>
      /// <returns>A filesystem path representing the script's absolute path</returns>
      std::filesystem::path absolute_path() {
        return absolute_path_;
      }

      /// <summary>
      /// A getter for the relative path where the python script module is located
      /// </summary>
      /// <returns>A filesystem path representing the script's path relative to the working directory</returns>
      std::filesystem::path relative_path() {
        return relative_path_;
      }

      /// <summary>
      /// A getter for the python
      /// </summary>
      /// <returns></returns>
      std::shared_ptr<py::module_> script_module() const { return script_module_; }

    private:
      std::string name_;
      std::filesystem::path absolute_path_;
      std::filesystem::path relative_path_;
      std::shared_ptr<py::module_> script_module_;
    };
  }
}