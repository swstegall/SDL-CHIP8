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

	int frameRate = 0;

	while(!play.quit)
	{
		//Emulate Cycle
		play.emulate_cycle();

		//DEBUG
		//printGFX();
		//cout << endl;

		//If the draw flag is set, update the screen
		if(play.drawFlag)
		{
			play.sleep(10);
			play.draw_graphics();
		}

		play.drawFlag = false;

		//Store key press state (Press and Release)
		play.set_keys();

		//Tbh I have no idea why this works... :^)
		if(play.frameRate % 60 == 0)
		{
			SDL_SetRenderDrawColor(play.gRenderer, 0x00, 0x00, 0x00, 0xFF);
			SDL_RenderClear(play.gRenderer);
			SDL_SetRenderDrawColor(play.gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderPresent(play.gRenderer);
			play.frameRate = 0;
		}

		play.frameRate++;
	}

	//Free resources and close SDL
	play.close();
	return 0;
}
