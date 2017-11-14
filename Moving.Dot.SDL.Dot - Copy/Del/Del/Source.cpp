/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include "Dot.h"
#include "LTexture.h"

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;


Dot dot;
Dot dot2;
Dot *LocalPlayer;
Dot *RemotePlayer;
float timer;
//Starts up SDL and creates window
bool init();

//Frees media and shuts down SDL
void close();

bool gameOver;

LTexture winText(string("font.TTF"));
LTexture TimeText(string("font.TTF"));
//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;



bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

void close()
{

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}


struct not_digit {
	bool operator()(const char c)
	{
		return c != ' ' && !isdigit(c);
	}
};

void sendMessage(string message)
{

}

void GameOver(bool redWin, int timer)
{
	if (gameOver == false)
	{
		if (redWin)
		{
			winText.loadFromRenderedText(string("Red wins. Caught blue in: " + to_string(timer) + " Seconds!"), SDL_Color(), gRenderer);
			winText.setColor(244, 66, 66);
		}
		else
		{
			winText.loadFromRenderedText(string("Blue wins. Wasn't Caught!"), SDL_Color(), gRenderer);
			winText.setColor(66, 69, 244);
		}
		sendMessage(string("GameOver" + to_string(redWin)));
		gameOver = true;
	}
}

void onMessage(string message)
{
	not_digit notADigit;
	
	//what happens when the game recieves a message
	if (message.compare(0, 6, "Update") == 0)
	{
		string::iterator end = std::remove_if(message.begin(), message.end(), notADigit);
		string all_numbers(message.begin(), end);
		stringstream ss(all_numbers);
		vector<int> vec;

		for (int i = 0; ss >> i;)
		{
			vec.push_back(i);
			cout << i << endl;
		}
		// update position if dot is the remote player;
		RemotePlayer->SetPosition(vec.at(0), vec.at(1));
		
		cout << "Update message" << endl;
	}
	else if (message.compare(0, 8, "GameOver"))
	{
		string::iterator end = std::remove_if(message.begin(), message.end(), notADigit);
		string all_numbers(message.begin(), end);
		stringstream ss(all_numbers);
		vector<int> vec;

		for (int i = 0; ss >> i;)
		{
			vec.push_back(i);
			cout << i << endl;
		}
		if (vec.at(0) == 1)
		{
			GameOver(true, timer);
		}
		else if (vec.at(0) == 0)
		{
			GameOver(true, timer);
		}
		else
		{
			cout << "gameover num is wrong, num is: " << to_string(vec.at(0)) << endl;
		}
	}
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		
		//Main loop flag
		bool quit = false;
		//Event handler
		SDL_Event e;

		TimeText.setColor(66, 69, 244);

		int currentTime = 0;
		float lastTime = 0;
		float timeInSeconds = 0;
		//number displayed to players
		
		bool caught = false;
		//The dot that will be moving around on the screen
		dot = Dot(true);
		dot2 = Dot(false);
		LocalPlayer = &dot;
		RemotePlayer = &dot2;

		dot2.SetPosition(600,400);
		onMessage(string("Update: X:050 Y:080"));

		dot.Init(gRenderer);
		dot2.Init(gRenderer);
		TimeText.loadFromRenderedText(string("Time: " + to_string((int)timer)), SDL_Color(), gRenderer);
		//networking stuff here.
		//if it cant find host become host.
		//host is always chaser
		//set up listener/connection.
		// if it finds host join game.
		//while loop to stop game entering the gameloop while other player hasn't
		//connected.
		bool isHost = true;


		//While application is running
		while (!quit)
		{
			//Handle events on queue
			while (SDL_PollEvent(&e) != 0)
			{
				//User requests quit
				if (e.type == SDL_QUIT)
				{
					quit = true;
				}
				//Handle input for the dot
				dot.handleEvent(e);
			}
			
			//Move the dot
			LocalPlayer->move(SCREEN_HEIGHT, SCREEN_WIDTH);

			//Clear screen
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderClear(gRenderer);

			//Render objects
			LocalPlayer->render(gRenderer);
			RemotePlayer->render(gRenderer);
			TimeText.render(0, 0, gRenderer);
			if (gameOver)
			{
				winText.render(150, 240, gRenderer);
			}

			if (isHost)
			{
				if (dot.Checkcollision(dot2.GetCenterX(), dot2.GetCenterY()))
				{
					//game over red wins
					GameOver(true, timer);
					cout << "Colliding" << endl;
				}
			}
				//Update screen
			SDL_RenderPresent(gRenderer);

			currentTime = SDL_GetTicks();
			timeInSeconds = currentTime / 1000;


			if (timeInSeconds != lastTime)
			{
				timer = timeInSeconds;
				TimeText.loadFromRenderedText(string("Time: " + to_string((int)timer)), SDL_Color(), gRenderer);
				cout << "Time: " << timer << endl;
				if (timer > 20 && isHost)
				{
					//game over Blue wins
					GameOver(false, timer);
				}
			}
			
			lastTime = timeInSeconds;

			string message("Update " + LocalPlayer->GetPosAsString());
			sendMessage(message);
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}






/*
convert string to int vector, for getting x and y from other player
#include <sstream>
#include <string>
#include <iostream>

int main() {
std::string s = "123 123 123 123 123";
std::istringstream iss(s);

int arr[5];
for (auto& i : arr) {
iss >> i;
}
}
*/