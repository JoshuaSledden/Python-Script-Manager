# Python Script Manager

## Table of Contents
- [Python Script Manager](#python-script-manager)
	- [Table of Contents](#table-of-contents)
	- [Overview](#overview)
	- [Features](#features)
	- [Prerequisites and Requirements](#prerequisites-and-requirements)
	- [Installation](#installation)
	- [Usage](#usage)
	- [Importing Python Script Manager to Fly For Fun](#importing-python-script-manager-to-fly-for-fun)
	- [Support and Contribution](#support-and-contribution)

## Overview
The Python Script Manager is a robust tool designed to integrate Python scripting seamlessly into a C++ environment. Primarily geared towards game development, it provides a fluent scripting system that bridges the gap between Python and C++ functionalities. This tool is essential for developers looking to leverage the simplicity of Python within C++-based applications, particularly in the context of game world interactions and management.

## Features
- **Python-C++ Integration**: Utilizes `pybind11` to embed Python within the C++ environment, ensuring smooth interaction between the two languages.
- **Script Lifecycle Management**: Handles the entire lifecycle of Python scripts, from loading to execution and unloading, within the C++ application.
- **Dynamic Script Reloading**: Supports real-time reloading of scripts, enabling on-the-fly updates and testing.
- **Comprehensive Binding Support**: Includes bindings for various types, movers, and world elements, allowing extensive manipulation and interaction within the game world.
- **Logging and Event Handling**: Custom logging functionalities and event handling mechanisms for better script management and debugging.
- **Module Function Caching**: Implements caching for module functions, enhancing performance by reducing redundant loading and parsing of frequently used scripts.

## Prerequisites and Requirements
- **Python**: A compatible Python installation is required for script execution. Ensure that Python is properly installed and accessible by the C++ environment.
- **pybind11**: This tool relies heavily on `pybind11` for embedding Python into C++. `pybind11` must be installed and configured correctly.
- **C++17 Compiler**: The project is built using C++17 standards; thus, a compatible C++ compiler is necessary.
- **Game Development Context**: While primarily designed for game development, the Python Script Manager can be adapted for other C++-Python integration needs.

## Installation
1. **Clone the Repository**: Clone or download the Python Script Manager repository to your local machine.
2. **Configure pybind11**: Ensure `pybind11` is installed and properly set up in your development environment.
3. **Build the Project**: Use a C++17 compatible compiler to build the project. Follow the build instructions specific to your platform.

## Usage
- Integrate the Python Script Manager into your C++ project.
- Utilize the provided APIs to load, execute, and manage Python scripts.
- Leverage the binding functionalities to interact with Python scripts efficiently.

## Importing Python Script Manager to Fly For Fun
For detailed instructions on integrating the Python Script Manager into the Fly For Fun game, please refer to the [FlyFF Integration Guide](FlyFF.md).

## Support and Contribution
For support, feature requests, or contributions, please open an issue or a pull request in the project repository.
