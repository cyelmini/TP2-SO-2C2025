#include "include/string.h"
#include "include/syscalls.h"
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
