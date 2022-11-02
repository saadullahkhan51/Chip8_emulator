// to jump to address we set pc to the address value


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chip8.h" // Your cpu core implementation

void chip8::clearDisp(){
	}

unsigned char chip8_fontset[80] =
{
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
chip8::chip8()
{
	// empty
}

chip8::~chip8()
{
	// empty
}

void chip8::initialize()
{
  // Initialize registers and memory once
  pc = 0x200;
  opcode = 0;
  I = 0;
  sp = 0;
  // Clear display
  for(int i=0; i<2048; i++){
		gfx[i] = 0;
  }
  // Clear stack
  for(int i=0; i<16; i++){
	  stack[i] = 0;
  }
  // Clear registers V0-VF
  for(int i=0; i<16; i++){
	  key[i] = V[i] = 0;
  }
  // Clear memory
  for(int i=0; i<4096; i++){
	  memory[i] = 0;
  }

  // load fontset
  for(int i=0; i<80; ++i) memory[i] = chip8_fontset[i];

  // Reset timers
  delay_timer = sound_timer = 0;

  drawFlag = true;

  srand(time(NULL));
}
 
void chip8::emulateCycle()
{
  // Fetch Opcode from the memory at the location specified by pc. Fetch 2 succesive bytes and merge to get opcode
  opcode = memory[pc] << 8 | memory[pc + 1];
  printf("Current opcode: 0x%X\n", opcode);
  // Decode Opcode 
  switch(opcode & 0xF000){
	  case 0xA000:
		  {// Execute opcode; sets I to the address at NNN
		  I = opcode & 0x0FFF;
		  pc += 2;
	  break;}

	  case 0x0000:
		{
		switch(opcode & 0x000F){
			case 0x0000:
				{// Execute opcode; clears the screen
				for(int i=0; i<2048; i++){
					gfx[i] = 0x0;
				}

				drawFlag = true;
				pc += 2;
			break;}
			case 0x000E:
				{// Execute opcode; returns from subroutine
				--sp;
				pc = stack[sp];
				pc += 2;
			break;}

		 	default:
				printf("Unknown opcode: 0x%X\n", opcode);

		}
		break;}
	  case 0x1000:
		{// Execute opcode; jumps to address at NNN
		pc = opcode & 0x0FFF;
	  break;}
	  case 0x2000:
		{// Execute opcode; calls subroutine at NNN
		stack[sp] = pc; // storing current address
		++sp;
		pc = opcode & 0x0FFF; // discards the 4 MSB
	  break;}
	  case 0x3000:
		{// Execute opcode; skips next instruction if VX == NN
		if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)){
			pc += 4;
		}
		else 
			pc +=2;
	  break;}
	  case 0x4000:
		{// Execute opcode; skips next instruction if VX != NN
		if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)){
			pc += 4;
		}
		else 
			pc +=2;
	  break;}
	  case 0x5000:
		{// Execute opcode; skips next instruction if VX == VY
		if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]){
			pc += 4;
		}
		else 
			pc +=2;
	  break;}
          case 0x6000:
		{// Execute opcode; sets VX = NN
		V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
		pc += 2;
	  break;}
	  case 0x7000:
		{// Execute opcode; adds NN to VX. carry flag not changed
		V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
		pc += 2;
	  break;}
	  case 0x8000:
		{// Execute opcode

		switch(opcode & 0x000F){
			case 0x0000:
				{// Execute opcode; sets VX = VY
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
				pc += 2;
			break;}
			case 0x0001:
				{// Execute opcode; BitOp VX = VX|VY
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
				pc += 2;
			break;}
			case 0x0002:
				// Execute opcode; BitOp VX = VX&VY
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
				pc += 2;
			break;
			case 0x0003:
				{// Execute opcode; BitOp VX = VX^VY
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
				pc += 2;
			break;}
			case 0x0004:
				{// Execute opcode; adds VY to VX, VF set to 1 if carry(sum > 255) else VF=0
				if (V[(opcode & 0x00F0) >> 4] + V[(opcode & 0x0F00) >> 8] >  0xFF)
					V[0xF] = 1;
				else
					V[0xF] = 0;
				V[(opcode & 0x0F00) >> 8] += V[(opcode & 0xF00) >> 4];
				pc += 2;
			break;}
			case 0x0005:
				{// Execute opcode; VX -= VY, VF = 0 if borrow
				if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) 
				        V[0xF] = 0;
				else
					V[0xF] = 1; 	
				V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			       	pc += 2;
			break;}
			case 0x0006:
				{// Execute opcode; stores LSB of VX in VF, shifts VX to right by 1 bit
				V[0xF] = (V[(opcode & 0x0F00) >> 8] & 0x1);
				V[(opcode & 0x0F00) >> 8] >>= 1;
				pc += 2;
			break;}
			case 0x0007:
				{// Execute opcode; VX = VY-VX
				if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) 
				        V[0xF] = 0;
				else
					V[0xF] = 1; 	
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8] ;
			       	pc += 2;
			break;}
			case 0x000E:
				{// Execute opcode; stores MSB of VX in VF, shifts VX to left by 1 bit
				V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
			        V[(opcode & 0x0F00) >> 8] <<= 1;
				pc += 2;	
			break;}

		 	default:
				printf("Unknown opcode: 0x%X\n", opcode);

		}
	  break;}
	  case 0x9000:
		{// Execute opcode
		if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]){
			pc += 4;
		}
		else 
			pc +=2;
	  break;}
	  case 0xB000:
		{// Execute opcode
		pc = (opcode & 0x0FFF) + V[0x0];
	  break;}
	  case 0xC000: 
	  	{// CXNN: Sets VX to a random number and NN
		V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
		pc += 2;
	  break;}
	  case 0xD000:
		{// Execute opcode; draws sprite at (VX, VY) with width of 8, and height of N pixels
		
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;
		
		V[0xF] = 0;
		for(int row=0; row<height; row++){
			pixel = memory[I+row];
			for(int col=0; col<8; col++){
				if((pixel & (0x80 >> col)) == 1){
					if(gfx[(x + col + ((y + row) * 64))] == 1) V[0xF] = 1;
					gfx[x + col + ((y + row) * 64)] ^= 1;
				}
			}
		}

		drawFlag = true;
		pc += 2;
		
	  break;}
	  case 0xE000:
		{// Execute opcode
		switch(opcode & 0x000F){
			case 0x000E:
				{
				if(key[V[(opcode & 0x0F00) >> 8]] != 0)
					pc += 4;
				else
					pc += 2;
			break;}
			case 0x0001:
				{
				if(key[V[(opcode & 0x0F00) >> 8]] == 0)
					pc += 4;
				else
					pc += 2;
			break;}
			default:
					printf ("Unknown opcode [0xE000]: 0x%X\n", opcode);
		}		
	  break;}
	  case 0xF000:
		{// Execute opcode
		switch(opcode & 0x000F){
			case 0x0007:
				{// Execute opcode
				V[(opcode & 0x0F00) >> 8] = delay_timer;
				pc += 2;
			break;}
			case 0x000A:
				{// Execute opcode
				bool keyPress = false;
				
				for (int i=0; i<16 ; i++) {
					if (key[i] != 0){
						V[(opcode & 0x0F00) >> 8] = i;
						keyPress = true;
					}					
				}
				if (!keyPress) {
					return;
				}
				pc += 2;
			break;}
			case 0x0005:
				{// Execute opcode
				switch(opcode & 0x00F0){
					case 0x0010:
						{// Execute opcode
						delay_timer = V[(opcode & 0x0F00) >> 8];
						pc += 2;
					break;}
					case 0x0050:
						{// Execute opcode
						int X = (opcode & 0x0F00) >> 8;
						for (int i=0; i<=X; ++i) {
							memory[I+i] = V[i];
						}
						I = X + 1;
						pc += 2;
					break;}
					case 0x0060:
						{// Execute opcode
						int X = (opcode & 0x0F00) >> 8;
						for (int i=0; i<=X; ++i) {
							V[i] = memory[I+i];
						}
						I += X + 1;
						pc += 2;
					break;}	
					
					default:
						printf("Unknown opcode 0x%X\n", opcode);
				}

			break;}
			case 0x0008:
				{// Execute opcode
				sound_timer = V[(opcode & 0x0F00) >> 8];
				pc += 2;
			break;}
			case 0x000E:
				{// Execute opcode
				if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)	V[0xF] = 1;	// handles overflow
				
				else	V[0xF] = 0;
					
				I += V[(opcode & 0x0F00) >> 8];
				pc += 2;
			break;}
			case 0x0009:
				{// Execute opcode
				I = V[(opcode & 0x0F00) >> 8] * 0x5;
				pc += 2;
			break;}
			case 0x0003:
				{// Execute opcode; set_BCD(VX), *I=BCD(3), *I+1=BCD(2), *I+2=BCD(1)
				// take decimal representation of VX, place Hundreds digit at memory[I] ....
				memory[I] = V[(opcode & 0x0F00) >> 8] / 100; // hundreds digit
				memory[I+1] = (V[(opcode & 0xF00) >> 8] / 10) % 10; // tens digit
				memory[I+2] = (V[(opcode & 0xF00) >> 8] % 100) % 10; // ones digit
			break;}

		 	default:
				printf("Unknown opcode: 0x%X\n", opcode);

		}
	  break;}
	 

	  default:
		printf("Unknown opcode: 0x%X\n", opcode);
  }

 
  // Update timers
  if(delay_timer > 0) --delay_timer;

  if(sound_timer > 0){
	  if (sound_timer == 1) printf("BEEP!!\n");
	  --sound_timer;
  }
}

void chip8::debugRender() {
	for (int y=0; y<32; ++y){
		for (int x=0; x<64; ++x){
			if (gfx[(y*64) + x] == 0) printf("O");
			else printf(" ");
		}
		printf("\n");
	}
	printf("\n");
}

bool chip8::loadApplication(const char * filename)
{
	initialize();
	printf("Loading: %s\n", filename);
		
	// Open file
	FILE * pFile = fopen(filename, "rb");
	if (pFile == NULL)
	{
		fputs ("File error", stderr); 
		return false;
	}

	// Check file size
	fseek(pFile , 0 , SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int)lSize);
	
	// Allocate memory to contain the whole file
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL) 
	{
		fputs ("Memory error", stderr); 
		return false;
	}

	// Copy the file into the buffer
	size_t result = fread (buffer, 1, lSize, pFile);
	if (result != lSize) 
	{
		fputs("Reading error",stderr); 
		return false;
	}

	// Copy buffer to Chip8 memory
	if((4096-512) > lSize)
	{
		for(int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");
	
	// Close file, free buffer
	fclose(pFile);
	free(buffer);

	return true;
}
