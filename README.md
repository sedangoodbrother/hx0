# Fencing Game

## Project Overview
This project is a 2D fencing game that simulates a fencing match between two players. The game is designed to be played on a computer and features animations, input handling, and various gameplay mechanics. The project is implemented in C++ using SDL (Simple DirectMedia Layer) for rendering and input handling.

---

## Project Structure
The project is organized into the following main components:

### 1. **Source Code Files**
- **`character.cpp` and `character.h`**: Contains the logic for character actions, animations, and input handling.
- **`common.cpp` and `common.h`**: Includes shared utilities and constants used across the project.
- **`menu.cpp` and `menu.h`**: Implements the game menu and user interface.
- **`main.cpp`**: The entry point of the game, initializing the game loop and managing the overall flow.

### 2. **Assets**
- **Sprites**: Located in the `assets/` folder, including textures for player actions (e.g., `player_attack.png`, `player_idle.png`).
- **Animation Frames**: Found in `assets/fencer_animation_frames/`, containing detailed animations for various actions.
- **Fonts**: The `font/` folder contains fonts like `Arian LT Light.ttf` for rendering text.

### 3. **Build System**
- **CMake**: The project uses CMake for cross-platform build configuration. The `CMakeLists.txt` file defines the build rules.
- **Build Artifacts**: The `build/` folder contains compiled binaries and intermediate files.

### 4. **Configuration Files**
- **`input.txt`**: Defines key mappings for Player 1 and Player 2.
- **CMakeModules/**: Contains custom CMake modules for finding SDL dependencies.

---

## Gameplay and Features

### Gameplay
- **Two-Player Mode**: The game supports two players, each controlling a fencer.
- **Actions**: Players can move, attack, and parry using predefined key mappings.
- **Scoring**: Points are awarded based on successful hits.

### Features
- **Animations**: Smooth animations for player actions, including attacks, parries, and idle states.
- **Input Handling**: Customizable key mappings for both players.
- **Collision Detection**: Accurate hitbox and hurtbox management for realistic gameplay.
- **Game Periods**: Matches are divided into periods, with support for sudden death in case of a tie.

---

## References
- **SDL Documentation**: [https://wiki.libsdl.org/](https://wiki.libsdl.org/)
- **Fencing Techniques**: Inspiration from real-world fencing techniques and animations.
- **SDL Game-making tutorials**: [https://www.parallelrealities.co.uk/tutorials/](https://www.parallelrealities.co.uk/tutorials/)
---

## Build Guide

### Linux
1. Install dependencies:
   ```bash
   sudo apt update
   sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
   ```
2. Clone the repository and navigate to the project directory:
   ```bash
   git clone <repository-url>
   cd <repository-folder>
   ```
3. Create a build directory and run CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```
4. Run the game:
   ```bash
   ./Fencing
   ```

### Windows
1. Install dependencies:
   - Download and install [CMake](https://cmake.org/download/).
   - Install SDL2, SDL2_image, and SDL2_ttf libraries.
2. Clone the repository and navigate to the project directory.
3. Open the project in CMake GUI:
   - Set the source folder to the project directory.
   - Set the build folder to a new directory (e.g., `build/`).
   - Configure and generate the project files.
4. Build the project using your preferred IDE (e.g., Visual Studio).
5. Run the generated executable.

---
