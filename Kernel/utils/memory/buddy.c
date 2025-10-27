/* ----------------------
 * Buddy Memory Manager
 * ----------------------
 * Administra memoria dividiéndola en bloques de tamaño potencia de 2
 * - Al reservar: se busca el bloque más chico que alcance el pedido,
 *   si es más grande, se divide en dos "buddies"
 * - Al liberar: se fusiona con su buddy libre para formar bloques mayores
 * Usa listas por orden (potencia de 2) para asignar y fusionar bloques rapido
 */

#include "../../include/memoryManagement.h"

#define FREE 0
#define USED 1
#define SPLIT 2
#define MIN_EXP 4
#define MAX_EXP 28

typedef struct Node {
	uint8_t state;
} Node;

typedef struct Node *TNode;

typedef struct MemoryManagerCDT {
	uint8_t *treeStart;
	TNode tree;
	uint64_t size;
	uint64_t used;
	uint8_t maxExp;
	uint64_t totalNodes;
} MemoryManagerCDT;

static MemoryManagerADT memoryBaseAddress = NULL;

static int64_t getNodeIndex(uint8_t *ptr, uint8_t *exponent);
static uint8_t getExponentPtr(void *memoryToFree);
static uint8_t getExponent(uint64_t size);
static int64_t findFreeNode(uint64_t node, uint8_t level, uint8_t targetLevel);
static uint64_t getNodeLevel(uint8_t exponent);
static void setMerge(uint64_t node);
static void splitTree(uint64_t node);
static void setSplitedChildren(uint64_t node);

MemoryManagerADT mm_create(void *const restrict startAddress, uint64_t totalSize) {
	if (totalSize < POW2(MIN_EXP)) {
		return NULL;
	}

	uint8_t computedMax = MIN_EXP;
	while (((uint64_t) 1 << (computedMax + 1)) <= (totalSize) && computedMax < MAX_EXP) {
		computedMax++;
	}

	uint64_t nodes = ((uint64_t) 1 << (computedMax - MIN_EXP + 1)) - 1;

	uint64_t needed = sizeof(MemoryManagerCDT) + (nodes * sizeof(Node)) + POW2(MIN_EXP);
	if (totalSize < needed) {
		return NULL;
	}

	uint8_t *base = (uint8_t *) startAddress;
	memoryBaseAddress = (MemoryManagerADT) base;
	MemoryManagerADT manager = (MemoryManagerADT) memoryBaseAddress;

	manager->maxExp = computedMax;
	manager->totalNodes = nodes;
	manager->tree = (TNode) (base + sizeof(MemoryManagerCDT));
	manager->treeStart = base + sizeof(MemoryManagerCDT) + (nodes * sizeof(Node));

	manager->size = totalSize - (sizeof(MemoryManagerCDT) + (nodes * sizeof(Node)));
	manager->used = 0;

	for (uint64_t i = 0; i < manager->totalNodes; i++) {
		manager->tree[i].state = FREE;
	}

	return manager;
}

static MemoryManagerADT getMemoryManager(void) {
	return (MemoryManagerADT) memoryBaseAddress;
}

void *mm_alloc(size_t size) {
	MemoryManagerADT manager = getMemoryManager();
	if (size > manager->size - manager->used || size == 0 || size > POW2(17)) {
		return NULL;
	}
	uint8_t exponent = getExponent(size);
	int64_t nodo = findFreeNode(0, 0, manager->maxExp - exponent);
	if (nodo == -1)
		return NULL;
	splitTree(nodo);
	setSplitedChildren(nodo);
	manager->used += POW2(exponent);
	uint64_t offset = (uint64_t) (nodo - getNodeLevel(exponent)) * POW2(exponent);
	if (offset >= manager->size) {
		return NULL;
	}
	return (void *) (manager->treeStart + offset);
}

void mm_free(void *const restrict memoryToFree) {
	MemoryManagerADT manager = getMemoryManager();
	if (memoryToFree == NULL) {
		return;
	}
	uint8_t exponent = getExponentPtr(memoryToFree);
	int64_t nodo = getNodeIndex((uint8_t *) memoryToFree, &exponent);
	if (nodo < 0)
		return;
	manager->tree[nodo].state = FREE;
	setMerge(nodo);
	manager->used -= POW2(exponent);
}

static int64_t getNodeIndex(uint8_t *ptr, uint8_t *exponent) {
	MemoryManagerADT manager = getMemoryManager();
	int64_t node = 0;
	uint8_t levelExponent = *exponent;
	while (levelExponent > 0 && manager->tree[node].state != USED) {
		/* compute offset in bytes from treeStart, then shift by levelExponent */
		if ((uintptr_t) ptr < (uintptr_t) manager->treeStart)
			return -1;
		uint64_t offset = (uint64_t) ((uintptr_t) ptr - (uintptr_t) manager->treeStart);
		if (offset >= manager->size)
			return -1;
		node = (int64_t) ((offset >> levelExponent) + getNodeLevel(levelExponent));
		levelExponent--;
	}
	*exponent = levelExponent;
	return node;
}

static uint8_t getExponentPtr(void *memoryToFree) {
	MemoryManagerADT manager = getMemoryManager();
	if ((uintptr_t) memoryToFree < (uintptr_t) manager->treeStart)
		return 0;
	uint64_t address = (uint64_t) ((uintptr_t) memoryToFree - (uintptr_t) manager->treeStart);
	uint8_t exponent = manager->maxExp;
	uint64_t size = POW2(manager->maxExp);
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
	MemoryManagerADT manager = getMemoryManager();
	return (POW2((uint64_t) manager->maxExp - exponent) - 1);
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
		if (buddy >= manager->totalNodes)
			return;
	}

	if (buddy < manager->totalNodes && manager->tree[buddy].state == FREE && manager->tree[node].state == FREE) {
		uint64_t parent = (node - 1) / 2;
		if (parent < manager->totalNodes && manager->tree[parent].state == SPLIT) {
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
	if (node < manager->totalNodes) {
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
