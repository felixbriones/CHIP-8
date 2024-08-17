#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "main.h"

void initialize_chip(chip8_t* c);

int main(int argc, char** argv)
{
	chip8_t chip;
	SDL_Event event;
	SDL_Window* window = NULL;
	SDL_Renderer* render = NULL;

	// If no ROM file is provided, print usage and terminate
	if(argc != 2)
	{
		printf("Usage: %s\n", argv[0]);
		return 1;
	}

	setup_graphics(&window, &render);
	initialize_chip(&chip);
	load_game(&chip, argv[1]);

	for(;;)
	{
		// Emulate 1 cycle	
		emulate_cycle(&chip);

		// Update screen if draw flag is set
		if(chip.draw_flag)
		{
			draw_graphics(&window, &render, &chip);
			chip.draw_flag = false;
		}
		
		SDL_Delay(DELAY_SDL_60FPS);

		// Store key press state (Press & release)
		setup_input(&chip, &event);
		// set_keys();
	}

	return 0;	
}

void setup_input(chip8_t* chip, SDL_Event* event)
{
	// Poll for currently pending events, grabbing next one from event queue if available. Returns 0 if there are none
	// Automatically removes event in question from queue
	while(SDL_PollEvent(event) != 0)
	{
		// Examines what type of event is being examined
		switch(event->type)		
		{
			// Quit if escape key is press. Note: These macros are defined by the SDL SDK
			//case SDLK_ESCAPE:
			case SDL_QUIT:
				exit(0);
				break;
			break;
			// Take action if key is pressed down
			case SDL_KEYDOWN:
				switch (event->key.keysym.sym)
				{
					case SDLK_1:
						chip->key[0x0] = 1;
						break;
					case SDLK_2:
						chip->key[0x1] = 1;
						break;
					case SDLK_3:
						chip->key[0x2] = 1;
						break;
					case SDLK_4:
						chip->key[0x3] = 1;
						break;
					case SDLK_q:
						chip->key[0x4] = 1;
						break;
					case SDLK_w:
						chip->key[0x5] = 1;
						break;
					case SDLK_e:
						chip->key[0x6] = 1;
						break;
					case SDLK_r:
						chip->key[0x7] = 1;
						break;
					case SDLK_a:
						chip->key[0x8] = 1;
						break;
					case SDLK_s:
						chip->key[0x9] = 1;
						break;
					case SDLK_d:
						chip->key[0xA] = 1;
						break;
					case SDLK_f:
						chip->key[0xB] = 1;
						break;
					case SDLK_z:
						chip->key[0xC] = 1;
						break;
					case SDLK_x:
						chip->key[0xD] = 1;
						break;
					case SDLK_c:
						chip->key[0xE] = 1;
						break;
					case SDLK_v:
						chip->key[0xF] = 1;
						break;
					break;
				}
				break;
			// Take action if key is released 
			case SDL_KEYUP:
				switch (event->key.keysym.sym)
				{
					case SDLK_1:
						chip->key[0x0] = 0;
						break;
					case SDLK_2:
						chip->key[0x1] = 0;
						break;
					case SDLK_3:
						chip->key[0x2] = 0;
						break;
					case SDLK_4:
						chip->key[0x3] = 0;
						break;
					case SDLK_q:
						chip->key[0x4] = 0;
						break;
					case SDLK_w:
						chip->key[0x5] = 0;
						break;
					case SDLK_e:
						chip->key[0x6] = 0;
						break;
					case SDLK_r:
						chip->key[0x7] = 0;
						break;
					case SDLK_a:
						chip->key[0x8] = 0;
						break;
					case SDLK_s:
						chip->key[0x9] = 0;
						break;
					case SDLK_d:
						chip->key[0xA] = 0;
						break;
					case SDLK_f:
						chip->key[0xB] = 0;
						break;
					case SDLK_z:
						chip->key[0xC] = 0;
						break;
					case SDLK_x:
						chip->key[0xD] = 0;
						break;
					case SDLK_c:
						chip->key[0xE] = 0;
						break;
					case SDLK_v:
						chip->key[0xF] = 0;
						break;
					break;
				}
				break;
		}
	}
}

/* @brief: After initializing the system, load ROM into memory
 * @arg chip:
 * @arg game_rom:
 * @return: */
void load_game(chip8_t* chip, const char* game_rom)
{
	// Open file in read-only/binary mode
	FILE* file = fopen(game_rom, "rb");
	uint32_t rom_size = 0;

	if(file == NULL)
	{
		printf("Could not open game file\n");
		exit(1);
	} 

	// Move file pointer to end of file
	fseek(file, 0, SEEK_END);
	// Get the current position of the file pointer, which is file size
	rom_size = ftell(file);
	 // Reset file pointer to start. Note: Mandatory, otherwise we won't read from beginning of file
    	fseek(file, 0, SEEK_SET);
	
	// Exit if ROM is too large to store in memory
	if(rom_size > SIZE_MEMORY - GAME_START_ADDRESS)
	{
		printf("Game ROM is too large. Exiting...\n");
		fclose(file);
		exit(1);
	}

	// Copy game logic into memory, starting at memory address 0x200
	fread(chip->memory + GAME_START_ADDRESS, sizeof(uint8_t), rom_size, file);
	fclose(file);
}

int setup_graphics(SDL_Window** window, SDL_Renderer** renderer)
{
	int retval = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	*window = SDL_CreateWindow("Felix's CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		GFX_XAXIS * GFX_SCALE, GFX_YAXIS * GFX_SCALE, SDL_WINDOW_SHOWN);
	// Create render for var: window, initialize using first rendering driver available which supports requested features. Use hardware accel if possible
	*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
	printf("SDL_Init completed with code: %d\n", retval);
	return retval;
}

int draw_graphics(SDL_Window** window, SDL_Renderer** renderer, chip8_t* chip)
{
	// Set color for clearing screen to black
	SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);
	// Clear the renderer with the specified color
	SDL_RenderClear(*renderer);
	// Set color for clearing screen to white
	SDL_SetRenderDrawColor(*renderer, 0, 255, 255, 255);

	// Loop through each pixel in the display
	for(uint8_t y = 0; y < GFX_YAXIS; y++)
	{
		for(uint8_t x = 0; x < GFX_XAXIS; x++)
		{
			// If the rectangle is set, draw a white rectangle at its location
			// display[y * 64 + x] != 0
			if(chip->gfx[y * GFX_XAXIS + x])
			{
				// Define the rectangle representing the pixel. A pixel in CHIP-8 is set to 10 pixels
				// {x coordinate, y coordinate, width, height}
				SDL_Rect pixel = {x * PIXEL_WIDTH, y * PIXEL_HEIGHT, PIXEL_WIDTH, PIXEL_HEIGHT};	
				// Draw the rectangle on the renderer
				SDL_RenderFillRect(*renderer, &pixel);
			}
		}	
	}	

	// Present the rendered display
	SDL_RenderPresent(*renderer);
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
	// Clear draw flag 
	chip->draw_flag = false;
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
		chip->memory[i + OFFSET_FONT] = chip8_fontset[i];	
	}

	// Reset timers
	chip->delay_timer = 0;
	chip->sound_timer = 0;
}

void emulate_cycle(chip8_t* chip)
{
	// Felix: Fetch
	// Fetch opcode from memory pointed to by PC
	// Note: Each address has only 1 byte of an opcode, but opcodes are 2 bytes long. Fetch 2 successive bytes and merge them
	chip->opcode = (chip->memory[chip->pc] << 8) | chip->memory[chip->pc + 1];
	printf("Fetched opcode 0x: %04X\n", chip->opcode);
	printf("Program counter 0x: %04X\n", chip->pc);
	
	// Felix: Decode
	// Decode opcode. Look at the most significant nibble
	switch(chip->opcode & 0xF000)
	{
		case 0x0000:
			switch(chip->opcode & 0x000F)
			{
				// 0x00E0: Clear screen
				case 0x0000:
					execute_opcode_0x00E0(chip);
					break;
				// 0x00EE: Return from subroutine
				case 0x000E:
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
				// 0x8XY6 (SHR): If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
				case 0x0006:
					execute_opcode_0x8XY6(chip);
					break;
				// 0x8XY7 (SUBN): If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
				case 0x0007:
					execute_opcode_0x8XY7(chip);
					break;
				// 0x8XYE (SHL): If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
				case 0x000E:
					execute_opcode_0x8XYE(chip);
					break;
				default:
					printf("Unknown opcode: %d\r\n", chip->opcode);
			}
			break;
		// 0x9XY0 (SNE): The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
		case 0x9000:
			execute_opcode_0x9XY0(chip);
			break;
		// 0xANNN (LD): The value of register I is set to NNN
		case 0xA000:
			execute_opcode_0xANNN(chip);
			break;
		// 0xBNNN (JMP): The program counter is set to nnn plus the value of V0
		case 0xB000:
			execute_opcode_0xBNNN(chip);
			break;
		// 0xCXKK (RND): Set Vx = random byte AND kk. 
		case 0xC000:
			execute_opcode_0xCXKK(chip);
			break;
		// 0xDXYN (DRW): Draw a sprite at coordinate (value @ Vx, value @ Vy) with a height of n pixels
		case 0xD000:
			execute_opcode_0xDXYN(chip);
			break;
		case 0xE000:
			switch(chip->opcode & 0x00FF)
			{
				// 0xEX9E (SKP): Skip next instruction if key with the value of Vx is pressed
				case 0x009E:
					execute_opcode_0xEX9E(chip);
					break;
				// 0xEXA1 (SKNP): Skip next instruction if key with the value of Vx is not pressed
				case 0x00A1:
					execute_opcode_0xEXA1(chip);
					break;
				default:
					printf("Unknown opcode: %d\r\n", chip->opcode);
			}
		case 0xF000:
			switch(chip->opcode & 0x00FF)
			{
				// 0xFX07 (LD): The value of DT is placed into Vx.
				case 0x0007:
					execute_opcode_0xFX07(chip);
					break;
				// 0xFX0A (LD): Wait for a key press, store the value of the key in Vx.
				case 0x000A:
					execute_opcode_0xFX0A(chip);
					break;
				// 0xFX15 (LD): DT is set equal to the value of Vx.
				case 0x0015:
					execute_opcode_0xFX15(chip);
					break;
				// 0xFX18 (LD): ST is set equal to the value of Vx.
				case 0x0018:
					execute_opcode_0xFX18(chip);
					break;
				// 0xFX1E (ADD): The values of I and Vx are added, and the results are stored in I.
				case 0x001E:
					execute_opcode_0xFX1E(chip);
					break;
				// 0xFX29 (LD): Set I = location of sprite for digit Vx.
				case 0x0029:
					execute_opcode_0xFX29(chip);
					break;
				// 0xFX33 (LD): Store BCD representation of Vx in memory locations I, I+1, and I+2
				case 0x0033:
					execute_opcode_0xFX33(chip);
					break;
				// 0xFX55 (LD): Store registers V0 through Vx in memory starting at location I.
				case 0x0055:
					execute_opcode_0xFX55(chip);
					break;
				// 0xFX65 (LD): Read registers V0 through Vx from memory starting at location I.
				case 0x0065:
					execute_opcode_0xFX65(chip);
					break;
				default:
					printf("Unknown opcode: %d\r\n", chip->opcode);
			}
	}
	
	// Update timers (@60Hz)
	handle_delay_timer(chip);
	handle_sound_timer(chip);
}

void handle_delay_timer(chip8_t* chip)
{
	if(chip->delay_timer == 0)
	{
		return;
	}
	--chip->delay_timer;
}

void handle_sound_timer(chip8_t* chip)
{
	if(chip->delay_timer == 0)
	{
		return;
	}
	if(chip->delay_timer == 1)
	{
		printf("beep");
	}
	--chip->sound_timer;
}

// 0x0000 (CLS): Clear screen
void execute_opcode_0x00E0(chip8_t* chip)
{
	memset(chip->gfx, 0, sizeof(chip->gfx));
	chip->pc += 2;
}

//Felix
// 0x00EE (RET): Return from subroutine. The interpreter sets the program counter to the address at the top of the stack, 
// then subtracts 1 from the stack pointer.
void execute_opcode_0x00EE(chip8_t* chip)
{
	chip->sp--;
	chip->pc = chip->stack[chip->sp];
	chip->pc += 2;
}

// 0x1NNN (JP): Jump to subroutine @ NNN 
// Note: Unlike CALL, this only changes PC without updating the stack
void execute_opcode_0x1NNN(chip8_t* chip)
{
	chip->pc = chip->opcode & 0xFFF;
}

// Felix: 
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
	chip->pc += 2;
}

// 0x7XKK (ADD): Adds the value kk to the value of register Vx, then stores the result in Vx. (Vx = Vx + kk)
void execute_opcode_0x7XKK(chip8_t* chip)
{
	uint8_t byte = chip->opcode & 0x00FF;
	chip->v[(chip->opcode & 0x0F00) >> 8] += byte;
	chip->pc += 2;
}

// 0x8XY0 (LD): Stores the value of register Vy in register Vx (Vx = Vy)
void execute_opcode_0x8XY0(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];
	uint8_t vy = chip->v[(chip->opcode & 0x00F0) >> 4];

	// Load Vy into Vx 
	chip->v[x] = vy;
	chip->pc += 2;
}

// 0x8XY1 (OR): OR operation with Vx and Vy. Result stored in Vx (Vx = Vx OR Vy)
void execute_opcode_0x8XY1(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];
	uint8_t vy = chip->v[(chip->opcode & 0x00F0) >> 4];

	// Perform OR operation and store in Vx
	chip->v[x] = vx|vy;
	chip->pc += 2;
}

// 0x8XY2 (AND): AND operation with Vx and Vy. Result stored in Vx (Vx = Vx AND Vy)
void execute_opcode_0x8XY2(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];
	uint8_t vy = chip->v[(chip->opcode & 0x00F0) >> 4];

	// Perform AND operation and store in Vx
	chip->v[x] = vx&vy;
	chip->pc += 2;
}

// 0x8XY3 (XOR): XOR operation with Vx and Vy. Result stored in Vx (Vx = Vx XOR Vy)
void execute_opcode_0x8XY3(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];
	uint8_t vy = chip->v[(chip->opcode & 0x00F0) >> 4];

	// Perform XOR operation and store in Vx
	chip->v[x] = vx^vy;
	chip->pc += 2;
}

// 0x8XY4 (ADD): Add Vy to Vx. If sum is greater than 255, VF is set 1 (0 otherwise). Sum stored in Vx 
void execute_opcode_0x8XY4(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];
	uint8_t y = (chip->opcode & 0x00F0) >> 4;
	uint8_t vy = chip->v[y];
	uint16_t sum = vx + vy;

	// Check if sum is greater than 255
	if(sum > 255) 
	{
		chip->v[0xF] = 1;	
	} 
	else
	{
		chip->v[0xF] = 0;	
	}

	chip->v[x] = sum & 0xFF;
	chip->pc += 2;
}

// 0x8XY5 (SUB): If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
// Set Vx = Vx - Vy, set VF = NOT borrow.
void execute_opcode_0x8XY5(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];
	uint8_t y = (chip->opcode & 0x00F0) >> 4;
	uint8_t vy = chip->v[y];

	// Check if sum is greater than 255
	if(vx > vy) 
	{
		chip->v[0xF] = 1;	
	} 
	else
	{
		chip->v[0xF] = 0;	
	}

	chip->v[x] = vx - vy;
	chip->pc += 2;
}

// 0x8XY6 (SHR): If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
void execute_opcode_0x8XY6(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];

	// Set VF if appropriate 
	chip->v[0xF] = (vx & 0x1) ? 1 : 0;
	
	// Divide Vx by 2
	chip->v[x] /= 2;
	chip->pc += 2;
}

// 0x8XY7 (SUBN): If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
// Set Vx = Vy - Vx, set VF = NOT borrow.
void execute_opcode_0x8XY7(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];
	uint8_t vy = chip->v[(chip->opcode & 0x00F0) >> 4];

	// Set VF if appropriate 
	chip->v[0xF] = (vy > vx) ? 1 : 0;
	
	// Subtract Vy from Vx
	chip->v[x] = vy - vx;
	chip->pc += 2;
}

// 0x8XYE (SHL): If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
// Set Vx = Vx SHL 1.
void execute_opcode_0x8XYE(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t vx = chip->v[x];

	// Set VF if MSB is set
	chip->v[0xF] = (vx & 0x80 == 0x80) ? 1 : 0;
	
	// Multiply Vx by 2
	chip->v[x] *= 2;
	chip->pc += 2;
}

// 0x9XY0 (SNE): The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
// Skip next instruction if Vx != Vy.
void execute_opcode_0x9XY0(chip8_t* chip)
{
	uint8_t vx = chip->v[(chip->opcode & 0x0F00) >> 8];
	uint8_t vy = chip->v[(chip->opcode & 0x00F0) >> 4];

	// Skip next instruction if Vx and Vy are not equal
	if(vx != vy)
	{
		chip->pc += 2;
	}
	chip->pc += 2;
}

// 0xANNN (LD): The value of register I is set to NNN
// Set I = nnn.
void execute_opcode_0xANNN(chip8_t* chip)
{
	chip->i = chip->opcode & 0xFFF;
	chip->pc += 2;
}

// 0xBNNN (JP): The program counter is set to nnn plus the value of V0
// Jump to location nnn + V0
void execute_opcode_0xBNNN(chip8_t* chip)
{
	uint16_t nibbles = chip->opcode & 0xFFF;
	chip->pc = nibbles + chip->v[0];
}

// 0xCXKK (RND): The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk. The results are stored in Vx. 
// Set Vx = random byte AND kk.
void execute_opcode_0xCXKK(chip8_t* chip)
{
	srand(time(NULL));	
	uint8_t random = rand() % 255;
	uint8_t byte = chip->opcode & 0xFF;	
	uint8_t x = (chip->opcode & 0xF00) >> 8;

	// AND random number w/ kk
	chip->v[x] = random & byte;
	chip->pc += 2;
}

// 0xDXYN (DRW): Draw a sprite at coordinate (value @ Vx, value @ Vy) with a height of n pixels (rows). Width locked at 8 pixels. 
// Each row of 8 pixels read as bit coded starting from memory location I 
// This function does not change value of I. Current state of pixel XOR'd with current value in memory. 
// If pixels changed from 1 to 0, VF = 1 (collision detection)
// In other words, set VF if a new sprite collides with what's already on screen
void execute_opcode_0xDXYN(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t y = (chip->opcode & 0x00F0) >> 4;
	uint8_t height = chip->opcode & 0x000F;	

	uint8_t sprite_byte;
	uint8_t sprite_pixel;
	uint32_t screen_pixel;
	// Wrap around if attempting to draw off-screen per specification
	uint8_t x_coor = chip->v[x] % GFX_XAXIS;
	uint8_t y_coor = chip->v[y] % GFX_YAXIS;

	// Reset VF
	chip->v[0xF] = 0;

	// Loop over each row
	for(uint8_t row = 0; row < height; row++)
	{
		// Fetch pixel value starting from memory location i.
		sprite_byte = chip->memory[chip->i + row];

		// Loop over 8 bits of one row. Sprites are always 8 pixels wide
		for(uint8_t column = 0; column < SPRITE_MAX_WIDTH; column++)
		{
			// Target each individual bit/pixel of sprite_byte for an AND operation
			sprite_pixel = sprite_byte & (0x80 >> column);
			// Determine where on screen we will draw the sprite
			// x_coor, y_coor = starting positions / row/column = current sprite offset
			// We multiply by VIDEO_WIDTH to properly convert 2D screen to 1D array. (i.e. first pixel of second row is index 64)
			screen_pixel = (y_coor + row) * GFX_XAXIS + (x_coor + column);
			
			// Check if isolated bit/pixel is set
			if (sprite_pixel != 0)
			{
				// 0xFFFFFFFF = white pixel. If newly-calculated sprite and current state of pixel are both on, a collision has occurred
				if(chip->gfx[screen_pixel] == 0xFFFFFFFF)
				{
					chip->v[0xF] = 1;	
				}
				// If pixel is white, turn it black and vice versa. Specification indicates toggling pixel if collision occurs
				chip->gfx[screen_pixel] ^= 0xFFFFFFFF;
			}
		}
	}

	// We changed our gfx[] array and thus need to update the screen
	chip->draw_flag = true;
	chip->pc += 2;
}

// 0xEX9E (SKP): Skip next instruction if key with the value of Vx is pressed.
// Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2
void execute_opcode_0xEX9E(chip8_t* chip)
{	
	chip->pc += 2;
	if(chip->key[chip->v[(chip->opcode & 0x0F00) >> 8]])
	{
		chip->pc += 2;
	}
}

// 0xEXA1 (SKNP): Skip next instruction if key with the value of Vx is not pressed.
// Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
void execute_opcode_0xEXA1(chip8_t* chip)
{
	chip->pc += 2;
	if(!chip->key[chip->v[(chip->opcode & 0x0F00) >> 8]])
	{
		chip->pc += 2;
	}
}

// 0xFX07 (LD): The value of DT is placed into Vx.
// Set Vx = delay timer value.
void execute_opcode_0xFX07(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;

	chip->v[x] = chip->delay_timer;
	chip->pc += 2;
}

// 0xFX0A (LD): Wait for a key press, store the value of the key in Vx.
// All execution stops until a key is pressed, then the value of that key is stored in Vx.
void execute_opcode_0xFX0A(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t key_press;

	// Check all 15 keys for a key press
	for(uint8_t i = 0; i < NUM_KEYS; i++)
	{
		key_press = chip->key[i];
		if(key_press)
		{
			chip->v[x] = key_press;
			// Only increment once a key has been pressed to simulate waiting 
			chip->pc += 2;
			return;
		}
	}
}

// 0xFX15 (LD): DT is set equal to the value of Vx.
// Set delay timer = Vx.
void execute_opcode_0xFX15(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;

	chip->delay_timer = chip->v[x];
	chip->pc += 2;
}

// 0xFX18 (LD): ST is set equal to the value of Vx.
// Set sound timer = Vx.
void execute_opcode_0xFX18(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;

	chip->sound_timer = chip->v[x];
	chip->pc += 2;
}

// 0xFX1E (ADD): The values of I and Vx are added, and the results are stored in I.
// Set I = I + Vx.
void execute_opcode_0xFX1E(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;

	chip->i += chip->v[x];
	chip->pc += 2;
}

// 0xFX29 (LD): The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx. See section 2.4, Display, for more information on the Chip-8 hexadecimal font.
// Set I = location of sprite for digit Vx.
void execute_opcode_0xFX29(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;
	uint8_t digit = chip->v[x];

	// Fontset begins @ Memory address 0x50, starting with 0. Each individual digit is 5 bytes in size
	chip->i = (digit * SIZE_FONT_CHAR) + OFFSET_FONT;
	chip->pc += 2;
}

// 0xFX33 (LD): Store BCD representation of Vx in memory locations I, I+1, and I+2
// Example: Integer = 143.  memory[i] = 1, memory[i+1] = 4, memory[i+2] = 3
void execute_opcode_0xFX33(chip8_t* chip)
{
	chip->memory[chip->i] = chip->v[(chip->opcode & 0x0F00) >> 8] / 100;
	chip->memory[chip->i + 1] = (chip->v[(chip->opcode & 0x0F00) >> 8] / 10) % 10;
	chip->memory[chip->i + 2] = chip->v[(chip->opcode & 0x0F00) >> 8] % 10;
	chip->pc += 2;
}

// 0xFX55 (LD): Store registers V0 through Vx in memory starting at location I.
// The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
void execute_opcode_0xFX55(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;

	for(uint8_t j = 0; j <= x; j++)
	{
		chip->memory[chip->i + j] = chip->v[j];
	}
	chip->pc += 2;
}

// 0xFX65 (LD): Read registers V0 through Vx from memory starting at location I.
// The interpreter reads values from memory starting at location I into registers V0 through Vx.
void execute_opcode_0xFX65(chip8_t* chip)
{
	uint8_t x = (chip->opcode & 0x0F00) >> 8;

	for(uint8_t j = 0; j <= x; j++)
	{
		chip->v[j] = chip->memory[chip->i + j];
	}
	chip->pc += 2;
}
