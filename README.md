# TP2 SO 2C2025 - x64BareBones

x64BareBones es una base minimalista para desarrollar sistemas operativos sobre la arquitectura Intel x86_64.
El objetivo es ofrecer un kernel bootstrappeado por Pure64, cargar modulos adicionales y facilitar la experimentacion con sincronizacion, planificacion y administracion de memoria en un entorno reproducible.

## Instrucciones de compilacion y ejecucion

### Requisitos
- `nasm`
- `qemu-system-x86_64`
- `gcc`
- `make`

Como alternativa, el contenedor `agodio/itba-so-multi-platform:3.0` ya incluye las dependencias y es el camino recomendado para evitar diferencias de entorno.

### Flujo recomendado con Docker
```bash
# Compilar toolchain y kernel (bitmap manager por defecto)
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
El comando genera `pvs-report.log` y la version HTML en `pvs-report-html/index.html`. 
Antes de relanzar el analisis conviene borrar `pvs-report-html/` para regenerar la salida limpia.

### Compilacion manual 
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
| Comando      | Tipo        | Descripción breve                                                             | Parámetros                             |
|:------------|:------------|:------------------------------------------------------------------------------|:---------------------------------------|
| `help`      | built-in    | Lista comandos y recordatorios de uso                                         | sin parámetros                         |
| `font-size` | built-in    | Cambia el tamaño de fuente                                                   | `<n>` (1 a 3)                          |
| `block`     | built-in    | Bloquea un proceso por PID                                                   | `<pid>`                                |
| `unblock`   | built-in    | Desbloquea un proceso bloqueado                                              | `<pid>`                                |
| `kill`      | built-in    | Finaliza un proceso                                                          | `<pid>`                                |
| `nice`      | built-in    | Ajusta prioridad de un proceso                                               | `<pid> <priority>`                     |
| `mem`       | built-in    | Muestra memoria total/ocupada/libre                                          | sin parámetros                         |
| `clear`     | aplicación  | Limpia la pantalla                                                           | `<&>` (opcional)                       |
| `ps`        | aplicación  | Lista procesos y su estado                                                   | `<&>` (opcional)                       |
| `loop`      | aplicación  | Imprime su ID con un saludo cada una determinada cantidad de segundos        | `<seconds> <&>` (opcional)             |
| `cat`       | aplicación  | Imprime el stdin tal como lo recibe                                          | sin parámetros                         |
| `wc`        | aplicación  | Cuenta la cantidad de líneas del input                                       | sin parámetros                         |
| `filter`    | aplicación  | Filtra las vocales del input                                                 | sin parámetros                         |
| `mvar`      | aplicación  | Lanza lectores/escritores sincronizados. Se ejecuta siempre en background    | `<iteraciones> <lectores> <escritores>`|
| `testmem`   | test        | Ciclo infinito que asigna y libera memoria verificando que no se superpongan | `<&>` (opcional)                       |
| `testproc`  | test        | Crea/bloquea/desbloquea procesos dummy                                       | `<&>` (opcional)                       |
| `testprio`  | test        | Valida prioridades cooperativas                                              | `<tope>`                               |
| `testsync`  | test        | Prueba sincronización con/sin semáforos                                      | `<iteraciones> <usar_sem>`               |

### Caracteres especiales para pipes y background
- `|` conecta la salida de un proceso con la entrada del siguiente (`cat archivo | wc`).

- `&` al final del comando ejecuta el proceso en segundo plano (`loop 5 &`). El default es ejecutar en foreground.

- Los built-ins no se pueden conectar mediante pipes.

### Atajos de teclado
- `Ctrl+C`: termina el proceso en foreground sin cerrar la shell.

- `Ctrl+D`: envia EOF a la entrada estandar del proceso actual.

### Ejemplos rapidos
- `ps` para inspeccionar procesos activos.

- `loop 2 &` para lanzar un proceso en background y recuperar la shell.

- `mvar 2 2` para observar sincronizacion con la variable compartida.

- `testsync 10 1` vs `testsync 10 0` para comparar ejecucion con y sin semaforos.

### Requerimientos faltantes o parcialmente implementados
Al día de la entrega no hay requerimientos faltantes ni parcialmente implementados.
Todos los puntos solicitados en el enunciado fueron implementados y verificados.

## Limitaciones
- La shell es minimalista: no hay autocompletado ni historial de comandos.

- El analisis PVS-Studio requiere conexion a internet en la primera ejecucion para instalar licencias comunitarias.

## Uso de IA y fuentes

### Herramientas de IA utilizadas
Se hizo uso de herramientas de inteligencia artificial como apoyo en los siguientes casos:

- Para pedir explicaciones teóricas o ejemplos sobre temas de la materia.

- Para redactar o comentar funciones repetitivas o de lógica básica (por ejemplo, recorrer strings, contar caracteres, etc.).

- En algunos casos, para ahorrar tiempo en tareas mecánicas, como generar encabezados de funciones o documentar archivos .h 
con descripciones de lo que hace cada función.

### Fuentes
Se utilizaron las clases prácticas provistas por la cátedra para la redacción del código. Además, los tests de memoria, prioridad, procesos 
y sincronización están basados en los tests de la cátedra y se adaptaron en los siguientes puntos: 

- Se agregaron llamadas a sys_exit() antes de cada return. En nuestra implementación, cada proceso le tiene que avisar al kernel que terminó su ejecución llamando a sys_exit(). Si no se hace, el proceso queda marcado como activo en el scheduler, y puede seguir ocupando memoria o bloqueando el cambio de contexto.

- En el test de sincronizacion, se modificó la ubicación de la destrucción del semáforo, moviéndola desde la función my_process_inc a la función principal test_sync. Ahora, el proceso padre (la función test_sync) es el único que destruye el semáforo, y lo hace recién después de esperar a todos los hijos. De esta forma, el recurso  compartido se libera una sola vez, evitando conflictos y bloqueos. Esta modificación se alinea mejor con nuestra implementación de semáforos.
