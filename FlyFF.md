# Fly For Fun (FlyFF) Integration Guide

## Table of Contents
- [Fly For Fun (FlyFF) Integration Guide](#fly-for-fun-flyff-integration-guide)
	- [Table of Contents](#table-of-contents)
		- [Adding pybind11 to Visual Studio](#adding-pybind11-to-visual-studio)
		- [Adding Python Libraries and Includes](#adding-python-libraries-and-includes)
		- [File Structure and Project Setup](#file-structure-and-project-setup)
		- [Modifying StdAfx Precompiled Header](#modifying-stdafx-precompiled-header)
		- [Adding Pybind Interpreter to WinMain](#adding-pybind-interpreter-to-winmain)
		- [Loading Scripts in InitInstance](#loading-scripts-in-initinstance)
		- [Dispatching Events in FlyFF](#dispatching-events-in-flyff)
		- [Binding Classes with pybind11](#binding-classes-with-pybind11)
		- [Gotchas: Working Directory and C++17's Filesystem](#gotchas-working-directory-and-c17s-filesystem)

### Adding pybind11 to Visual Studio
1. **Open WorldServer Project**: In Visual Studio, open the WorldServer project.
2. **Modify Project Properties**: Right-click on the project in the Solution Explorer and select 'Properties'.
3. **Add pybind11 Header**: Go to `C/C++` > `General` > `Additional Include Directories`. Add the path to the pybind11 header files.

### Adding Python Libraries and Includes
1. **Open Project Properties**: Right-click on the WorldServer project and select 'Properties'.
2. **Add Python Includes**: Navigate to `C/C++` > `General` > `Additional Include Directories`. Add the path to the Python include directory.
3. **Add Python Libraries**: Go to `Linker` > `General` > `Additional Library Directories`. Add the path to the Python libraries directory.

### File Structure and Project Setup
- Add the `ScriptManager` files to the WorldServer folder and project.
Recommended folder structure within the WorldServer project:


```
WorldServer
     └── ScriptManager
		├── ScriptManager.h
		├── Logging.h
		│
		├── Bindings
		│ 	├── Bindings.cpp
		│ 	└── Bindings.h
		│
		└── Models
			└── ScriptModule.h
```


### Modifying StdAfx Precompiled Header
Add the following includes to the bottom of the StdAfx precompiled header file:
```cpp
#include "ScriptManager\ScriptManager.h"
#include "ScriptManager\Bindings\Bindings.h"
```

### Adding Pybind Interpreter to WinMain
In Worldserver.exe's WinMain function, add the pybind interpreter:
```cpp
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow) {
    // Initialize the Pybind11 interpreter
    py::scoped_interpreter guard{};
    py::gil_scoped_release release;

    // ... rest of the code
}
```

### Loading Scripts in InitInstance
In the InitInstance function of WorldServer.cpp, include the load_scripts function call:
```cpp
xSRand( timeGetTime() ); // Initialize random seed

if (Script("WorldServer.ini") == TRUE) {
    Scripting::load_scripts(DIR_SCRIPTS);
    // ... rest of the code
}
```

### Dispatching Events in FlyFF
A simple example of dispatching events in flyff would be to create a simple chat command handler for python.
Any in-game chat message that starts with "." will be forwarded to python to handle as a script command.

In CDPSrvr::OnChat, dispatch events like so:
```cpp
if (sChat[0] == '.' && ParsingScriptCommand(strChat, static_cast<CMover*>(pUser))) {
    return;
}
```

Add the ParsingScriptCommand function to FuncTextCmd.cpp:
```cpp
bool ParsingScriptCommand(LPCTSTR lpszString, CMover* pMover) {
    CScanner scanner;
    scanner.SetProg((LPTSTR)lpszString);
    scanner.dwValue = (DWORD_PTR)pMover;
    scanner.GetToken(); // skip /
    scanner.GetToken(); // get command

#ifdef __WORLDSERVER
    Scripting::dispatch_event("on_chat_command", std::string(scanner.m_pBuf));
#endif // __WORLDSERVER
    return true;
}
```

Add this function definition to CmdHdr.h:
```cpp
class CMover;
extern bool ParsingScriptCommand(LPCTSTR lpszString, CMover* pMover);
// ... other function declarations
```

Create a script in the DIR_SCRIPT folder named chat_commands.py:
```py
import shlex
import example_module

def reload_script(script_name):
    example_module.reload_script(script_name)

def on_chat_command(command_string):
    parsed_command = shlex.split(command_string)
    action = parsed_command[0]

    if action == ".reload_script":
        reload_script(parsed_command[1])

# Additional command handling can be added here
```

Typing ``.reload_script chat_commands`` in the in-game chat will reload the script, demonstrating the event handler functionality. This examples covers C++ calling python and python calling back in to C++.

### Binding Classes with pybind11
Example for binding the CMover class:

```cpp
inline void init_mover_bindings(py::module& module) {
    py::class_<CMover>(module, "Mover")
        .def(py::init<>())
        .def_readonly("index", &CMover::m_dwIndex)
        .def_readonly("player_id", &CMover::m_idPlayer);
}
```
Include this in Bindings.cpp within the pybind11 embedded module.

Then you can pass CMover objects around in python but be careful with handling references and object lifecycle.

```py
import example_module

def example_mover_usage(input_mover: example_module.Mover):
    print(f"Mover index: {input_mover.index}, Player ID: {input_mover.player_id}")

# Additional usage of the Mover class can be implemented here
```

### Gotchas: Working Directory and C++17's Filesystem

When working with C++17's filesystem in FlyFF, be aware of the working directory settings. Update the following section in WorldServer.cpp's Script function:

```cpp
if (s.Token == "ResourceFolder") {
    s.GetToken();
    // Adjusting to the new filesystem path
    char buffer[MAX_PATH];
    GetModuleFileName(nullptr, buffer, MAX_PATH);
    std::filesystem::path executable_path(buffer);
    std::string resource_path(s.Token);
    std::filesystem::path full_path = executable_path.parent_path() / resource_path;
    std::filesystem::current_path(full_path);

    if (!SetCurrentDirectory(full_path.string().c_str())) {
        FLERROR_LOG( PROGRAM_NAME, _T( "Folder not found, %s" ), full_path.string().c_str() );
        return FALSE;
    }
}
```
This change ensures the correct setting of the working directory relative to the executable's location.