#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <fstream>

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int X_SCALE = 6;
const int Y_SCALE = 6;

//CHIP-8 Emulator:
//Emulation Constants
const int maxMem = 4096;
const int stackSize = 16;
const int regSize = 16;
const int screenSize = 64 * 32;

const string game = "games/PONG2";

class Game
{
public:
  Game();
  void keyboard_up(unsigned char key, int x, int y);
  void keyboard_down(unsigned char key, int x, int y);
  bool initialize();
  bool load_media();
  void close();
  SDL_Texture* load_texture(string path);
  void load_file();
  void load_game();
  void clear_memory();
  void load_fonts();
  void clear_stack();
  void clear_registers();
  void clear_screen();
  void system_initialize();
  void emulate_cycle();
  void display_memory();
  void draw_call();
  void draw_graphics();
  void setup_graphics();
  void set_keys();
  void print_graphics();
  void sleep(unsigned int mseconds);

  //main quit flag for the program
  bool quit = false;

  //Update Screen
  bool drawFlag;
  int frameRate;

  //window renderer
  SDL_Renderer* gRenderer = NULL;

private:
  //key press states
  unsigned char key[16];

  //window target
  SDL_Window* gWindow = NULL;

  //SDL_Events are handled with e
  SDL_Event e;

  //Related to Loading ROM into Memory
  streampos size;
  char* memblock;
  unsigned char memory[maxMem];
  ifstream file;

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
};
