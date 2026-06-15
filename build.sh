#!/bin/bash

# Build it
make

# Run it
qemu-system-x86_64 ./SSn-DOS.img -serial stdio

# Clean it
make clean