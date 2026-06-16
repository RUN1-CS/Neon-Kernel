#ifndef VIDEO_H
#define VIDEO_H

// VGA Text Mode Constants
#define VGA_CTRL_REGISTER 0x3d4
#define VGA_DATA_REGISTER 0x3d5
#define VGA_OFFSET_LOW 0x0f
#define VGA_OFFSET_HIGH 0x0e

unsigned char port_byte_in(unsigned short port);

void port_byte_out(unsigned short port, unsigned char data);

void set_cursor(int offset);

int get_cursor();

// VGA Text Mode
#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0f

void set_char_at_video_memory(char character, int offset);

void str_print(char *string);

int get_row_from_offset(int offset);

int get_offset(int col, int row);

int move_offset_to_new_line(int offset);

void memory_copy(char *source, char *dest, int nbytes);

int scroll_ln(int offset);

void clear_line();

void clear_screen();

#endif // VIDEO_H
