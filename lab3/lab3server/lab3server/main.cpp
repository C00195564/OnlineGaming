#include <iostream>
#include <string>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;
const int serverPort = 54010;
string serverip = "127.0.0.1";
void main()
{
	WSAData data;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &data);

	if (wsOk != 0)
	{
		cerr << "cant init winsock" << endl;
	}

	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverip.c_str(), &hint.sin_addr);

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	listen(listening, SOMAXCONN);

	fd_set master;

	FD_ZERO(&master);

	FD_SET(listening, &master);

	while (true)
	{
		fd_set copy = master;
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];
			if (sock == listening)
			{
				SOCKET client = accept(listening, nullptr, nullptr);

				FD_SET(client, &master);
				cout << "somone connected;" << endl;
				//Sends the position of the new connection in the file descriptor to the new connection
				//This is used to determine who is the chaser and who is the fleer
				string msg("Connected client id: " + to_string(master.fd_count) + "\n"); 
				if (master.fd_count == 3)	//if two people connect to server(check is 3 because the listener socket is in the set aswell
				{
					
					for (int i = 0; i < master.fd_count; i++)
					{
						SOCKET outsock = master.fd_array[i];
						if (outsock != listening && outsock != sock)
						{
							char buf[4096] = "STARTGAME"; //sends the start game to all players
							
							send(outsock, buf, sizeof(buf), 0);
						}
					}
				}
			}
			else
			{
				char buf[4096];
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					for (int i = 0; i < master.fd_count; i++)
					{
						SOCKET outsock = master.fd_array[i];
						if (outsock != listening && outsock != sock)
						{
							send(outsock, buf, bytesIn, 0);
						}
					}
				}
			}


		}

	}

	if (listening == INVALID_SOCKET)
	{
		cerr << "cant create socket" << endl;
	}



	WSACleanup();
}


	/*

	closesocket(clientSocket);

	WSACleanup();
}
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
	if (clientSocket == INVALID_SOCKET)
	{

	}

	char host[NI_MAXHOST];
	char service[NI_MAXHOST];

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXHOST);
	
	inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
	cout << "connected on port: " << ntohs(client.sin_port) << endl;

	closesocket(listening);

	char buf[4096];

	while (true)
	{
		ZeroMemory(buf, 4096);

		int bytesrecieved = recv(clientSocket, buf, 4096, 0);
		if (bytesrecieved == SOCKET_ERROR)
		{
			cerr << "error in recv" << endl;
			break;
		}
		if (bytesrecieved == 0)
		{
			cerr << "client disconnect;" << endl;
		}

		int i = send(clientSocket, buf, bytesrecieved + 1, 0);
	}*/