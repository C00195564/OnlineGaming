/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <WS2tcpip.h>
#include "Dot.h"
#include "LTexture.h"
#include "TcpListener.h"

#pragma comment(lib, "ws2_32.lib")

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int serverPort = 54010;
string serverip  = "127.0.0.1";

SOCKET sock;

Dot dot;
Dot dot2;
Dot *LocalPlayer;
Dot *RemotePlayer;
float timer;
float starttimer;
//Starts up SDL and creates window
bool init();

//Frees media and shuts down SDL
void close();

bool gameOver;
bool startGame = false;

int clientsocket;

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

void sendMessage(string message, SOCKET sock)
{
		int sendResult = send(sock, message.c_str(), message.size() + 1, 0);
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
		sendMessage(string("GameOver" + to_string(redWin)), sock);
		gameOver = true;
	}
}

void onMessage(string);

//receive data from the server
void recieveMessage(SOCKET sock)
{
	char buf[4096];
	string userinput;
	ZeroMemory(buf, 4096);
	int bytesReceived = recv(sock, buf, 4096, 0);
	if (bytesReceived < 0)
	{
		cout << "Server> " << string(buf, 0, bytesReceived) << endl;
	}
	onMessage(string(buf));
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
		dot = Dot(true);//chaser
		dot2 = Dot(false);//fleer
		LocalPlayer = &dot;
		RemotePlayer = &dot2;

		dot2.SetPosition(600,400);

		dot.Init(gRenderer);
		dot2.Init(gRenderer);
		TimeText.loadFromRenderedText(string("Time: " + to_string((int)timer)), SDL_Color(), gRenderer);
		//networking stuff here.
		WSAData data;
		WORD ver = MAKEWORD(2, 2);
		int wsResult = WSAStartup(ver, &data);
		if (wsResult != 0)
		{
			cerr << "Can't atrts winsock, Err #" << wsResult << endl;
			
		}

		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET)
		{
			cerr << "cant create socket, err #" << WSAGetLastError() << endl;
			WSACleanup();
		}

		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(serverPort);
		inet_pton(AF_INET, serverip.c_str(), &hint.sin_addr);

		int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
		if (connResult == SOCKET_ERROR)
		{
			cerr << "cont connect to server, err #" << WSAGetLastError() << endl;
			closesocket(sock);
			WSACleanup();
		}

		//dont start until two are connected to server
		cout << "waiting for other client" << endl;
		while (startGame == false)
		{
			recieveMessage(sock);
			starttimer = SDL_GetTicks(); //timer starts when both players connect
		}
		


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
				//Handle input for the local player
				LocalPlayer->handleEvent(e);
			}
			
			//Move the  local player, remote player uses onMessage to update position
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
				if (dot.Checkcollision(dot2.GetCenterX(), dot2.GetCenterY()))	//check if players have collided
				{
					//game over red wins
					GameOver(true, timer);
					cout << "Colliding" << endl;
				}
			}
				//Update screen
			SDL_RenderPresent(gRenderer);

			// sdl ticks() startcounting thousandths of a second from the STARTUP of the program
			currentTime = SDL_GetTicks() - starttimer; // take away the start time of game from the start time of program to get the time since start of game


			timeInSeconds = currentTime / 1000;// get time in seconds


			if (timeInSeconds != lastTime)// only done once per second
			{
				//update time on screen
				timer = timeInSeconds;
				TimeText.loadFromRenderedText(string("Time: " + to_string((int)timer)), SDL_Color(), gRenderer);
				cout << "Time: " << timer << endl;
				if (timer > 20 && isHost)
				{
					//game over Blue wins
					GameOver(false, timer);
				}
				lastTime = timeInSeconds;
			}
			
			
			//send update message of local player to other client so they can update their remote player
			string message("Update " + LocalPlayer->GetPosAsString());
			sendMessage(message, sock);

			//get message from server
			recieveMessage(sock);
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}

//
void onMessage(string message)
{
	//if the message is from yourself dont do anything
	not_digit notADigit;

	//what happens when the game recieves a message

	//take the first part of the message and compare it to a string
	if (message.compare(0, 9, "Connected") == 0)	//0 and 9 define the part of the string to compare , "Connected" to if the comparison is true it returns 0

	{

		//take the string and remove everything but the integers in it 
		string::iterator end = std::remove_if(message.begin(), message.end(), notADigit);
		string all_numbers(message.begin(), end);
		stringstream ss(all_numbers);
		vector<int> vec;
		int i;

		for (; ss >> i;)
		{
			vec.push_back(i);
			cout << i << endl;
		}
		if (vec.at(0) == 2) //if you are the first person to connect you are the chaser. see server project to see where this message is sent from
		{
			LocalPlayer = &dot;
			RemotePlayer = &dot2;
		}
		else if (vec.at(0) == 3) //if you are the second to connect you are the fleer.  see server project to see where this message is sent from
		{
			LocalPlayer = &dot2;
			RemotePlayer = &dot;
		}
	}
	else if (message.compare(0, 9, "STARTGAME") == 0)
	{
		startGame = true;
	}
	else if (message.compare(0, 6, "Update") == 0)
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
	else if (message.compare(0, 8, "GameOver") == 0)
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
			GameOver(false, timer);
		}
		else
		{
			cout << "gameover num is wrong, num is: " << to_string(vec.at(0)) << endl;
		}
	}
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

//void main()
//{
//	WSAData data;
//	WORD ver = MAKEWORD(2, 2);
//	int wsResult = WSAStartup(ver, &data);
//	if (wsResult != 0)
//	{
//		cerr << "Can't atrts winsock, Err #" << wsResult << endl;
//		return;
//	}
//
//	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
//	if (sock == INVALID_SOCKET)
//	{
//		cerr << "cant create socket, err #" << WSAGetLastError() << endl;
//		WSACleanup();
//	}
//
//	sockaddr_in hint;
//	hint.sin_family = AF_INET;
//	hint.sin_port = htons(serverPort);
//	inet_pton(AF_INET, serverip.c_str(), &hint.sin_addr);
//
//	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
//	if (connResult == SOCKET_ERROR)
//	{
//		cerr << "cont connect to server, err #" << WSAGetLastError() << endl;
//		closesocket(sock);
//		WSACleanup();
//		return;
//	}
//
//	char buf[4096];
//	string userinput;
//
//	//send message
//	int sendResult = send(sock, userinput.c_str(), userinput.size() + 1, 0);
//	if (sendResult != SOCKET_ERROR)
//	{
//		ZeroMemory(buf, 4096);
//		int bytesReceived = recv(sock, buf, 4096, 0);
//		if (bytesReceived < 0)
//		{
//			cout << "Server> " << string(buf, 0, bytesReceived) << endl;
//		}
//	}
//	else
//	{
//
//	}
//}