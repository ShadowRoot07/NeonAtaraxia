# Mystery of Limbo Makefile - Optimized for Termux X11
CXX = clang++
# Agregamos -Iinclude para que encuentre todas las subcarpetas (world, gfx, player)
CXXFLAGS = -Iinclude -Isrc `sdl2-config --cflags` -std=c++17 -MMD -O2
LDFLAGS = `sdl2-config --libs` -lSDL2_mixer -lSDL2_ttf

# Directorios
SRC_DIR = src
BUILD_DIR = build
TARGET = limbo_core

# Encontrar todos los .cpp recursivamente, excluyendo EarthSkill.cpp
SOURCES = $(shell find $(SRC_DIR) -name '*.cpp' ! -name 'EarthSkill.cpp')
# Generar la lista de objetos manteniendo la estructura de carpetas en build/
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))
DEPS = $(OBJECTS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "🟢 Compilación exitosa: ./$(TARGET)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "🧹 Limpieza completada."

.PHONY: all clean

