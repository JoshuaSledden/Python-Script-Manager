#pragma once
#include "..\ScriptManager.h"

namespace scripting {
  namespace definitions {

      inline void event_handled(const std::string& text) {
        output_text(text);
      }

      namespace loadtest {
        inline void apply_definitions(py::module& module) {
          module.def("event_handled", &event_handled);
        }
      }
  }
}
