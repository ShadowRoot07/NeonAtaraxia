#!/bin/bash

# Uso: ./create_module.sh <carpeta> <nombre_clase>
# Ejemplo: ./create_module.sh states GameplayState

DIR=$1
NAME=$2

if [ -z "$DIR" ] || [ -z "$NAME" ]; then
    echo "Uso: ./create_module.sh <carpeta> <nombre_clase>"
    exit 1
fi

mkdir -p include/$DIR src/$DIR

# Crear Header (.h) con guardas y constructor RAII
cat <<EOF > include/$DIR/$NAME.h
#ifndef ${NAME^^}_H
#define ${NAME^^}_H

class $NAME {
public:
    $NAME();
    ~$NAME() = default;

    // RAII: Deshabilitar copia para prevenir errores de memoria
    $NAME(const $NAME&) = delete;
    $NAME& operator=(const $NAME&) = delete;

    // Habilitar semántica de movimiento
    $NAME($NAME&& other) noexcept = default;
    $NAME& operator=($NAME&& other) noexcept = default;

private:
};

#endif
EOF

# Crear Implementación (.cpp)
cat <<EOF > src/$DIR/$NAME.cpp
#include "$DIR/$NAME.h"

$NAME::$NAME() {
    // Inicialización de recursos
}
EOF

echo "✅ Módulo $NAME creado en $DIR/"
