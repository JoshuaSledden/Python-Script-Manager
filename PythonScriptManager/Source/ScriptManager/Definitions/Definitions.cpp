#include <pybind11\embed.h>
#include "..\..\User.h"
#include "ExampleDefinitions.h"
#include "LoadTestDefinitions.h"
using namespace scripting::definitions;

PYBIND11_EMBEDDED_MODULE(example_module, module)
{
  module.doc() = "Example Module";
  example::apply_definitions(module);
  loadtest::apply_definitions(module);
  User::apply_class_definitions(module);
}