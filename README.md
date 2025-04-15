# Particle Simulation using Verlet Integration 

## Overview
A lightweight C++ particle simulator built with OpenGL, designed to simulate basic physics interactions using Verlet integration. 



https://github.com/user-attachments/assets/eddf0975-4d7c-405f-9e6f-a205fee8867f



## Features
- **Verlet Integration** for physics calculations
- **Space Partitioning** 
- **Instance Rendering** 
- **Customizable Simulation Parameters** (set before compilation)
- **GLFW & GLEW for OpenGL rendering**
- **GLM for mathematical computations** (on top of a custom math library)

## Dependencies
The project requires the following libraries, all included in the `Dependencies` folder:
- [GLFW 3.4](https://www.glfw.org/)
- [GLEW 2.1.0](http://glew.sourceforge.net/)
- [GLM (Mathematics Library)](https://glm.g-truc.net/0.9.9/index.html)

## Installation & Setup
### 1. Configure Project Properties
Before running the simulator, ensure the following settings are correctly configured in your development environment:
- **Include Directories:**
  - `$(SolutionDir)Dependencies\glfw-3.4.bin.WIN32\include`
  - `$(SolutionDir)Dependencies\glew-2.1.0\include`
  - `src\vendor`
- **Library Directories:**
  - `$(SolutionDir)Dependencies\glfw-3.4.bin.WIN32\lib-vc2022`
  - `$(SolutionDir)Dependencies\glew-2.1.0\lib\Release\Win32`
- **Additional Dependencies:**
  - `glfw3.lib`
  - `glew32s.lib`
  - `opengl32.lib`
  - `user32.lib`
  - `gdi32.lib`
  - `shell32.lib`
- **Preprocessor Definitions:**
  - `GLEW_STATIC`

### 2. Build & Run
  **Current Supported Configuration**:
  - **Platform**: x86

## Usage
**Controls**
- ESC to close
- Z to zoom out
- X to zoom in
- P to pause
- ARROW KEYS to move the camera
- R to reset camera movement
- Spacebar to apply a central force to all particles (press and hold)
- Mouse Left Click to apply an attractive force centered on the cursor
- Mouse Right Click to apply a repuslive force centerd on the cursor
- Simulation parameters must be set **before compilation** within the `application.cpp` file under **SIMULATION PARAMETERS**:
```cpp
// Simulation parameters
const float fixedDeltaTime = 1.0f / 60.0f;
const unsigned int subSteps = 2;
(...)
```
The parameters cannot be modified at runtime. Modify them in the source code and recompile to apply changes.
Physics constants can be modified in Constants.h and their implementation file.

## Known Issues & Limitations
- **Performance Limit:** The simulation struggles with more than **7500 particles** with 8 substeps (with debugger).
- Particles aren't stable when stacked on top of each other.

## Contribution
This project is open for contributions. Feel free to submit pull requests to improve performance, fix issues, or add features.

## License
This project is licensed under the [MIT License](LICENSE).
