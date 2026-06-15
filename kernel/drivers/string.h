#ifndef STRING_H
#define STRING_H

#include <stddef.h>

int strcmp(const char *s1, const char *s2);

int isspace(int c);

size_t strlen(const char *str);

char *strcpy(char *dest, const char *src);

char *strtok_r(char *str, const char *delim, char **saveptr);

#endif // STRING_H