#ifndef STRING_C
#define STRING_C

#include "string.h"

int strcmp(const char *s1, const char *s2){
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

#endif // STRING_C