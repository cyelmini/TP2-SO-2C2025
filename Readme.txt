# TP2 SO 2C2025 - x64BareBones

x64BareBones es una base minimalista para desarrollar sistemas operativos sobre la arquitectura Intel x86_64.
El objetivo es ofrecer un kernel bootstrappeado por Pure64, cargar modulos adicionales y facilitar la experimentacion con sincronizacion, planificacion y administracion de memoria en un entorno reproducible.

## Caracteristicas clave
- Toolchain reproducible provisto mediante Docker (`compile.sh`).
- Kernel modular con loader propio y soporte para modulos de usuario.
- Userland de ejemplo con sincronizacion basada en pipes y un set de tests de laboratorio.
- Integracion opcional con PVS-Studio para analisis estatico automatizado.
- Scripts auxiliares para generar imagenes de disco (`Image/`) y ejecutar en QEMU (`run.sh`).

## Instrucciones de compilacion y ejecucion

### Requisitos
- `nasm`
- `qemu-system-x86_64`
- `gcc`
- `make`

Como alternativa, el contenedor `agodio/itba-so-multi-platform:3.0` ya incluye las dependencias y es el camino recomendado para evitar diferencias de entorno.

### Flujo recomendado con Docker
```bash
# Compilar toolchain y kernel (buddy manager por defecto)
./compile.sh

# Ejecutar imagen resultante en QEMU
./run.sh
```

#### Seleccionar administrador de memoria
```bash
./compile.sh --mm=bitmap
./compile.sh --mm=buddy
```

#### Analisis estatico con PVS-Studio
```bash
./compile.sh --pvs
```
El comando genera `pvs-report.log` y la version HTML en `pvs-report-html/index.html`. Antes de relanzar el analisis conviene borrar `pvs-report-html/` para regenerar la salida limpia.

### Compilacion manual (Linux nativo)
```bash
cd Toolchain
make all

cd ..
make all
```

### Ejecucion del kernel sin Docker
```bash
./run.sh
```
El script levanta QEMU utilizando la imagen generada en `Image/`.

## Estructura del repositorio
```
Bootloader/              # Pure64 + bmfs para crear la imagen booteable
Kernel/                  # Codigo del kernel y utilidades compartidas
Userland/                # Modulos de ejemplo y tests de usuario
Toolchain/               # Herramientas para empaquetar y compilar modulos
Image/                   # Artefactos generados (qcow2, vmdk)
compile.sh               # Script principal de build (usa Docker)
run.sh                   # Ejecuta la imagen resultante en QEMU
```

## Instrucciones de replicacion

### Comandos y tests disponibles en la shell
| Comando      | Tipo        | Descripcion breve | Parametros |
|--------------|-------------|-------------------|------------|
| `help`       | built-in    | Lista comandos y recordatorios de uso. | n/a |
| `font-size`  | built-in    | Cambia el tamaño de fuente. | `<n>` (8..32) |
| `block`      | built-in    | Bloquea un proceso por PID. | `<pid>` |
| `unblock`    | built-in    | Desbloquea un proceso bloqueado. | `<pid>` |
| `kill`       | built-in    | Finaliza un proceso. | `<pid>` |
| `nice`       | built-in    | Ajusta prioridad de un proceso. | `<pid> <priority>` |
| `mem`        | built-in    | Muestra memoria total/ocupada/libre. | n/a |
| `clear`      | aplicacion  | Limpia la pantalla. | n/a |
| `ps`         | aplicacion  | Lista procesos y su estado. | n/a |
| `loop`       | aplicacion  | Imprime un mensaje periodicamente. | `<seconds>` |
| `cat`        | aplicacion  | Muestra stdin o el contenido de un archivo. | `[archivo]` |
| `wc`         | aplicacion  | Cuenta lineas recibidas por stdin. | n/a |
| `filter`     | aplicacion  | Filtra vocales de la entrada. | n/a |
| `mvar`       | aplicacion  | Lanza lectores/escritores sincronizados. | `<iteraciones> <lectores> <escritores>` |
| `testmem`    | test        | Stress del administrador de memoria. | n/a |
| `testproc`   | test        | Crea/bloquea/desbloquea procesos dummy. | n/a |
| `testprio`   | test        | Valida prioridades cooperativas. | `<tope>` |
| `testsync`   | test        | Prueba sincronizacion con/ sin semaforos. | `<iteraciones> <usar_sem>` |

### Caracteres especiales para pipes y background
- `|` conecta la salida de un proceso con la entrada del siguiente (`cat archivo | wc`).
- `&` al final del comando ejecuta el proceso en segundo plano (`loop 5 &`).
- Los built-ins no se pueden conectar mediante pipes.

### Atajos de teclado
- `Ctrl+C`: termina el proceso en foreground sin cerrar la shell.
- `Ctrl+D`: envia EOF a la entrada estandar del proceso actual.

### Ejemplos rapidos
- `ps` para inspeccionar procesos activos.
- `loop 2 &` para lanzar un proceso en background y recuperar la shell.
- `cat lorem.txt | filter | wc` para demostrar pipes encadenados.
- `mvar 10 2 2` para observar sincronizacion con la variable compartida.
- `testsync 500 1` vs `testsync 500 0` para comparar ejecucion con y sin semaforos.

### Requerimientos faltantes o parciales
A la fecha no se detectaron requerimientos pendientes respecto al enunciado del TP. Registrar cualquier hallazgo en Issues antes de iterar.

## Resolucion de problemas rapida
- **Docker sin permisos**: confirma que tu usuario pertenece al grupo `docker` o ejecuta con `sudo`.
- **QEMU no arranca**: verifica que la compilacion termine sin errores y que `Image/x64BareBonesImage.qcow2` exista.
- **Analisis PVS-Studio falla**: asegurate de tener conectividad para instalar el paquete la primera vez y borra `pvs-report-html/` antes de repetir el comando.

## Limitaciones
- Solo soporta arquitectura x86_64 y ejecucion bajo QEMU; no hay soporte para hardware real.
- No se incluye sistema de archivos persistente; los modulos se cargan en memoria al iniciar.
- La shell es minimalista: no hay autocompletado ni historial de comandos.
- El analisis PVS-Studio requiere conexion a internet en la primera ejecucion para instalar licencias comunitarias.

## Autores originales
- Rodrigo Rearden (RowDaBoat)
- Augusto Nizzo McIntosh

## Mantenimiento actual
Este fork corresponde al trabajo practico de Sistemas Operativos (2C 2025). Las contribuciones deben documentarse con commits descriptivos y, de ser posible, incluir pruebas en `Userland/SampleCodeModule/tests/`.

## Uso de IA y fuentes
- README redactado y curado con asistencia de GitHub Copilot.
- No se incorporaron fragmentos de codigo externos adicionales mas alla de los provistos por el repositorio base.

## Licencia
Ver `License.txt` para detalles sobre derechos de autor y restricciones de uso.