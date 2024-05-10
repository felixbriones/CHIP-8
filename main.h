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
// 0x2NNN (CALL): Call subroutine @ NNN 
void execute_opcode_0x2NNN(chip8_t* chip);
// 0x3XKK (SE): Skip next instruction if Vx = KK
void execute_opcode_0x3XKK(chip8_t* chip);
// 0x4XKK (SNE): Skip next instruction if Vx != KK
void execute_opcode_0x4XKK(chip8_t* chip);
// 0x5XY0 (SE): Skip next instruction if Vx = Vy
void execute_opcode_0x5XY0(chip8_t* chip);
// 0x6XKK (LD): Places the value KK into register Vx
void execute_opcode_0x6XKK(chip8_t* chip);
// 0x7XKK (ADD): Adds the value kk to the value of register Vx, then stores the result in Vx. (Vx = Vx + kk)
void execute_opcode_0x7XKK(chip8_t* chip);
// 0x8XY0 (LD): Stores the value of register Vy in register Vx (Vx = Vy)
void execute_opcode_0x8XY0(chip8_t* chip);
// 0x8XY1 (OR): OR operation with Vx and Vy. Result stored in Vx (Vx = Vx OR Vy)
void execute_opcode_0x8XY1(chip8_t* chip);
// 0x8XY2 (AND): AND operation with Vx and Vy. Result stored in Vx (Vx = Vx AND Vy)
void execute_opcode_0x8XY2(chip8_t* chip);
// 0x8XY3 (XOR): XOR operation with Vx and Vy. Result stored in Vx (Vx = Vx XOR Vy)
void execute_opcode_0x8XY3(chip8_t* chip);
// 0x8XY4 (ADD): Add Vy to Vx. If sum is greater than 255, VF is set 1 (0 otherwise). Sum stored in Vx 
void execute_opcode_0x8XY4(chip8_t* chip);
// 0x8XY5 (SUB): Then Vy is subtracted from Vx, and the results stored in Vx.
void execute_opcode_0x8XY5(chip8_t* chip);
// 0x8XY6 (SHR): If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
void execute_opcode_0x8XY6(chip8_t* chip);
// 0x8XY7 (SUBN): If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
void execute_opcode_0x8XY7(chip8_t* chip);
// 0x8XYE (SHL): If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
void execute_opcode_0x8XYE(chip8_t* chip);
// 0x9XY0 (SNE): The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
void execute_opcode_0x9XY0(chip8_t* chip);
// 0xANNN (LD): The value of register I is set to NNN
void execute_opcode_0xANNN(chip8_t* chip);
// 0xBNNN (JP): The program counter is set to nnn plus the value of V0
void execute_opcode_0xBNNN(chip8_t* chip);
// 0xCXKK (RND): Set Vx = random byte AND kk.
void execute_opcode_0xCXKK(chip8_t* chip);
// 0xDXYN (DRW): Draw a sprite at coordinate (value @ Vx, value @ Vy) with a height of n pixels (rows)
void execute_opcode_0xDXYN(chip8_t* chip);
// 0xEX9E (SKP): Skip next instruction if key with the value of Vx is pressed
void execute_opcode_0xEX9E(chip8_t* chip);
// 0xEXA1 (SKNP): Skip next instruction if key with the value of Vx is not pressed
void execute_opcode_0xEXA1(chip8_t* chip);
// 0xFX07 (LD): The value of DT is placed into Vx.
void execute_opcode_0xFX07(chip8_t* chip);
// 0xFX0A (LD): Wait for a key press, store the value of the key in Vx.
void execute_opcode_0xFX0A(chip8_t* chip);
// 0xFX15 (LD): DT is set equal to the value of Vx.
void execute_opcode_0xFX15(chip8_t* chip);
// 0xFX18 (LD): ST is set equal to the value of Vx.
void execute_opcode_0xFX18(chip8_t* chip);
// 0xFX33 (LD): Store BCD representation of Vx in memory locations I, I+1, and I+2
void execute_opcode_0xFX33(chip8_t* chip);

// Emulator operations prototypes:
void emulate_cycle(chip8_t* c);
