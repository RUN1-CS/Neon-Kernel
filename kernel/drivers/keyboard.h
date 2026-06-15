#ifndef KEYBOARD_H
#define KEYBOARD_H

#define RELEAS_BIT 0x80
#define KEYBOARD_BUFFER_SIZE 128

unsigned char inb(unsigned short port);

void outb(unsigned short port, unsigned char data);

unsigned char get_scancode(void);

char keyboard_get_char(void);

char * readline(const char *prompt, char *buffer);

#endif