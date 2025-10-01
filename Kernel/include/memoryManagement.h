#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#define HEAP_SIZE (256 * 1024 * 1024)

/**
 * Información general del estado del heap.
 */
typedef struct mem {
	uint64_t size;
	uint64_t used;
	uint64_t free;
} mem_t;

typedef struct MemoryManagerCDT *MemoryManagerADT;

/**
 * @brief Inicializa el gestor de memoria en una región contigua
 *
 * @param startAddress Dirección de inicio de la región a administrar
 * @param totalSize    Tamaño total en bytes de la region
 * @return MemoryManagerADT Handle al memory manager, o NULL si falla
 */
MemoryManagerADT mm_create(void *const restrict startAddress, uint64_t totalSize);

/**
 * @brief Reserva memoria dinámica dentro del heap administrado
 *
 * @param size Cantidad de bytes a reservar
 * @return void* Puntero al bloque asignado, o NULL si no hay espacio
 */
void *mm_alloc(size_t size);

/**
 * @brief Libera un bloque previamente asignado
 *
 * @param ptr Puntero devuelto por mm_alloc(), ignora NULL
 */
void mm_free(void *const restrict ptr);

/**
 * @brief Obtiene informacion del heap administrado
 *
 * @return mem_t Estructura con tamaño total, usado y libre
 */
mem_t mm_info(void);

#endif /* MEMORY_MANAGER_H */