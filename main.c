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
			//draw_graphics();
		}
		
		// Store key press state (Press & release)
		//set_keys();
	}

	return 0;	
}

void initialize_chip(chip8_t* chip)
{
	// Program counter begins @ 0x200
	chip->pc = 0x200; 
	// Reset opcode
	chip->opcode = 0;
	// Reset index register
	chip->i = 0;
	// Reset stack pointer
	chip->sp = 0;
	// Clear display
	memset(chip->gfx, 0, sizeof(chip->gfx));	
	// Clear stack
	memset(chip->stack, 0, sizeof(chip->stack));	
	// Clear registers V0 - VF 
	memset(chip->v, 0, sizeof(chip->v));	
	// Clear memory 
	memset(chip->memory, 0, sizeof(chip->memory));	

	// Load fontset. Should be loaded into memory address 0x50
	for(int i = 0; i < FONTSET_SIZE; ++i)
	{
	//	chip->memory[i + OFFSET_FONT] = chip8_fontset[i];	
	}

	// Reset timers
	chip->delay_timer = 0;
	chip->sound_timer = 0;
}

void emulate_cycle(chip8_t* chip)
{
	// Fetch opcode from memory pointed to by PC
	// Note: Each address has only 1 byte of an opcode, but opcodes are 2 bytes long. Fetch 2 successive bytes and merge them
	chip->opcode = (chip->memory[chip->pc] << 8) | chip->memory[chip->pc + 1];
	
	// Decode opcode. Look at the most significant nibble
	switch(chip->opcode & 0xF000)
	{
		case 0x0000:
			switch(chip->opcode & 0x000F)
			{
				// 0x00E0: Clear screen
				case 0x0000:
					execute_opcode_0x00E0();
					break;
				// 0x00EE: Return from subroutine
				case 0x00EE:
					execute_opcode_0x00EE(chip);
					break;
				default:
					printf("Unknown opcode: %d\r\n", chip->opcode);
			}
		break;
		// 0x1NNN (JP): Jump to subroutine @ NNN 
		case 0x1000:
			execute_opcode_0x1NNN(chip);
			break;	
		// 0x2NNN: Call subroutine @ NNN 
		case 0x2000:
			execute_opcode_0x2NNN(chip);
			break;	
		// 0x3XKK (SE): Skip next instruction if Vx = KK
		case 0x3000:
			execute_opcode_0x3XKK(chip);
			break;	
		// 0x4XKK (SNE): Skip next instruction if Vx != KK
		case 0x4000:
			execute_opcode_0x4XKK(chip);
			break;	
		// 0x5XY0 (SE): Skip next instruction if Vx = Vy
		case 0x5000:
			execute_opcode_0x5XY0(chip);
			break;	
		// 0x6XKK (LD): Places the value KK into register Vx
		case 0x6000:
			execute_opcode_0x6XKK(chip);
			break;	
		// 0x7XKK (ADD): Adds the value kk to the value of register Vx
		case 0x7000:
			execute_opcode_0x7XKK(chip);
			break;	
		case 0x8000:
			switch(chip->opcode & 0x000F)
			{
				// 0x8XY0 (LD): Stores the value of register Vy in register Vx
				case 0x0000:
					execute_opcode_0x8XY0(chip);
					break;
				// 0x8XY1 (OR): OR operation with Vx and Vy. Result stored in Vx
				case 0x0001:
					execute_opcode_0x8XY1(chip);
					break;
				// 0x8XY2 (AND): AND operation with Vx and Vy. Result stored in Vx
				case 0x0002:
					execute_opcode_0x8XY2(chip);
					break;
				// 0x8XY3 (XOR): XOR operation with Vx and Vy. Result stored in Vx
				case 0x0003:
					execute_opcode_0x8XY3(chip);
					break;
				// 0x8XY4 (ADD): Vx = Vx + Vy 
				case 0x0004:
					execute_opcode_0x8XY4(chip);
					break;
				// 0x8XY5 (SUB): Vx = Vx - Vy 
				case 0x0005:
					execute_opcode_0x8XY5(chip);
					break;
				default:
					printf("Unknown opcode: %d\r\n", chip->opcode);
			}

		// 0xDXYN: Draw a sprite at coordinate (value @ Vx, value @ Vy) with a height of n pixels
		case 0xD000:
			execute_opcode_0xDXYN(chip);
			break;
		case 0xE000:
			switch(chip->opcode & 0x00FF)
			{
				// 0xEX9E: Skip next instruction if key with the value of Vx is pressed
				case 0x009E:
					execute_opcode_0xEX9E(chip);
					break;
				// 0xEXA1: Skip next instruction if key with the value of Vx is not pressed
				case 0x00A1:
					execute_opcode_0xEXA1(chip);
					break;
				default:
					printf("Unknown opcode: %d\r\n", chip->opcode);
			}
		case 0xF000:
			switch(chip->opcode & 0x00FF)
			{
				// 0xFX33: Store BCD representation of Vx in memory locations I, I+1, and I+2
				case 0x0033:
					execute_opcode_0xFX33(chip);
					break;
				default:
					printf("Unknown opcode: %d\r\n", chip->opcode);
			}
	}
	
	// Update timers (@60Hz)
	if(chip->delay_timer > 0)
	{
		--chip->delay_timer;
	}

	if(chip->sound_timer > 0)
	{
		--chip->sound_timer;
		if(chip->sound_timer == 1)
		{
			printf("Beep");
		}
	}
		
}

// 0x0000 (CLS): Clear screen
void execute_opcode_0x00E0(void)
{
}

// 0x00EE (RET): Return from subroutine. The interpreter sets the program counter to the address at the top of the stack, 
// then subtracts 1 from the stack pointer.
void execute_opcode_0x00EE(chip8_t* chip)
{
	chip->pc = chip->stack[chip->sp];
	chip->sp--;
}

// 0x1NNN (JP): Jump to subroutine @ NNN 
// Note: Unlike CALL, this only changes PC without updating the stack
void execute_opcode_0x1NNN(chip8_t* chip)
{
	chip->pc = chip->opcode & 0xFFF;
}

// 0x2NNN (CALL): Call subroutine @ NNN 
// Place current address of PC on stack, jump to subroutine, increment SP, and update PC
void execute_opcode_0x2NNN(chip8_t* chip)
{
	chip->stack[chip->sp] = chip->pc;
	chip->sp++;
	chip->pc = chip->opcode & 0xFFF;
}

// 0x3XKK (SE): Skip next instruction if Vx = KK
void execute_opcode_0x3XKK(chip8_t* chip)
{
	uint8_t vx = chip->v[(chip->opcode & 0x0F00) >> 8];
	uint8_t byte = chip->opcode & 0x00FF;

	if(vx == byte)
	{
		chip->pc += 2;
	}
	chip->pc += 2;
}

// 0x4XKK (SNE): Skip next instruction if Vx != KK
void execute_opcode_0x4XKK(chip8_t* chip)
{
	uint8_t vx = chip->v[(chip->opcode & 0x0F00) >> 8];
	uint8_t byte = chip->opcode & 0x00FF;

	if(vx != byte)
	{
		chip->pc += 2;
	}
	chip->pc += 2;
}

// 0x5XY0 (SE): Skip next instruction if Vx = Vy
void execute_opcode_0x5XY0(chip8_t* chip)
{
	uint8_t vx = chip->v[(chip->opcode & 0x0F00) >> 8];
	uint8_t vy = chip->v[(chip->opcode & 0x00F0) >> 4];
	if(vx == vy)
	{
		chip->pc += 2;
	}
	chip->pc += 2;
}

// 0x6XKK (LD): Places the value KK into register Vx
void execute_opcode_0x6XKK(chip8_t* chip)
{
	uint8_t byte = chip->opcode & 0x00FF;
	chip->v[(chip->opcode & 0x0F00) >> 8] = byte;
}

// 0x7XKK (ADD): Adds the value kk to the value of register Vx, then stores the result in Vx. (Vx = Vx + kk)
void execute_opcode_0x7XKK(chip8_t* chip)
{
	uint8_t byte = chip->opcode & 0x00FF;
	chip->v[(chip->opcode & 0x0F00) >> 8] += byte;
}

// 0x8XY0 (LD): Stores the value of register Vy in register Vx (Vx = Vy)
void execute_opcode_0x8XY0(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];
	uint8_t vy = chip->v[(chip->opcode & 0x00F0) >> 4];

	// Load Vy into Vx 
	chip->v[x] = vy;
}

// 0x8XY1 (OR): OR operation with Vx and Vy. Result stored in Vx (Vx = Vx OR Vy)
void execute_opcode_0x8XY1(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];
	uint8_t vy = chip->v[(chip->opcode & 0x00F0) >> 4];

	// Perform OR operation and store in Vx
	chip->v[x] = vx|vy;
}

// 0x8XY2 (AND): AND operation with Vx and Vy. Result stored in Vx (Vx = Vx AND Vy)
void execute_opcode_0x8XY2(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];
	uint8_t vy = chip->v[(chip->opcode & 0x00F0) >> 4];

	// Perform AND operation and store in Vx
	chip->v[x] = vx&vy;
}

// 0x8XY3 (XOR): XOR operation with Vx and Vy. Result stored in Vx (Vx = Vx XOR Vy)
void execute_opcode_0x8XY3(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];
	uint8_t vy = chip->v[(chip->opcode & 0x00F0) >> 4];

	// Perform XOR operation and store in Vx
	chip->v[x] = vx&vy;
}

// 0x8XY4 (ADD): Add Vy to Vx. If sum is greater than 255, VF is set 1 (0 otherwise). Sum stored in Vx 
void execute_opcode_0x8XY4(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];
	uint8_t vy = chip->v[(chip->opcode & 0x00F0) >> 4];

	// Check if sum is greater than 255
	if(vx > 0xFF - vy) 
	{
		chip->v[0xF] = 1;	
	} 
	else
	{
		chip->v[0xF] = 0;	
	}

	chip->v[x] += vy;
	chip->pc += 2;
}

// 0x8XY5 (SUB): If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
void execute_opcode_0x8XY5(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];
	uint8_t vy = chip->v[(chip->opcode & 0x00F0) >> 4];
	
	// Set VF if appropriate 
	chip->v[0xF] = (vx > vy) ? 1 : 0;

	// Subtract
	chip->v[x] -= vy;
	chip->pc += 2;
}

// 0xDXYN: Draw a sprite at coordinate (value @ Vx, value @ Vy) with a height of n pixels (rows). Width locked at 8 pixels. 
// Each row of 8 pixels read as bit coded starting from memory location I 
// This function does not change value of I. Current state of pixel XOR'd with current value in memory. 
// If pixels changed from 1 to 0, VF = 1 (collision detection)
// In other words, set VF if a new sprite collides with what's already on screen
void execute_opcode_0xDXYN(chip8_t* chip)
{
	uint16_t x = chip->v[(chip->opcode & 0x0F00) >> 8];
	uint16_t y = chip->v[(chip->opcode & 0x00F0) >> 4];

	uint16_t height = chip->v[chip->opcode & 0x000F];	
	uint16_t pixel;

	// Reset VF
	chip->v[0xF] = 0;

	// Loop over each row
	for(uint16_t yline = 0; yline < height; yline++)
	{
		// Fetch pixel value starting from memory location i.
		pixel = chip->memory[chip->i + yline];

		// Loop over 8 bits of one row
		for(uint16_t xline = 0; xline < SPRITE_MAX_WIDTH; xline++)
		{
			// Check if the current evaluated pixel is set to 1 (Note that 0x80 >> xline scan through the byte, one bit at the time)
			if(pixel & (0x80 >> xline) != 0	)
			{
				// Check if the pixel on the display is set to 1. If set, we need to register the collision by setting the VF register
				if(chip->gfx[(x + xline + ((y + yline) * GFX_XAXIS))])
				{
					chip->v[0xF] = 1;	
				}
				// Set the pixel value by using XOR
				chip->gfx[(x + xline + ((y + yline) * GFX_XAXIS))] ^= 1;
			}
		}
	}

	// We changed our gfx[] array and thus need to update the screen
	chip->draw_flag = true;
	chip->pc += 2;
}

// 0xEX9E: Skip next instruction if key with the value of Vx is pressed
void execute_opcode_0xEX9E(chip8_t* chip)
{
	chip->pc += 2;
	if(chip->key[chip->v[(chip->opcode & 0x0F00) >> 8]])
	{
		chip->pc += 2;
	}
}

// 0xEXA1: Skip next instruction if key with the value of Vx is not pressed
void execute_opcode_0xEXA1(chip8_t* chip)
{
	chip->pc += 2;
	if(!chip->key[chip->v[(chip->opcode & 0x0F00) >> 8]])
	{
		chip->pc += 2;
	}
}

// 0xFX33: Store BCD representation of Vx in memory locations I, I+1, and I+2
// Example: Integer = 143.  memory[i] = 1, memory[i+1] = 4, memory[i+2] = 3
void execute_opcode_0xFX33(chip8_t* chip)
{
	chip->memory[chip->i] = chip->v[(chip->opcode & 0x0F00) >> 8] / 100;
	chip->memory[chip->i + 1] = (chip->v[(chip->opcode & 0x0F00) >> 8] / 10) % 10;
	chip->memory[chip->i + 2] = chip->v[(chip->opcode & 0x0F00) >> 8] % 10;
	chip->pc += 2;
}
