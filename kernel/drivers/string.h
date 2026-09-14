#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdarg.h>

int strcmp(const char *s1, const char *s2);

int isspace(int c);

size_t strlen(const char *str);

char *strcpy(char *dest, const char *src);

char *strtok_r(char *str, const char *delim, char **saveptr);

char *snprintf(char *buffer, const char *format, ...);

char *sprintf(char *buffer, const char *format, ...);

int atoi(const char *str);

#endif // STRING_H
