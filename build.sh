#!/usr/bin/env bash

# Build it
make

# Run it
qemu-system-x86_64 -drive format=raw,file=./SSn-DOS.img -serial stdio

# Clean it
make clean