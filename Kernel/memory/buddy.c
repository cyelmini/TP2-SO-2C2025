/* ----------------------
 * Buddy Memory Manager
 * ----------------------
 * Administra memoria dividiéndola en bloques de tamaño potencia de 2
 * - Al reservar: se busca el bloque más chico que alcance el pedido,
 *   si es más grande, se divide en dos "buddies"
 * - Al liberar: se fusiona con su buddy libre para formar bloques mayores
 * Usa listas por orden (potencia de 2) para asignar y fusionar bloques rapido
 */

#include <memoryManagement.h>

#define FREE 0
#define USED 1
#define SPLIT 2
#define POW2(x) ((uint64_t) 1 << (x))
#define MIN_EXP 4
#define MAX_EXP 28
#define TOTAL_NODES (POW2(MAX_EXP - MIN_EXP + 1) - 1)

typedef struct Node {
	uint8_t state;
} Node;

typedef struct Node *TNode;

typedef struct MemoryManagerCDT {
	uint8_t treeStart;
	TNode tree;
	uint64_t size;
	uint64_t used;
} MemoryManagerCDT;

static MemoryManagerADT memoryBaseAddress = NULL;

MemoryManagerADT mm_create(void *const restrict startAddress, uint64_t totalSize) {
	if (totalSize < POW2(MIN_EXP)) {
		return NULL;
	}

	memoryBaseAddress = startAddress;
	MemoryManagerADT manager =
		(MemoryManagerADT) memoryBaseAddress; // En startAdress se guarda la informacion del struct en si

	manager->treeStart = startAddress + sizeof(MemoryManagerCDT); // Por lo que +sizeof, se empieza a guardar el arbol
	manager->tree =
		(TNode) (manager->treeStart +
				 HEAP_SIZE); // El mismo concepto para todos los nodos, luego de la direccion inicial del arbol
	manager->size = HEAP_SIZE;
	manager->used = 0;

	for (uint64_t i = 0; i < TOTAL_NODES; i++) {
		manager->tree[i].state = FREE;
	}

	return manager;
}

static MemoryManagerADT getMemoryManager(void) {
	return (MemoryManagerADT) memoryBaseAddress;
}

void *mm_alloc(size_t size) {
	MemoryManagerADT manager = getMemoryManager();
	if (size > manager->size - manager->used || size == 0) {
		return NULL;
	}
	uint8_t exponent = getExponent(size);
	int64_t nodo = findFreeNode(0, 0, MAX_EXP - exponent);
	if (nodo == -1)
		return NULL;
	splitTree(nodo);
	setSplitedChildren(nodo);
	manager->used += POW_2(exponent);
	return (void *) (manager->treeStart + (nodo - getNodeLevel(exponent)) * POW_2(exponent));
}

void freeMemory(void *const restrict memoryToFree) {
	MemoryManagerADT manager = getMemoryManager();
	if (memoryToFree == NULL) {
		return;
	}
	uint8_t exponent = getExponentPtr(memoryToFree);
	uint8_t nodo = getNodeIndex(memoryToFree, &exponent);
	manager->tree[nodo].state = FREE;
	setMerge(nodo);
	manager->used -= POW_2(exponent);
}

static int64_t getNodeIndex(uint8_t *ptr, uint8_t *exponent) {
	MemoryManagerADT manager = getMemoryManager();
	int64_t node = 0;
	uint8_t levelExponent = *exponent;
	while (levelExponent > 0 && manager->tree[node].state != USED) {
		node = (*(ptr - manager->treeStart) >> levelExponent) + getNodeLevel(levelExponent); //
		levelExponent--;
	}
	*exponent = levelExponent;
	return node;
}

static uint8_t getExponentPtr(void *memoryToFree) {
	MemoryManagerADT manager = getMemoryManager();
	uint64_t address = (uint8_t *) memoryToFree - manager->treeStart;
	uint8_t exponent = MAX_EXP;
	uint64_t size = POW_2(MAX_EXP);
	while (address % size != 0) {
		exponent--;
		size >>= 1;
	}
	return exponent;
}

static uint8_t getExponent(uint64_t size) {
	uint8_t exponent = MIN_EXP;
	uint64_t pot = (uint64_t) 1 << MIN_EXP;

	while (pot < size) {
		pot = pot << 1;
		exponent++;
	}

	return exponent;
}
static int64_t findFreeNode(uint64_t node, uint8_t level, uint8_t targetLevel) {
	MemoryManagerADT manager = getMemoryManager();
	if (level == targetLevel) {
		if (manager->tree[node].state == FREE)
			return node;
		else
			return -1;
	}
	if (manager->tree[node].state != SPLIT && manager->tree[node].state != FREE)
		return -1;
	int64_t left = findFreeNode(node * 2 + 1, level + 1, targetLevel);
	if (left != -1)
		return left;
	return findFreeNode(node * 2 + 2, level + 1, targetLevel);
}

static uint64_t getNodeLevel(uint8_t exponent) {
	return (POW_2(MAX_EXP - exponent) - 1);
}

static void setMerge(uint64_t node) {
	MemoryManagerADT manager = getMemoryManager();
	if (node == 0)
		return;

	uint64_t buddy;
	if (node % 2 == 0) {
		if (node == 0)
			return;
		buddy = node - 1;
	}
	else {
		buddy = node + 1;
		if (buddy >= TOTAL_NODES)
			return;
	}

	if (manager->tree[buddy].state == FREE && manager->tree[node].state == FREE) {
		uint64_t parent = (node - 1) / 2;
		if (parent < TOTAL_NODES && manager->tree[parent].state == SPLIT) {
			manager->tree[parent].state = FREE;
			setMerge(parent);
		}
	}
}

static void splitTree(uint64_t node) {
	MemoryManagerADT manager = getMemoryManager();
	while (node) {
		node = (node - 1) / 2;
		manager->tree[node].state = SPLIT;
	}
}

static void setSplitedChildren(uint64_t node) {
	MemoryManagerADT manager = getMemoryManager();
	if (node < TOTAL_NODES) {
		manager->tree[node].state = USED;
	}
}

mem_t mm_info(void) {
	MemoryManagerADT manager = getMemoryManager();
	mem_t info = {0, 0, 0};
	if (manager == NULL) {
		return info;
	}
	info.size = manager->size;
	info.used = manager->used;
	info.free = manager->size - manager->used;
	return info;
}