#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "main.h"

void initialize_chip(chip8_t* c);

int main(int argc, char** argv)
{
	chip8_t chip;


	//setup_graphics();
	//setup_input();
	
	initialize_chip(&chip);
	//load_game();

	for(;;)
	{
		// Emulate 1 cycle	
		emulate_cycle(&chip);

		// Update screen if draw flag is set
		if(chip.draw_flag)
		{
			draw_graphics();
		}
		
		// Store key press state (Press & release)
		set_keys();
	}

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
				// 0x00EE: Return from subroutine. The interpreter sets the program counter to the address at the top of the stack, 
				// then subtracts 1 from the stack pointer.
				case 0x000E:
					c->pc = c->stack[c->sp];
					c->sp--;
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
				default:
					printf("Unknown opcode: %d\r\n", c->opcode);
			}

		// 0xDXYN: Draw a sprite at coordinate (value @ Vx, value @ Vy) with a height of n pixels (rows). Width locked at 8 pixels. 
		// Each row of 8 pixels read as bit coded starting from memory location I 
		// This function does not change value of I. Current state of pixel XOR'd with current value in memory. 
		// If pixels changed from 1 to 0, VF = 1 (collision detection)
		// In other words, set VF if a new sprite collides with what's already on screen
		case 0xD000:
			uint16_t x = c->v[(c->opcode & 0x0F00) >> 8];
			uint16_t y = c->v[(c->opcode & 0x00F0) >> 4];

			uint16_t height = c->v[c->opcode & 0x000F];	
			uint16_t pixel;

			// Reset VF
			c->v[0xF] = 0;

			// Loop over each row
			for(uint16_t yline = 0; yline < height; yline++)
			{
				// Fetch pixel value starting from memory location i.
				pixel = c->memory[c->i + yline];

				// Loop over 8 bits of one row
				for(uint16_t xline = 0; xline < SPRITE_MAX_WIDTH; xline++)
				{
					// Check if the current evaluated pixel is set to 1 (Note that 0x80 >> xline scan through the byte, one bit at the time)
					if(pixel & (0x80 >> xline) != 0	)
					{
						// Check if the pixel on the display is set to 1. If set, we need to register the collision by setting the VF register
						if(c->gfx[(x + xline + ((y + yline) * GFX_XAXIS))])
						{
							c->v[0xF] = 1;	
						}
						// Set the pixel value by using XOR
						c->gfx[(x + xline + ((y + yline) * GFX_XAXIS))] ^= 1;
					}
				}
			}

			// We changed our gfx[] array and thus need to update the screen
			c->draw_flag = true;
			c->pc += 2;

			break;
		case 0xE000:
			switch(c->opcode & 0x00FF)
			{
				// 0xEX9E: Skip next instruction if key with the value of Vx is pressed
				case 0x009E:
					c->pc += 2;
					if(c->key[c->v[(c->opcode & 0x0F00) >> 8]])
					{
						c->pc += 2;
					}
					break;
				// 0xEXA1: Skip next instruction if key with the value of Vx is not pressed
				case 0x00A1:
					c->pc += 2;
					if(!c->key[c->v[(c->opcode & 0x0F00) >> 8]])
					{
						c->pc += 2;
					}
					break;
				default:
					printf("Unknown opcode: %d\r\n", c->opcode);
			}
		case 0xF000:
			switch(c->opcode & 0x00FF)
			{
				// 0xFX33: Store BCD representation of Vx in memory locations I, I+1, and I+2
				case 0x0033:
					c->memory[c->i] = c->v[(c->opcode & 0x0F00) >> 8] / 100;
					c->memory[c->i + 1] = (c->v[(c->opcode & 0x0F00) >> 8] / 10) % 10;
					c->memory[c->i + 2] = c->v[(c->opcode & 0x0F00) >> 8] % 10;
					c->pc += 2;
					break;
				default:
					printf("Unknown opcode: %d\r\n", c->opcode);
			}
	}
	
	// Update timers (@60Hz)
	if(c->delay_timer > 0)
	{
		--c->delay_timer;
	}

	if(c->sound_timer > 0)
	{
		--c->sound_timer;
		if(sound_timer == 1)
		{
			printf("Beep");
		}
	}
		
}
