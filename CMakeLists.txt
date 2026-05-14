cmake_minimum_required(VERSION 3.14)
project(MysteryOfLimbo)

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

# NUEVO: SDL2_ttf
FetchContent_Declare(
  SDL2_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  GIT_TAG release-2.22.0
)

# Desactivar opciones innecesarias y forzar compilación estática/compartida según necesites
set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(SDL2TTF_VENDORED ON CACHE BOOL "" FORCE) # Esto ayuda a que SDL_ttf use sus propias librerías de fuentes

# Descargar y poner a disposición
FetchContent_MakeAvailable(SDL2 SDL2_mixer SDL2_ttf)
# ----------------------------------------------

# Incluimos tus carpetas de código
include_directories(include include/gfx src)

# Escaneo de archivos fuente
file(GLOB_RECURSE SOURCES "src/*.cpp")
# Excluimos el archivo que me mencionaste que da problemas
list(FILTER SOURCES EXCLUDE REGEX "src/elements/EarthSkill.cpp")

# Creamos la librería compartida (.so) para Android
add_library(main SHARED ${SOURCES})

# Vinculamos con los targets oficiales de SDL2 descargados
target_link_libraries(main
    android
    log
    SDL2::SDL2
    SDL2_mixer::SDL2_mixer
    SDL2_ttf::SDL2_ttf
)
