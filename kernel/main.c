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

#include "../apps/xxsh/shell.h"
#include "drivers/keyboard.h"
#include "drivers/video.h"

/* Provided by linker.ld */
extern char __bss_start[], __bss_end[];

void __attribute__((section(".text.entry"))) kernel_main(void) {
  for (volatile char *p = __bss_start; p < __bss_end; p++)
    *p = 0;

  clear_screen();

  str_print("Shin Seiki Disk Operating System Boot Sequence Completed\n");

  // Start the shell
  xxsh_loop();

  // Fallback infinite loop to prevent the kernel from exiting
  while (1) {
    __asm__ volatile("hlt");
  }
}