#pragma once
#include "..\ScriptManager.h"
#include <random>
#include <string>

namespace scripting {
  namespace events {

    inline void send_message(User* user, const std::string& message) {
      dispatch_event("on_message", user, message);
    }

  }
}
