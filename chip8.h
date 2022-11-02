class chip8 {
	public:
		chip8();
		~chip8();

		bool drawFlag;

		void emulateCycle();
		void debugRender();
		bool loadApplication(const char *filename);
		void clearDisp();
		unsigned char key[16];		//HEX keypad
		unsigned char gfx[64 * 32];	//2048 pixel graphics. array that holds pixel state
									//Drawing in XOR mode to allow collision detection
	private:
		unsigned short opcode;		//chip8 has 35 2-byte opcodes
		unsigned char memory[4096];	//has 4k memory
		unsigned char V[16];		//has 15 general purpose registers and 1 register for carry flag
		unsigned short I, pc;		//Index register and program counter. 0x000 to 0xFFF value range
		unsigned char delay_timer;	//count at 60Hz. will count to 0 if set to >0
		unsigned char sound_timer;

		unsigned short stack[16];	//store return address when subroutines are called
		unsigned short sp;
		
		void initialize();
};
		
