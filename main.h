#include <stdint.h>
#include <stdbool.h>

#define SIZE_MEMORY 4096
#define SIZE_STACK 16
#define NUM_GENERAL_PURPOSE_REGISTERS 16
#define NUM_KEYS 16
#define GFX_XAXIS 64
#define GFX_YAXIS 32
#define OFFSET_FONT 50
#define FONTSET_SIZE 80
#define SPRITE_MAX_WIDTH 8

/* Input keys
 * Keypad       Keyboard
+-+-+-+-+    +-+-+-+-+
|1|2|3|C|    |1|2|3|4|
+-+-+-+-+    +-+-+-+-+
|4|5|6|D|    |Q|W|E|R|
+-+-+-+-+ => +-+-+-+-+
|7|8|9|E|    |A|S|D|F|
+-+-+-+-+    +-+-+-+-+
|A|0|B|F|    |Z|X|C|V|
+-+-+-+-+    +-+-+-+-+
*/

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
	uint16_t opcode;
	// Index register I
	uint16_t i;
	// Program counter
	uint16_t pc;
	// Some operations allow the CPU to jump. Stack saves return address
	uint16_t stack[SIZE_STACK];
	// Stack pointer to keep track of where we are in stack
	uint16_t sp;
	// Flag which specifies screen needs to be updated
	bool draw_flag;
	// Note: Chip 8 does not have any interrupts or hardware registers

} chip8_t;

// Opcode execution prototypes:
// 0x0000: Clear screen
void execute_opcode_0x00E0(void);
// 0x00EE: Return from subroutine
void execute_opcode_0x00EE(chip8_t* chip);
// 0x1NNN (JP): Jump to subroutine @ NNN 
void execute_opcode_0x1NNN(chip8_t* chip);
// 0x2NNN: Call subroutine @ NNN 
void execute_opcode_0x2NNN(chip8_t* chip);
// 0x8XYN: Perform addition operation. Vx = Vx + Vy. VF is carry
void execute_opcode_0x8XY4(chip8_t* chip);
// 0xDXYN: Draw a sprite at coordinate (value @ Vx, value @ Vy) with a height of n pixels (rows)
void execute_opcode_0xDXYN(chip8_t* chip);
// 0xEX9E: Skip next instruction if key with the value of Vx is pressed
void execute_opcode_0xEX9E(chip8_t* chip);
// 0xEXA1: Skip next instruction if key with the value of Vx is not pressed
void execute_opcode_0xEXA1(chip8_t* chip);
// 0xFX33: Store BCD representation of Vx in memory locations I, I+1, and I+2
void execute_opcode_0xFX33(chip8_t* chip);

// Emulator operations prototypes:
void emulate_cycle(chip8_t* c);
