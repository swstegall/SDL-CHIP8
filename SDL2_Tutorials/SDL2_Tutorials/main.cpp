//Based on code from Laurence Muller at Multigesture.net.
//Written by swstegall


#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <stdlib.h>
#include <time.h>

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int X_SCALE = 6;
const int Y_SCALE = 6;

//Keyboard stuff
unsigned char key[16];
void keyboardUp(unsigned char key, int x, int y);
void keyboardDown(unsigned char key, int x, int y);

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Loads individual image as texture
SDL_Texture* loadTexture( std::string path );

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Event handler
SDL_Event e;

//Program quit flag
bool quit = false;

/*-----------------------------------------*/
//CHIP-8 Emulator:
//Emulation Constants
const int maxMem = 4096;
const int stackSize = 16;
const int regSize = 16;
const int screenSize = 64 * 32;

//Related to Loading ROM into Memory
streampos size;
char* memblock;
unsigned char memory[maxMem];
ifstream file("PONG", ios::in|ios::binary|ios::ate);

//Current Opcode
unsigned short opcode;

//CPU Registers
unsigned char V[regSize];

//Index Register
unsigned short I;

//Program Counter
unsigned short pc;

//System Memory Map
//0x000-0x1FF - CHIP-8 Interpreter (Contains font set in emu)
//0x050-0x0A0 - Used for the built-in 4x5 pixel font set (0-F)
//0x200-0xFFF - Program ROM and work RAM

//Graphics Array
unsigned char gfx[screenSize];

//Timer Registers
unsigned char delay_timer;
unsigned char sound_timer;

//Stack and Stack Pointer
unsigned short stack[stackSize];
unsigned short sp;

//CHIP-8 Fontset
unsigned char chip8_fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

//Update Screen
bool drawFlag;

//Loads File into Memory
void loadFile()
{
	if(file.is_open())
	{

		size = file.tellg();
		memblock = new char [size];
		file.seekg (0, ios::beg);
		file.read (memblock, size);
		file.close();

		cout << "The entire file content is in memory." << endl;

	}

	else cout << "Unable to open file." << endl;
}

//Copies Game from Memory into System Memory
void loadGame()
{

	for(int i = 0; i < size; ++i)
	{

		memory[i + 512] = memblock[i];

	}

}

//Clears System Memory
void clearMem()
{

	for(int i = 0; i < maxMem; ++i)
	{

		memory[i] = 0;

	}

}

//Loads Fontset
void loadFonts()
{

	for(int i = 0; i < 80; ++i)
	{

		memory[i] = chip8_fontset[i];

	}

}

//Clear Stack
void clearStack()
{

	for(int i = 0; i < stackSize; ++i)
	{

		stack[i] = 0;

	}

}

//Clear Registers 
void clearRegisters()
{

	for(int i = 0; i < regSize; ++i)
	{

		V[i] = 0;

	}

}

//Clear Screen
void clearScreen()
{

	for(int i = 0; i < screenSize; ++i)
	{

		gfx[i] = 0;

	}

}

//Initialize System
void systemInitialize()
{

	//Default Values
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;

	//Clear display
	clearScreen();
	SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
	SDL_RenderClear(gRenderer);
	SDL_RenderPresent(gRenderer);

	//Set Drawing Color to White
	SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

	//Clear stack
	clearStack();

	//Clear Registers V0-VF
	clearRegisters();

	//Clear Memory
	clearMem();

	//Load Fontset
	loadFonts();
	
	//Load Game into Memory
	loadFile();
	loadGame();

	//Reset Timers
	delay_timer = 0;
	sound_timer = 0;

	//Clear Screen Once
	drawFlag = true;

	//Seed Random
	srand(NULL);
}

//Emulate One Cycle
void emulateCycle()
{

	bool tempFlag;

	//Fetch Opcode
	opcode = memory[pc] << 8 | memory[pc + 1];

	//Decode Opcode
	switch(opcode & 0xF000)
	{

	case 0x0000:
		switch(opcode & 0x000F)
		{
		case 0x0000: //0x00E0: Clears the screen
			clearScreen();
			drawFlag = true;
			pc += 2;
			break;

		case 0x000E: //0x00EE: Returns from subroutine
			--sp;
			pc = stack[sp];
			pc += 2;
			break;

		default:
			printf("Unknown Opcode [0x0000]: 0x%X\n", opcode);
			pc += 2;
			break;
		}
		break;

	case 0x1000:
		pc = opcode & 0x0FFF;
		break;

	case 0x2000:
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;

	case 0x3000:
		if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			pc += 2;
		pc += 2;
		break;

	case 0x4000:
		if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			pc += 2;
		pc += 2;
		break;

	case 0x5000:
		if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
			pc += 2;
		pc += 2;
		break;

	case 0x6000:
		V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
		pc += 2;
		break;

	case 0x7000:
		V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
		pc += 2;
		break;

	case 0x8000:
		switch(opcode & 0x000F)
		{
		case 0x0000:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0001:
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0002:
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0003:
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0004:
			if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
				V[0xF] = 1; //carry
			else
				V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc +=2;
			break;

		case 0x0005:
			if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
				V[0xF] = 1;
			else
				V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0006:
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;

		case 0x0007:
			if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
				V[0xF] = 1;
			else
				V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x000E:
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
			break;

		default:
			printf("Unknown Opcode [0x8000]: 0x%X\n", opcode);
			pc += 2;
			break;

		}
		break;

	case 0x9000:
		if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
			pc += 2;
		pc += 2;
		break;

	case 0xA000: //ANNN: Sets I to the address NNN
		//Execute opcode
		I = opcode & 0x0FFF;
		pc += 2;
		break;

	case 0xB000:
		pc = (opcode & 0x0FFF) + V[0];
		break;

	case 0xC000:
		V[(opcode & 0x0F00) >> 8] = ((rand() % 0xFF) & (opcode & 0x00FF));
		pc += 2;
		break;

	case 0xD000:
		{
			unsigned short x = V[(opcode & 0x0F00) >> 8];
			unsigned short y = V[(opcode & 0x00F0) >> 4];
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;

			V[0xF] = 0;
			for (int yline = 0; yline < height; yline++)
			{
				pixel = memory[I + yline];
				for(int xline = 0; xline < 8; xline++)
				{
					if((pixel & (0x80 >> xline)) != 0)
					{
						if(gfx[(x + xline + ((y + yline) * 64))] == 1)
						{
							V[0xF] = 1;                                    
						}
						gfx[x + xline + ((y + yline) * 64)] ^= 1;
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

		case 0x009E:
			//EX9E: Skips the  next instruction
			//if the key stored in VX is pressed
			if(key[V[(opcode & 0x0F00) >> 8]] != 0)
				pc += 2;
			pc += 2;
			break;

		case 0x00A1:
			if(key[V[(opcode & 0x0F00) >> 8]] == 0)
				pc += 2;
			pc += 2;
			break;

		default:
			printf("Unknown Opcode [0xE000]: 0x%X\n", opcode);
			pc += 2;
			break;

		}
		break;

	case 0xF000:
		switch(opcode & 0x00FF)
		{

		case 0x0007:
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x000A:
			{
				bool keyPress = false;

					for(int i = 0; i < 16; ++i)
					{
						if(key[i] != 0)
						{
							V[(opcode & 0x0F00) >> 8] = i;
							keyPress = true;
						}
					}

				// If we didn't received a keypress, skip this cycle and try again.
				if(!keyPress)						
					return;
				pc += 2;
				break;
			}
		case 0x0015:
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0018:
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x001E:
			if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
				V[0xF] = 1;
			else
				V[0xF] = 0;
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0029:
			I = V[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
			break;

		case 0x0033:
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
			pc += 2;
			break;

		case 0x0055:
			for(int i = 0; i < ((opcode & 0x0F00) >> 8); ++i)
			{

				memory[I + i] = V[i];

			}
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		case 0x0065:
			for(int i = 0; i < ((opcode & 0x0F00) >> 8); ++i)
			{

				V[i] = memory[I + i];

			}
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		default:
			printf("Unknown Opcode [0xF000]: 0x%X\n", opcode);
			pc += 2;
			break;

		}
		break;

	}

	//Update Timers
	if(delay_timer > 0)
		--delay_timer;
	if(sound_timer > 0)
		--sound_timer;

	if(sound_timer == 1)
		cout << "BEEP!" << endl;

}

//Displays System Memory Contents
void dispMem()
{

	for(int i = 0; i < maxMem; ++i)
	{

		cout << memory[i] << " ";

	}

	cout << endl;

}
/*-----------------------------------------*/

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (SCREEN_WIDTH * X_SCALE) / 2, (SCREEN_HEIGHT * Y_SCALE) / 2, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
			SDL_RenderSetScale(gRenderer, X_SCALE, Y_SCALE);

			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Nothing to load
	return success;
}

void close()
{
	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

SDL_Texture* loadTexture( std::string path )
{
	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	return newTexture;
}

void drawCall()
{
	
	SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

	int rowNum;

	for(int y = 0; y < 32; y++)
	{

		for(int x = 0; x < 64; x++)
		{

			rowNum = y * 64;

			if(gfx[x + rowNum] != 0)
			{

				SDL_RenderDrawPoint(gRenderer, x, y);

			}

		}

	}

	drawFlag = false;

}

void drawGraphics()
{
	drawCall();
	SDL_RenderPresent(gRenderer);
	drawFlag = false;

}

void setupGraphics()
{

	//Start up SDL and create a window
	if(init())
		cout << "SDL Successfully Initialized!" << endl;

}

void setKeys()
{
	//User presses a key
	if(e.type == SDL_KEYDOWN)
		{
			//Select surfaces based on key presses
			switch(e.key.keysym.sym)
				{

			case SDLK_x:
				std::cout << "Key 'x' is pressed" << endl;
				key[0x0] = 1;
				break;

			case SDLK_1:
				std::cout << "Key '1' is pressed." << endl;
				key[0x1] = 1;
				break;

			case SDLK_2:
				std::cout << "Key '2' is pressed." << endl;
				key[0x2] = 1;
				break;

			case SDLK_3:
				std::cout << "Key '3' is pressed." << endl;
				key[0x3] = 1;
				break;

			case SDLK_q:
				std::cout << "Key 'q' is pressed." << endl;
				key[0x4] = 1;
				break;

			case SDLK_w:
				std::cout << "Key 'w' is pressed." << endl;
				key[0x5] = 1;
				break;

			case SDLK_e:
				std::cout << "Key 'e' is pressed." << endl;
				key[0x6] = 1;
				break;

			case SDLK_a:
				std::cout << "Key 'a' is pressed." << endl;
				key[0x7] = 1;
				break;

			case SDLK_s:
				std::cout << "Key 's' is pressed." << endl;
				key[0x8] = 1;
				break;

			case SDLK_d:
				std::cout << "Key 'd' is pressed." << endl;
				key[0x9] = 1;
				break;

			case SDLK_z:
				std::cout << "Key 'z' is pressed." << endl;
				key[0xA] = 1;
				break;

			case SDLK_c:
				std::cout << "Key 'c' is pressed." << endl;
				key[0xB] = 1;
				break;

			case SDLK_4:
				std::cout << "Key '4' is pressed." << endl;
				key[0xC] = 1;
				break;

			case SDLK_r:
				std::cout << "Key 'r' is pressed." << endl;
				key[0xD] = 1;
				break;

			case SDLK_f:
				std::cout << "Key 'f' is pressed." << endl;
				key[0xE] = 1;
				break;

			case SDLK_v:
				std::cout << "Key 'v' is pressed." << endl;
				key[0xF] = 1;
				break;

			//DEBUG:
			case SDLK_j:
				std::cout << "Key 'j' is pressed." << endl;
				dispMem();
				break;

				}

	}

}

void printGFX()
{

	for(int i = 0; i < screenSize; ++i)
	{

		cout << (int) gfx[i] << " ";

	}

}

void sleep(unsigned int mseconds)
{

	clock_t goal = mseconds + clock();
	while(goal > clock());

}

int main( int argc, char* args[] )
{
	//Set up render system and register input callbacks
	setupGraphics();

	//Initialize the CHIP-8 System and load the game into memory
	systemInitialize();

	while(!quit)
	{

		//Emulate Cycle
		emulateCycle();

		//DEBUG
		//printGFX();
		//cout << endl;

		//If the draw flag is set, update the screen
		if(drawFlag)
		{
		
			sleep(2);
			drawGraphics();

		}

		drawFlag = false;

		//Store key press state (Press and Release)
		setKeys();

	}

	//Free resources and close SDL
	close();

	return 0;
}