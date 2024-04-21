#include <stdint.h>

#define SIZE_MEMORY 4096
#define SIZE_STACK 16
#define NUM_GENERAL_PURPOSE_REGISTERS 16
#define NUM_KEYS 16
#define GFX_XAXIS 64
#define GFX_YAXIS 32

// CPU Specifications
typedef struct chip8_t
{
	/* Memory map:
 	0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
	0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
	0x200-0xFFF - Program ROM and work RAM */

	uint8_t memory[SIZE_MEMORY];
	// Chip 8 has 15 general registers while the 16th is used for the carry flag
	uint8_t v[NUM_GENERAL_PURPOSE_REGISTERS];
	// Chip 8 aspect ratio is 64x32
	uint8_t gfx[GFX_XAXIS * GFX_YAXIS];
	// Chip 8 has 2 timers @ 60Hz. Count down to 0 when set above 0
	uint8_t delay_timer;
	// Sound timer buzzes upon reaching 0
	uint8_t sound_timer;
	// Array for hex-based keypad (0x0 - 0xF)
	uint8_t key[NUM_KEYS];
	// Current opcode (2 bytes)
	uint16_t  opcode;
	// Index register I
	uint16_t i;
	// Program counter
	uint16_t pc;
	// Some operations allow the CPU to jump. Stack saves return address
	uint16_t stack[SIZE_STACK];
	// Stack pointer to keep track of where we are in stack
	uint16_t sp;
	// Note: Chip 8 does not have any interrupts or hardware registers

} chip8_t;
