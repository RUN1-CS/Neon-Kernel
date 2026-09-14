#ifndef IO_H
#define IO_H

#include <stdint.h>

// Low-level I/O port operations

// Write a byte to the specified I/O port
static inline void port_byte_out(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Read a byte from the specified I/O port
static inline uint8_t port_byte_in(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Read a word from the specified I/O port
static inline uint16_t port_word_in(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Write a word to the specified I/O port
static inline void port_word_out(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}


#endif // IO_H