#pragma once
#include "..\ScriptManager.h"

namespace scripting {
  namespace definitions {

    inline void reload_script(const std::string& module_name) {
      scripting::reload_script(module_name);
    }

    inline void output_text(const std::string& text) {
      std::cout << text.c_str() << std::endl;
    }

    inline void handle_message(const std::string& text) {
      output_text(text);
    }

    namespace example {
      inline void apply_definitions(py::module& module) {
        module.def("reload_script", &reload_script);
        module.def("send_message", &handle_message);
      }
    }
  }
}
