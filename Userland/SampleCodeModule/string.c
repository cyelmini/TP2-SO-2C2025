#include "include/string.h"
#include <stddef.h>

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