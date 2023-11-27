#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <random>

// Include pybind11
#include <pybind11/embed.h>

#include "ScriptManager\Bindings\Bindings.h"
namespace py = pybind11;

// Include your ScriptManager
#include "ScriptManager/ScriptManager.h"

void clear_console() {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

void dispatch_events(int dispatch_count) {
  // Create a random number generator
  std::random_device rd;  // Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(1, 20);


  for (auto loop = 0; loop < dispatch_count; ++loop) {
    // Generate a random number between 1 and 20
    const auto random_number = distrib(gen);

    // Create the string with the random number
    std::string result = "on_load_" + std::to_string(random_number);

    Scripting::dispatch_event(result);
  }
}

// Function to handle loadtest command
double load_test(double last_run_time_seconds, bool multi_threaded) {
  clear_console();

  int dispatch_count;
  auto num_threads = 1;

  std::cout << "Enter the number of dispatches for this test: ";
  std::cin >> dispatch_count;

  const auto start = std::chrono::high_resolution_clock::now();

  if (multi_threaded) {
    std::cout << "Number of threads: ";
    std::cin >> num_threads;

    auto events_per_thread = dispatch_count / num_threads;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    for (auto i = 0; i < num_threads; ++i) {
      threads.emplace_back(dispatch_events, events_per_thread);
    }

    for (auto& th : threads) {
      th.join();
    }
  }
  else {
    dispatch_events(dispatch_count);
    std::cin.clear();
  }

  const auto end = std::chrono::high_resolution_clock::now();

  const std::chrono::duration<double, std::milli> elapsed_time = end - start;
  std::cout << "Execution Finished" << std::endl;
  std::cout << "Multi-threaded: " << multi_threaded << std::endl;
  if (multi_threaded) {
    std::cout << "Threads: " << num_threads << std::endl;
  }

  // Convert milliseconds to seconds
  const auto total_seconds = elapsed_time.count() / 1000.0;

  // Calculate minutes and seconds
  const auto minutes = static_cast<int>(total_seconds / 60);
  const auto seconds = static_cast<int>(total_seconds) % 60;

  std::cout << "Total Events Dispatched: " << dispatch_count << std::endl;
  std::cout << "Time Elapsed: " << minutes << " minutes, " << seconds << " seconds" << std::endl;

  const auto total_events = static_cast<double>(dispatch_count);
  const auto average_time_per_event = (elapsed_time.count() / total_events) / 1000.0;

  std::cout << "Average Time per Event Dispatch: " << average_time_per_event << " seconds per dispatch" << std::endl;

  if (last_run_time_seconds > 0) {
    std::cout << "This Load Test was " << abs(last_run_time_seconds - total_seconds) << " seconds " << ((last_run_time_seconds < total_seconds) ? "slower" : "faster") << " than the last test." << std::endl;
  }

  std::cout << std::endl;
  std::cout << std::endl;

  return total_seconds;
}

// Function to handle example command
void example() {
  std::string input;
  while (true) {
    std::cout << "Enter a message to send to python or type exit." << std::endl;
    std::cout << "Input: ";
    std::getline(std::cin, input);

    std::istringstream iss(input);
    std::vector<std::string> words{ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };

    if (words.empty()) {
      continue;
    }

    if (words[0] == "exit") {
      clear_console();
      break;
    }

    Scripting::dispatch_event("on_ping", input);
  }
}

int main() {
  py::scoped_interpreter guard{};
  py::gil_scoped_release release;

  Scripting::load_scripts("scripts", [](const std::string& script_name, const std::shared_ptr<ScriptModule>&) {
      std::cout << "Script loaded callback: " << script_name << std::endl;
    });

  auto last_run_time_seconds = 0.0;

  std::string input;
  while (true) {
    std::cout << "loadtest : Run a loadtest of the script manager (-mt: multithreaded execution)" << std::endl;
    std::cout << "   -mt: multi-threaded execution." << std::endl;
    std::cout << std::endl;
    std::cout << "example: Run an example ping/ping script" << std::endl;
    std::cout << std::endl;
    std::cout << "Enter command: ";
    std::getline(std::cin, input);

    std::istringstream iss(input);
    std::vector<std::string> words{ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };

    if (words.empty()) {
      continue;
    }

    if (words[0] == "loadtest") {
      auto multi_threading = false;

      for (const auto& word : words) {
        if (word == "-mt") {
          multi_threading = true;
        }
      }

      last_run_time_seconds = load_test(last_run_time_seconds, multi_threading);
    }
    else if (words[0] == "example") {
      example();
    }
    else if (words[0] == "exit") {
      break;
    }
    else {
      std::cout << "Unknown command." << std::endl;
    }
  }

  return 0;
}
