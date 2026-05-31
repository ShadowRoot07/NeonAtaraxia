cmake_minimum_required(VERSION 3.14)
project(NeonAtaraxia)

set(CMAKE_CXX_STANDARD 17)

# --- GESTIÓN DE DEPENDENCIAS (FETCHCONTENT) ---
include(FetchContent)

# SDL2 base
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.30.2
)

# SDL2_mixer
FetchContent_Declare(
  SDL2_mixer
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
  GIT_TAG release-2.8.0
)

# === AGREGADO: SDL2_image para procesamiento de texturas PNG ===
FetchContent_Declare(
  SDL2_image
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  GIT_TAG release-2.8.2
)

# SDL2_ttf
FetchContent_Declare(
  SDL2_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  GIT_TAG release-2.22.0
)

# nlohmann/json para el ecosistema de datos
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)

set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(SDL2TTF_VENDORED ON CACHE BOOL "" FORCE)
set(SDL2IMAGE_VENDORED ON CACHE BOOL "" FORCE) # Asegura que use las dependencias internas de SDL_image

# Descargar y poner a disposición todas las dependencias (AÑADIDO SDL2_image aquí)
FetchContent_MakeAvailable(SDL2 SDL2_mixer SDL2_image SDL2_ttf json)
# ----------------------------------------------

include_directories(
    include
    include/gfx
    include/input
    include/physics
    include/player
    include/ui
    include/world
    include/core
    src 
)

# Escaneo de los archivos fuentes del MOTOR
file(GLOB_RECURSE MOTOR_SOURCES "src/*.cpp")
# list(FILTER MOTOR_SOURCES EXCLUDE REGEX "src/elements/EarthSkill.cpp")

# Definición del JUEGO / EJEMPLO activo
# set(ACTIVE_GAME "examples/Limbo/LimboGame.cpp")
set(ALL_SOURCES ${MOTOR_SOURCES} ${ACTIVE_GAME})

# Creamos la librería compartida (.so)
add_library(main SHARED ${ALL_SOURCES})

# Vinculamos con las librerías nativas de Android y los módulos de SDL
target_link_libraries(main
    android
    log
    SDL2::SDL2
    SDL2_mixer::SDL2_mixer
    SDL2_image::SDL2_image 
    SDL2_ttf::SDL2_ttf
    nlohmann_json::nlohmann_json
)

