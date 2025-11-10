#!/bin/bash
set -e  # Termina el script si algún comando falla

# Imagen de Docker
IMAGE_NAME="agodio/itba-so-multi-platform:3.0"
PROJECT_PATH="/root"
MM_TYPE_ARG="" 
RUN_PVS=0

# Verificar argumentos del script
for arg in "$@"; do
  if [[ "$arg" == "--mm="* ]]; then # Parsear argumento --mm=
    MM_TYPE_ARG="${arg#--mm=}" # Extrae el valor después de '--mm=' y lo guarda
    echo ">>> Tipo de Administrador de Memoria especificado: ${MM_TYPE_ARG}"
  elif [[ "$arg" == "--pvs" ]]; then
    RUN_PVS=1
    echo ">>> Analisis PVS-Studio habilitado"
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
  set -e
    make clean -C ${PROJECT_PATH}/Toolchain
    ${MAKE_COMMAND} clean # Usa la variable MAKE_COMMAND para limpiar también
    make -C ${PROJECT_PATH}/Toolchain
  RUN_PVS=${RUN_PVS}
  if [ \"\$RUN_PVS\" -eq 1 ]; then
    if ! command -v pvs-studio-analyzer >/dev/null 2>&1; then
      apt-get update
      apt-get install -y pvs-studio
    fi
    pvs-studio-analyzer credentials PVS-Studio Free FREE-FREE-FREE-FREE || true
    pvs-studio-analyzer trace -- ${MAKE_COMMAND}
    pvs-studio-analyzer analyze -o ${PROJECT_PATH}/pvs-report.log
    plog-converter -a GA:1,2 -t fullhtml -o ${PROJECT_PATH}/pvs-report-html ${PROJECT_PATH}/pvs-report.log
  else
    ${MAKE_COMMAND} # <--- Ejecuta el comando make final con las variables incluidas
  fi
"

if [ "$RUN_PVS" -eq 1 ]; then
  echo ">>> Reporte PVS-Studio: ./pvs-report.log"
  echo ">>> Reporte PVS-Studio HTML: ./pvs-report-html/index.html"
else
  clear
fi