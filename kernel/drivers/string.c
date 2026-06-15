#ifndef STRING_C
#define STRING_C

#include "string.h"

#include <stddef.h>

// Compare two strings
int strcmp(const char *s1, const char *s2){
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

// Detect white-space
int isspace(int c){
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

// Calculate the length of a null-terminated string
size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Copy a null-terminated string from src to dest
char *strcpy(char *dest, const char *src) {
    char *saved = dest;
    while ((*dest++ = *src++) != '\0');
    return saved;
}

// Thread-safe string tokenizer (required for parsing shell arguments)
char *strtok_r(char *str, const char *delim, char **saveptr) {
    char *token;

    if (str == NULL) {
        str = *saveptr;
    }

    if (str == NULL || *str == '\0') {
        *saveptr = NULL;
        return NULL;
    }

    // Scan leading delimiters
    while (*str != '\0') {
        const char *d = delim;
        while (*d != '\0' && *d != *str) {
            d++;
        }
        if (*d == '\0') {
            break; // Found a non-delimiter character
        }
        str++;
    }

    if (*str == '\0') {
        *saveptr = NULL;
        return NULL;
    }

    token = str;

    // Scan to the end of the token
    while (*str != '\0') {
        const char *d = delim;
        while (*d != '\0' && *d != *str) {
            d++;
        }
        if (*d != '\0') {
            *str = '\0';
            *saveptr = str + 1;
            return token;
        }
        str++;
    }

    *saveptr = NULL;
    return token;
}

#endif // STRING_C