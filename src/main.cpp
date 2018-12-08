//Based on code from Laurence Muller at Multigesture.net.
//Written by swstegall

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include "game.h"

using namespace std;

int main(int argc, char* args[])
{
  //Initialize game object
  Game play;

	//Set up render system and register input callbacks
	play.setup_graphics();

	//Initialize the CHIP-8 System and load the game into memory
	play.system_initialize();

	while(!play.quit)
	{
		//Emulate Cycle
		play.emulate_cycle();

		//If the draw flag is set, update the screen
		if(play.drawFlag)
    {
      play.draw_graphics();
      play.drawFlag = false;
      SDL_Delay(FRAME_DELAY);
    }

		//Store key press state (Press and Release)
		play.set_keys();
	}

	//Free resources and close SDL
	play.close();
	return 0;
}
