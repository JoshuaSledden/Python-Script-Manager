#include <pybind11/embed.h>
#include "Bindings.h"

PYBIND11_EMBEDDED_MODULE(example_module, module)
{
  module.doc() = "Example Module";
  init_generic_bindings(module);
}