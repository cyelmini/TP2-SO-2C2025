#include <stdio.h>
#include "../include/memoryManagement.h"
#include "../include/doubleLinkedList.h"

typedef struct Node {
	void *data;
	struct Node *prev;
	struct Node *next;
} Node;

typedef struct doubleLinkedListCDT {
	Node *head;
	Node *tail;
	Node *current;
	int size;
} doubleLinkedListCDT;

doubleLinkedListADT createDoubleLinkedListADT() {
	doubleLinkedListADT list = (doubleLinkedListADT) mm_alloc(sizeof(doubleLinkedListCDT));
	if (list == NULL) {
		return NULL;
	}
	list->head = NULL;
	list->tail = NULL;
	list->current = NULL;
	list->size = 0;
	return list;
}

Node *addNode(doubleLinkedListADT list, void *data) {
	if (list == NULL) {
		return NULL;
	}

	Node *newNode = (Node *) mm_alloc(sizeof(Node));
	if (newNode == NULL) { 
		return NULL;
	}
	newNode->data = data;
	newNode->next = NULL;

	if (list->size > 0) {
		list->tail->next = newNode;
	}
	else {
		list->head = newNode;
	}

	newNode->prev = list->tail;
	list->tail = newNode;
	list->size++;

	return newNode;
}

Node *addNodeInFirst(doubleLinkedListADT list, Node *node) {
	if (list == NULL || node == NULL) {
		return NULL;
	}
	node->prev = NULL;
	if (list->size > 0) {
		list->head->prev = node;
	}
	else {
		list->tail = node;
	}
	node->next = list->head;
	list->head = node;
	list->size++;
	return node;
}

void *removeNode(doubleLinkedListADT list, void *data) {
	if (list == NULL || data == NULL) {
		return NULL;
	}

	Node *current = list->head;
	while (current != NULL) {
		if (current->data == data) {
			if (current->prev != NULL) {
				current->prev->next = current->next;
			}
			else {
				list->head = current->next;
			}
			if (current->next != NULL) {
				current->next->prev = current->prev;
			}
			else {
				list->tail = current->prev;
			}

			list->size--;

			void *removedData = current->data;
			mm_free(current);
			return removedData;
		}
		current = current->next;
	}

	return NULL;
}

void *getFirstData(doubleLinkedListADT list) {
	if (list == NULL || list->size == 0) {
		return NULL;
	}
	return removeNode(list, list->head->data);
}

void freeLinkedListADT(doubleLinkedListADT list) {
	Node *current = list->head;
	Node *next;
	while (current) {
		next = current->next;
		mm_free(current);
		current = next;
	}
	mm_free(list);
}

int getSize(doubleLinkedListADT list) {
	if (list == NULL) {
		return -1;
	}
	return list->size;
}

int isEmpty(doubleLinkedListADT list) {
	if (list == NULL) {
		return -1;
	}
	return list->size == 0;
}

void toBegin(doubleLinkedListADT list) {
	if (list == NULL) {
		return;
	}
	list->current = list->head;
}

int hasNext(doubleLinkedListADT list) {
	if (list == NULL) {
		return 0;
	}
	return list->current != NULL;
}

void *nextInList(doubleLinkedListADT list) {
	if (!hasNext(list)) {
		return NULL;
	}
	void *data = list->current->data;
	list->current = list->current->next;
	return data;
}
