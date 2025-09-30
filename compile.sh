#!/bin/bash
set -e  # Termina el script si algún comando falla

# Imagen de Docker
IMAGE_NAME="agodio/itba-so-multi-platform:3.0"
PROJECT_PATH="/root"
MM_TYPE_ARG="" 

# Verificar argumentos del script
for arg in "$@"; do
  if [[ "$arg" == "--mm="* ]]; then # Parsear argumento --mm=
    MM_TYPE_ARG="${arg#--mm=}" # Extrae el valor después de '--mm=' y lo guarda
    echo ">>> Tipo de Administrador de Memoria especificado: ${MM_TYPE_ARG}"
  fi
done

# Inicializa MAKE_COMMAND con el comando base y el directorio del proyecto
MAKE_COMMAND="make -C ${PROJECT_PATH}"


if [ -n "$MM_TYPE_ARG" ]; then 
  MAKE_COMMAND="${MAKE_COMMAND} MM_TYPE=${MM_TYPE_ARG}"
  echo ">>> Pasando MM_TYPE=${MM_TYPE_ARG} a make..."
fi

# Ejecuta los comandos dentro del contenedor Docker
docker run --rm -v "${PWD}:/root" --privileged -ti "$IMAGE_NAME" bash -c "
    make clean -C ${PROJECT_PATH}/Toolchain
    ${MAKE_COMMAND} clean # Usa la variable MAKE_COMMAND para limpiar también
    make -C ${PROJECT_PATH}/Toolchain
    ${MAKE_COMMAND} # <--- Ejecuta el comando make final con las variables incluidas
"

clear