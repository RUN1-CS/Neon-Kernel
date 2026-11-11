#!/usr/bin/env bash

# Build it
make || exit 1

# Run it: boots SSn-DOS.img as a floppy, and attaches fs.img as the IDE disk
# for the filesystem (created on first run, NOT removed by `make clean`)
make run

# Clean it
make clean