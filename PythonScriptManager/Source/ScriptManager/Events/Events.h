#pragma once
#include "..\ScriptManager.h"
#include <random>
#include <string>

namespace scripting {
  namespace events {

    inline void random_loadtest_function() {
      // Create a random number generator
      std::random_device rd;  // Will be used to obtain a seed for the random number engine
      std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
      std::uniform_int_distribution<> distrib(1, 20);


      // Generate a random number between 1 and 20
      const auto random_number = distrib(gen);

      // Create the string with the random number
      const std::string result = "on_load_" + std::to_string(random_number);
      dispatch_event(result);
    }

  }
}
