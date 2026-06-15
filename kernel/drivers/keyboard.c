#ifndef KEYBOARD_C
#define KEYBOARD_C

#include "keyboard.h"
#include "video.h"

#include <stddef.h>

// Access to hardware I/O ports
unsigned char inb(unsigned short port){
    unsigned char result;
    // Read a byte from 'port' and store it in 'result'
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Write a byte to a hardware I/O port
void outb(unsigned short port, unsigned char data){
    // Write 'data' out to 'port'
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

// Poll the keyboard controller to get the next scancode
unsigned char get_scancode(void){
    // Bit 0 (0x01) of port 0x64 indicates the output buffer is full (data is ready)
    while ((inb(0x64) & 0x01) == 0) {
        // Loop endlessly (poll) until a key is pressed
    }
    // Read and return the raw scancode from the data port
    return inb(0x60);
}

// Translation array for standard US Keyboard Layout (Set 1)
const char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 0x00 - 0x09 */
  '9', '0', '-', '=', '\b', '\t', /* Backspace, Tab */
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',	/* 0x0A - 0x13 */
  '[', ']', '\n',    0,			/* Enter, Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 0x14 - 0x1D */
 '\'', '`',   0, '\\', 'z', 'x', 'c', 'v', 'b', 'n',	/* 0x1E - 0x27 */
  'm', ',', '.', '/',   0, '*',    0, ' ',   0		/* 0x28 - 0x32 */
};

// Function to get a readable character from the keyboard
char keyboard_get_char(void){
    unsigned char scancode = get_scancode();

    // Ignore key release events for now (highest bit is set)
    if (scancode & RELEAS_BIT) {
        return 0; 
    }

    // Safety bounds check against our translation array size
    if (scancode >= sizeof(scancode_to_ascii)) {
        return 0;
    }

    // Return the translated readable character
    return scancode_to_ascii[scancode];
}

// Read a line of input from the keyboard with a prompt
char *readline(const char *prompt, char *buffer) {
    // Print the prompt to the screen
    str_print((char *)prompt);

    // Prepare buffer for the input line
    buffer[0] = '\0'; // Initialize the buffer to an empty string

    size_t length = 0;
    while (1) {
        char c = keyboard_get_char();
        if (c == '\n' || c == '\r') { // Enter key pressed
            buffer[length] = '\0'; // Null-terminate the string
            str_print("\n"); // Move to the next line
            return buffer;
        } else if (c == '\b') { // Backspace key pressed
            if (length > 0) {
                length--;
                str_print("\b \b"); // Erase the last character from the screen
            }
        } else if (c != 0) { // Valid character received
            if (length + 1 >= KEYBOARD_BUFFER_SIZE) {
                str_print("\nInput too long!\n");
                buffer[0] = '\0'; // Reset the buffer
                length = 0;
            }
            buffer[length++] = c; // Add character to buffer
            char echo_buffer[2];
            echo_buffer[0] = c;
            echo_buffer[1] = '\0';
            str_print(echo_buffer); // Echo the character to the screen
        }
    }

    return buffer;
}

#endif