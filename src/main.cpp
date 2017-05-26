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

const int FRAMES_PER_SECOND = 60;

int main(int argc, char* args[])
{
  //Initialize game object
  Game play;

  //Timing
  time_t timer;
  int total_frames = 0;

	//Set up render system and register input callbacks
	play.setup_graphics();

	//Initialize the CHIP-8 System and load the game into memory
	play.system_initialize();

	while(!play.quit)
	{
		//Emulate Cycle
		play.emulate_cycle();

		//DEBUG
		//play.printGFX();
		//cout << endl;

		//If the draw flag is set, update the screen
		if(play.drawFlag)
    {
      play.draw_graphics();
      play.drawFlag = false;
    }

		//Store key press state (Press and Release)
		play.set_keys();

    if(total_frames % 2 == 0)
    {
      SDL_SetRenderDrawColor(play.gRenderer, 0x00, 0x00, 0x00, 0xFF);
      SDL_RenderClear(play.gRenderer);
      SDL_SetRenderDrawColor(play.gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
      SDL_RenderPresent(play.gRenderer);
    }

    //Cap the frame rate to a defined fps value
     if(time(&timer) < 1000 / FRAMES_PER_SECOND)
     {
         //Sleep the remaining frame time
         SDL_Delay((1000 / FRAMES_PER_SECOND) - time(&timer));
     }

     ++total_frames;
	}

	//Free resources and close SDL
	play.close();
	return 0;
}
