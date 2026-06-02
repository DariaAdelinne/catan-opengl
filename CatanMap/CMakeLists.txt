cmake_minimum_required(VERSION 3.16)
project(CatanMap LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Pentru macOS/Homebrew:
# Apple Silicon: /opt/homebrew
# Intel Mac:     /usr/local
#
# Daca CMake nu gaseste glfw, ruleaza cu:
# cmake -S . -B build -DCMAKE_PREFIX_PATH=$(brew --prefix)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(CatanMap
    main.cpp

    Board.cpp
    Geometry.cpp
    TileShared.cpp
    Shader.cpp
    Camera.cpp
    Skybox.cpp

    OreTile.cpp
    ForestTile.cpp
    SheepTile.cpp
    WheatTile.cpp
    BrickTile.cpp
    DesertTile.cpp

    Water.cpp
    Ports.cpp
    ShadowMap.cpp
    NumberToken.cpp
    Card.cpp
    Bitmap.cpp
    Pieces.cpp
    Dice.cpp

    external/glad/src/glad.c
)

target_include_directories(CatanMap PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
)

target_link_libraries(CatanMap PRIVATE
    glfw
    OpenGL::GL
)

# Pe macOS, OpenGL e deprecated, dar inca merge.
if(APPLE)
    target_compile_definitions(CatanMap PRIVATE GL_SILENCE_DEPRECATION)
endif()

# Copiem folderele necesare langa executabil,
# ca shader-ele si imaginile BMP sa fie gasite la runtime.
add_custom_command(TARGET CatanMap POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders
            $<TARGET_FILE_DIR:CatanMap>/shaders
)

add_custom_command(TARGET CatanMap POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/cards
            $<TARGET_FILE_DIR:CatanMap>/cards
)