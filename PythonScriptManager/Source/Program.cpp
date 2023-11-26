#include <thread>
#include <pybind11\embed.h>

#include "ScriptManager\ScriptManager.h"

int main()
{	
  // Initialize the Pybind11 interpreter
  py::scoped_interpreter guard{};
  py::gil_scoped_release release;
  Scripting::load_scripts("scripts");

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    Scripting::dispatch_event("on_ping", "from c++ with love");
  }
}
