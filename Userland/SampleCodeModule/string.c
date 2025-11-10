// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "include/string.h"
#include "include/syscalls.h"
#include <stddef.h>
#include <stdint.h>

static int my_isdigit(char c);

int strcmp(const char *s1, const char *s2) {
	while (*s1 != 0 && *s1 == *s2) {
		s1++;
		s2++;
	}
	return *s1 - *s2;
}

int strcpy(char *dest, const char *origin) {
	return strcpychar(dest, origin, '\0');
}

int strcpychar(char *dest, const char *origin, char limit) {
	int idx = 0;
	while (origin[idx] != limit && origin[idx] != '\0') {
		dest[idx] = origin[idx];
		idx++;
	}
	dest[idx] = 0;
	return idx;
}

int strlen(const char *s) {
	size_t len = 0;
	if (!s)
		return 0;
	while (s[len] != '\0') {
		len++;
	}
	return len;
}

char *find_char(const char *s, char c) {
	if (!s)
		return NULL;
	while (*s) {
		if (*s == c)
			return (char *) s;
		s++;
	}
	return NULL;
}

int str_in_list(const char *needle, char *list[], int list_len) {
	if (!needle || !list || list_len <= 0)
		return 0;
	for (int i = 0; i < list_len; i++) {
		if (list[i] && strcmp(needle, list[i]) == 0)
			return i;
	}
	return -1;
}

static int my_isdigit(char c) {
	return (c >= '0' && c <= '9');
}

uint32_t str_to_uint32(char *str) {
	uint32_t result = 0;
	int i = 0;
	int hasDigit = 0;

	while (str[i] == ' ' || str[i] == '\t')
		i++;

	while (my_isdigit(str[i])) {
		hasDigit = 1;
		result = result * 10 + (str[i] - '0');
		i++;
	}

	while (str[i] == ' ' || str[i] == '\t')
		i++;

	if (!hasDigit || str[i] != '\0')
		return -1;

	return result;
}
