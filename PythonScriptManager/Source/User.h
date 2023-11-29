#pragma once

#include <random>
#include <pybind11\functional.h>
namespace py = pybind11;

class User {
public:
  User() {
    std::random_device rd;  // Seed
    std::mt19937 gen(rd()); // Mersenne Twister engine

    // Define a distribution range for unsigned long
    std::uniform_int_distribution<unsigned long> dis(0, ULONG_MAX);

    // Generate a random id
    id_ = dis(gen);

  }
  ~User() = default;

  unsigned long get_id() const { return id_; }

  static void apply_class_definitions(const py::module& module) {
    py::class_<User>(module, "User")
      .def(py::init<>())
      .def_property_readonly("id", &User::get_id);
  }

private:
  unsigned long id_;
};