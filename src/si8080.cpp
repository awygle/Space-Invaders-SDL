#include "chip8.h"

unsigned char chip8_fontset[80] = { 
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

chip8::chip8() {}
chip8::~chip8() {}

void chip8::init() {
	pc		= 0x200;	
	opcode	= 0;		
	I		= 0;		
	sp		= 0;		

	for(int i = 0; i < 2048; i++)
		pixels[i] = 0;

	for(int i = 0; i < 16; i++)
		stack[i] = 0;

	for(int i = 0; i < 16; i++)
		key[i] = V[i] = 0;

	for(int i = 0; i < 4096; i++)
		memory[i] = 0;	

	delay_timer = 0;
	sound_timer = 0;

	drawFlag = true;
	sound = false;
}

void chip8::emulateCycle() {
	unsigned char *opcode = memory[pc];//f u
	
	switch(*opcode & 0xF0) {
		case 0x00:
			switch(opcode & 0x0F) {
				case 0x00: //NOP
					break;

				case 0x01: //LXI B,D16 [0x01, 0x34, 0x54]
					c = opcode[1];
					b = opcode[2];	
					pc += 2;	
					break;

				case 0x02: //STAX B B = 3f c = 16 A -> memory[3f16]
					memory[(b << 8) + c] = a;
					break;
					
				case 0x03: //3 bytes 1: the instruction 2: the "low data" 3: the "high data"
					c = opcode[1];
					b = opcode[2];	
					pc += 2;	
					break;
					
				case 0x04: //3 bytes 1: the instruction 2: the "low data" 3: the "high data"
					c = opcode[1];
					b = opcode[2];	
					pc += 2;	
					break;
					
				case 0x05: //3 bytes 1: the instruction 2: the "low data" 3: the "high data"
					c = opcode[1];
					b = opcode[2];	
					pc += 2;	
					break;

				default:
					cout << "Unknown opcode [0x00]: 0x" + opcode << endl; 					
					break;					
			}
			break;

		case 0x1000: // 0x1NNN: Jumps to address NNN
			pc = opcode & 0x0FFF;
			break;

		case 0x2000: // 0x2NNN: Calls subroutine at NNN.
			stack[sp] = pc;			
			sp++;					
			pc = opcode & 0x0FFF;	
			break;
		
		case 0x3000: // 0x3XNN: Skips the next instruction if VX equals NN
			if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
			break;
		
		case 0x4000: // 0x4XNN: Skips the next instruction if VX doesn't equal NN
			if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
			break;
		
		case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY.
			if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
				pc += 4;
			else
				pc += 2;
			break;
		
		case 0x6000: // 0x6XNN: Sets VX to NN.
			V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			pc += 2;
			break;
		
		case 0x7000: // 0x7XNN: Adds NN to VX.
			V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
			pc += 2;
			break;
		
		case 0x8000:
			switch(opcode & 0x000F) {
				case 0x0000: // 0x8XY0: Sets VX to the value of VY
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;

				case 0x0001: // 0x8XY1: Sets VX to "VX OR VY"
					V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;

				case 0x0002: // 0x8XY2: Sets VX to "VX AND VY"
					V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;

				case 0x0003: // 0x8XY3: Sets VX to "VX XOR VY"
					V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;

				case 0x0004: // 0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't					
					if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) 
						V[0xF] = 1;
					else 
						V[0xF] = 0;					
					V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
					pc += 2;					
					break;

				case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
					if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) 
						V[0xF] = 0; 
					else 
						V[0xF] = 1;					
					V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;

				case 0x0006: // 0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
					V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
					V[(opcode & 0x0F00) >> 8] >>= 1;
					pc += 2;
					break;

				case 0x0007: // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
					if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
						V[0xF] = 0;
					else
						V[0xF] = 1;
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];				
					pc += 2;
					break;

				case 0x000E: // 0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
					V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
					V[(opcode & 0x0F00) >> 8] <<= 1;
					pc += 2;
					break;

				default:
					cout << "Unknown opcode [0x8000]: 0x" + opcode << endl; 				
					break;
			}
			break;
		
		case 0x9000: // 0x9XY0: Skips the next instruction if VX doesn't equal VY
			if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
				pc += 4;
			else
				pc += 2;
			break;

		case 0xA000: // ANNN: Sets I to the address NNN
			I = opcode & 0x0FFF;
			pc += 2;
			break;
		
		case 0xB000: // BNNN: Jumps to the address NNN plus V0
			pc = (opcode & 0x0FFF) + V[0];
			break;
		
		case 0xC000: // CXNN: Sets VX to a random number and NN
			V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			pc += 2;
			break;
	
		case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
		{
			unsigned short x = V[(opcode & 0x0F00) >> 8];
			unsigned short y = V[(opcode & 0x00F0) >> 4];
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;

			V[0xF] = 0;
			for (int yline = 0; yline < height; yline++) {
				pixel = memory[I + yline];
				for(int xline = 0; xline < 8; xline++) {
					if((pixel & (0x80 >> xline)) != 0) {
						if(pixels[(x + xline + ((y + yline) * 64))] == 0xFFFFFFFF) {
							V[0xF] = 1;                                    
						}
						pixels[x + xline + ((y + yline) * 64)] ^= 0xFFFFFFFF;
					}
				}
			}
						
			drawFlag = true;			
			pc += 2;
		}
			break;
			
		case 0xE000:
			switch(opcode & 0x00FF)
			{
				case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
					if(key[V[(opcode & 0x0F00) >> 8]] != 0)
						pc += 4;
					else
						pc += 2;
					break;
				
				case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
					if(key[V[(opcode & 0x0F00) >> 8]] == 0)
						pc += 4;
					else
						pc += 2;
					break;

				default:
					cout << "Unknown opcode [0xE000]: 0x" + opcode << endl; 
					break;
			}
			break;
		
		case 0xF000:
			switch(opcode & 0x00FF)
			{
				case 0x0007: // FX07: Sets VX to the value of the delay timer
					V[(opcode & 0x0F00) >> 8] = delay_timer;
					pc += 2;
					break;
								
				case 0x000A: // FX0A: A key press is awaited, and then stored in VX		
				{
					bool keyPress = false;

					for(int i = 0; i < 16; i++)
					{
						if(key[i] != 0)
						{
							V[(opcode & 0x0F00) >> 8] = i;
							keyPress = true;
						}
					}

					if(!keyPress)						
						return;

					pc += 2;
				}				
					break;
				
				case 0x0015: // FX15: Sets the delay timer to VX
					delay_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;

				case 0x0018: // FX18: Sets the sound timer to VX
					sound_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;

				case 0x001E: // FX1E: Adds VX to I
					if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)	
						V[0xF] = 1;
					else
						V[0xF] = 0;
					I += V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;

				case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
					I = V[(opcode & 0x0F00) >> 8] * 0x5;
					pc += 2;
					break;

				case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2
					memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
					memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
					memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;					
					pc += 2;
					break;

				case 0x0055: // FX55: Stores V0 to VX in memory starting at address I					
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
						memory[I + i] = V[i];	
 
					I += ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
					break;

				case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address I					
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
						V[i] = memory[I + i];			

					I += ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
					break;

				default:
					cout << "Unknown opcode [0xF000]: 0x" + opcode << endl; 	
					break;
			}
			break;

		default:
			cout << "Unknown opcode: 0x" + opcode << endl;  				
			break;
	}	

	pc++;

	if(delay_timer > 0)
		delay_timer--;

	if(sound_timer > 0) {
		if(sound_timer == 1)
			sound = true;
		sound_timer--;
	} 
}

bool chip8::load(const char* filename) {
	init();
	cout << "Loading: " + (string)filename << endl;
		
	FILE* pFile = fopen(filename, "rb");
	if (pFile == NULL) {
		fputs("File error", stderr); 
		return false;
	}

	fseek(pFile, 0, SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	cout << "Filesize: " + (int)lSize << endl;
	
	char* buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL) {
		fputs("Memory error", stderr); 
		return false;
	}

	size_t result = fread(buffer, 1, lSize, pFile);
	if (result != lSize) {
		fputs("Reading error", stderr); 
		return false;
	}

	if((4096 - 512) > lSize) {
		for(int i = 0; i < lSize; i++)
			memory[i + 512] = buffer[i];
	}
	else
		cout << "Error: ROM too big for memory" << endl;
	
	fclose(pFile);
	free(buffer);

	return true;
}