#pragma once
#include "..\ScriptManager.h"

inline void reload_script(const std::string& module_name) {
  Scripting::reload_script(module_name);
}

inline void output_text(const std::string& text) {
  std::cout << text.c_str() << std::endl;
}

inline void pong(const std::string& text) {
  output_text(text);
}

inline void event_handled(const std::string& text) {
  output_text(text);
}

inline void init_generic_bindings(py::module& module) {
  module.def("reload_script", &reload_script);
  module.def("output_text", &output_text);
  module.def("pong", &pong);
  module.def("event_handled", &event_handled);
}