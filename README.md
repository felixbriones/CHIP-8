# CHIP-8
A C-based chip-8 emulator based off the tutorial by Laurence Muller: https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/

Command to build:
gcc main.c -o chip8_emulator -lSDL2

Command to build for debugging:
gcc main.c -o chip8_emulator -lSDL2 -g

Test ROMs used to confirm correct operations:
https://github.com/corax89/chip8-test-rom
https://github.com/Timendus/chip8-test-suite?tab=readme-ov-file

Usage:
./chip8_emulator (path to .rom or .ch8 file)

Debugging (via CGDB):
cgdb chip8_emulator
run (path to .rom or .ch8 file)

