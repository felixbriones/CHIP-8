#include <stdint.h>
#include <stdio.h>
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
	for(int i = 0; i < FONTSET_SIZE; ++i)
	{
	//	c->memory[i + OFFSET_FONT] = chip8_fontset[i];	
	}

	// Reset timers
	c->delay_timer = 0;
	c->sound_timer = 0;
}

void emulate_cycle(chip8_t* c)
{
	// Fetch opcode from memory pointed to by PC
	// Note: Each address has only 1 byte of an opcode, but opcodes are 2 bytes long. Fetch 2 successive bytes and merge them
	c->opcode = (c->memory[c->pc] << 8) | c->memory[c->pc + 1];
	
	// Decode opcode. Look at the most significant nibble
	switch(c->opcode & 0xF000)
	{
		case 0x0000:
			switch(c->opcode & 0x000F)
			{
				// 0x00E0: Clear screen
				case 0x0000:
					break;
				// 0x00EE: Return from subroutine
				case 0x000E:
					break;
				default:
					printf("Unknown opcode: %d\r\n", c->opcode);
			}
		break;
		// 0x2nnn: Call nnn
		case 0x2000:
			// Place current address of PC on stack, jump to subroutine, increment SP, and update PC
			c->stack[c->sp] = c->pc;
			c->sp++;
			c->pc = c->opcode & 0xFFF;
			break;	
		// Perform mathematical operation
		case 0x8000:
			switch(c->opcode & 0x000F)
			{
				// 0x0XY4: Add Vy to Vx. If sum is greater than 255, VF is set 1 (0 otherwise). Sum stored in X
				case 0x0004:
					// Vx > 255 - Vy
					if(c->v[(c->opcode & 0x0F00) >> 8] > 0xFF - c->v[(c->opcode & 0x00F0) >> 4])
					{
						c->v[0xF] = 1;	
					} 
					else
					{
						c->v[0xF] = 0;	
					}

					c->v[(c->opcode & 0x0F00) >> 8] += c->v[(c->opcode & 0x00F0) >> 4];
					c->pc += 2;
					break;
					
	
			}
			
	}
		
}
