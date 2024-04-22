#include <stdint.h>
#include <string.h>
#include "main.h"

void initialize_chip(chip8_t* c);

int main(int argc, char** argv)
{
	chip8_t chip;
	initialize_chip(&chip);

	return 0;	
}

void initialize_chip(chip8_t* c)
{
	// Program counter begins @ 0x200
	c->pc = 0x200; 
	// Reset opcode
	c->opcode = 0;
	// Reset index register
	c->i = 0;
	// Reset stack pointer
	c->sp = 0;
	// Clear display
	memset(c->gfx, 0, sizeof(c->gfx));	
	// Clear stack
	memset(c->stack, 0, sizeof(c->stack));	
	// Clear registers V0 - VF 
	memset(c->v, 0, sizeof(c->v));	
	// Clear memory 
	memset(c->memory, 0, sizeof(c->memory));	

	// Load fontset. Should be loaded into memory address 0x50
	for(int i = 0; i < 80; ++i)
	{
		memory[i + OFFSET_FONT] = chip8_fontset[i];	
	}

	// Reset timers
	c->delay_timer = 0;
	c->sound_timer = 0;
}

void emulate_cycle(chip8_t c)
{
	// One opcode is 2 bytes long
	c->opcode = 0;
	
}
