/*
 *  NK - A simple kernel written in C
 *  Copyright (C) 2026  RUN1/RUN1-CS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "drivers/video.h"
#include "drivers/keyboard.h"
#include "../apps/xxsh/shell.h"

void kernel_main(void) {
    // 1. Core OS Init
    // init_gdt();
    // init_idt();
    // init_video();
    // init_keyboard();

    clear_screen();

    str_print("Shin Seiki Disk Operating System Boot Sequence Completed\n");

    // 2. Start the shell
    xxsh_loop();

    // Fallback infinite loop to prevent the kernel from exiting
    while(1) {
        __asm__ volatile ("hlt");
    }
}