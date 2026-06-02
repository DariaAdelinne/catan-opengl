# CatanMap — 3D OpenGL Board Game Scene

<p align="center">
  <img src="screenshots/readme/01-hero-full-scene.jpg" alt="CatanMap 3D board game scene" width="920">
</p>

<p align="center">
  <strong>An interactive 3D board inspired by Catan, built from scratch in C++ and modern OpenGL.</strong>
</p>

<p align="center">
  <img alt="C++17" src="https://img.shields.io/badge/C%2B%2B-17-blue">
  <img alt="OpenGL" src="https://img.shields.io/badge/OpenGL-Modern%20Pipeline-green">
  <img alt="GLSL" src="https://img.shields.io/badge/GLSL-Shaders-orange">
  <img alt="CMake" src="https://img.shields.io/badge/CMake-3.16%2B-red">
</p>

## Overview

**CatanMap** is an educational computer graphics project that recreates a stylized 3D board-game environment inspired by *Catan*. The board contains 19 hexagonal tiles, procedural resource biomes, number tokens, player pieces, ports, dice, textured cards, development cards, animated details, water and an environmental skybox.

The project was designed to demonstrate a complete modern OpenGL rendering workflow: reusable geometry, GLSL shaders, transformations, camera interaction, lighting, transparency, shadow mapping and several advanced OpenGL state-machine techniques.

> This is a non-commercial educational project inspired by a board-game concept. It is not an official Catan product.

## Visual Showcase

### Complete board layout

<p align="center">
  <img src="screenshots/readme/02-board-top-view.jpg" alt="Top-down view of the complete CatanMap board" width="820">
</p>

### Procedural resource biomes

Each resource tile is assembled from reusable geometric primitives rather than imported 3D models. The scene includes forest, wheat, sheep, brick, ore and desert tiles.

<p align="center">
  <img src="screenshots/readme/04-procedural-biomes.jpg" alt="Procedural biome showcase" width="920">
</p>

### Textured cards and game elements

BMP textures are loaded into OpenGL and applied to resource cards and development cards. The board also contains dice, number tokens, pieces and ports.

<p align="center">
  <img src="screenshots/readme/03-resource-cards.jpg" alt="Textured resource cards" width="820">
</p>

<p align="center">
  <img src="screenshots/readme/06-game-elements.jpg" alt="Dice, cards, number tokens, pieces and ports" width="920">
</p>

### Skybox environment

<p align="center">
  <img src="screenshots/readme/07-skybox-environment.jpg" alt="CatanMap scene rendered inside an environmental skybox" width="920">
</p>

## Rendering Features

- Modern OpenGL pipeline with VAO/VBO buffers and GLSL shaders
- Model, view and projection transformations
- Phong-style lighting with hemispheric ambient light
- Quantized diffuse lighting for a stylized low-poly appearance
- Shadow mapping using a dedicated framebuffer and depth texture
- Gaussian 5×5 PCF filtering for smoother shadow edges
- Alpha blending and distance-based fading for cards and number tokens
- Face culling for correctly rendering the front and back of cards
- Stencil testing for tile outlines
- 4× MSAA anti-aliasing
- Reinhard tone mapping, saturation adjustment and gamma correction
- Procedural rounded corners for cards using an SDF in the fragment shader
- Cubemap skybox rendering

<p align="center">
  <img src="screenshots/readme/05-shadow-map-flow.jpg" alt="Shadow mapping render flow diagram" width="760">
</p>

## Interactive Controls

| Input | Action |
|---|---|
| `A` / `D` | Rotate the camera horizontally |
| `W` / `S` | Rotate the camera vertically |
| `Q` / `E` | Zoom in / out |
| Left mouse drag | Pan the camera target across the board |
| `R` | Randomize the tile distribution, number tokens and player pieces |
| `Space` | Roll the dice |
| `T` | Flip the next development card; reset the deck after all cards are revealed |
| `F` | Pin or unpin the cards and number tokens when changing the camera distance |
| `Esc` | Close the application |

## Project Structure

```text
catan-opengl/
├── README.md
├── screenshots/readme/       # Images embedded in this README
└── CatanMap/
    ├── main.cpp                 # Application entry point and render loop
    ├── Board.* / Tile.*         # Hexagonal board generation and tile data
    ├── ForestTile.*             # Procedural forest biome
    ├── WheatTile.*              # Procedural wheat biome
    ├── SheepTile.*              # Procedural pasture biome
    ├── BrickTile.*              # Procedural brick biome
    ├── OreTile.*                # Procedural ore biome
    ├── DesertTile.*             # Procedural desert biome
    ├── Card.* / Bitmap.*        # BMP loading and card rendering
    ├── Dice.* / Pieces.*        # Interactive game elements
    ├── Ports.* / NumberToken.*  # Board-game details
    ├── Water.* / Skybox.*       # Environment rendering
    ├── ShadowMap.*              # Depth framebuffer and shadow mapping
    ├── shaders/                 # GLSL vertex and fragment shaders
    ├── cards/                   # BMP card textures
    ├── skybox/                  # Cubemap BMP textures
    └── external/                # Included third-party headers and sources
```

## Build and Run

### Requirements

- CMake `3.16+`
- A C++17 compiler
- OpenGL
- GLFW 3

GLAD and GLM are included in the repository under `CatanMap/external/`.

### macOS with Homebrew

```bash
brew install cmake glfw
cd CatanMap
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build build
./build/CatanMap
```

### Linux (Debian / Ubuntu)

```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev
cd CatanMap
cmake -S . -B build
cmake --build build
./build/CatanMap
```

The CMake configuration copies the `shaders/`, `cards/` and `skybox/` asset folders next to the executable after a successful build.

## Technical Notes

The board is regenerated by shuffling tile types and number tokens while preserving the 19-tile board structure. Most visual objects are generated procedurally from simple primitives, which keeps the scene modular and makes it easy to extend individual biomes.

## Possible Future Improvements

- Import support for complex `.obj` models
- Tile selection and resource highlighting
- Movable player pieces
- Additional animations for water, cards and natural elements
- More detailed materials and normal maps
- Rendering optimizations for larger scenes

## Author

**Daria Cristea**  
Computer Graphics project — Technical University of Iași, Faculty of Automatic Control and Computer Engineering
