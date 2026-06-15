#ifndef VIDEO_C
#define VIDEO_C

#include "video.h"

// Port I/O functions
unsigned char port_byte_in(unsigned short port){
    unsigned char result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void port_byte_out(unsigned short port, unsigned char data){
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

// Setting Cursor Position
void set_cursor(int offset){
    offset /= 2; // Convert from cell offset to character offset
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
    port_byte_out(VGA_DATA_REGISTER, (unsigned char)(offset & 0xFF));
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
    port_byte_out(VGA_DATA_REGISTER, (unsigned char)((offset >> 8) & 0xFF));
}

// Getting Cursor Position
int get_cursor(){
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
    int offset = port_byte_in(VGA_DATA_REGISTER) << 8;
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
    offset += port_byte_in(VGA_DATA_REGISTER);
    return offset * 2; // Convert from character offset to cell offset
}

// Text Handeling

// Printing one character at a time
void set_char_at_video_memory(char character, int offset){
    unsigned char *vidmem = (unsigned char *) VIDEO_ADDRESS;
    vidmem[offset] = character;
    vidmem[offset + 1] = WHITE_ON_BLACK;
}

// Printing Text
void str_print(char *string){
    int offset = get_cursor();
    int i = 0;
    while (string[i] != 0) {
        if (offset >= MAX_ROWS * MAX_COLS * 2) {
            offset = scroll_ln(offset);
        }
        if (string[i] == '\n') {
            offset = move_offset_to_new_line(offset);
        } else {
            set_char_at_video_memory(string[i], offset);
            offset += 2;
        }
        i++;
    }
    set_cursor(offset);
}

// Handeling \n
int get_row_from_offset(int offset){
    return offset / (2 * MAX_COLS);
}

int get_offset(int col, int row){
    return 2 * (row * MAX_COLS + col);
}

int move_offset_to_new_line(int offset){
    return get_offset(0, get_row_from_offset(offset) + 1);
}

// Scrolling
void memory_copy(char *source, char *dest, int nbytes){
    int i;
    for (i = 0; i < nbytes; i++) {
        *(dest + i) = *(source + i);
    }
}

int scroll_ln(int offset){
    memory_copy(
            (char *) (get_offset(0, 1) + VIDEO_ADDRESS),
            (char *) (get_offset(0, 0) + VIDEO_ADDRESS),
            MAX_COLS * (MAX_ROWS - 1) * 2
    );

    for (int col = 0; col < MAX_COLS; col++) {
        set_char_at_video_memory(' ', get_offset(col, MAX_ROWS - 1));
    }

    return offset - 2 * MAX_COLS;
}

// Clearing Screen
void clear_screen(){
    for (int i = 0; i < MAX_COLS * MAX_ROWS; ++i) {
        set_char_at_video_memory(' ', i * 2);
    }
    set_cursor(get_offset(0, 0));
}

#endif