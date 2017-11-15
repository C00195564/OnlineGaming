#include "TcpListener.h"

CTcpListener::CTcpListener(std::string ipAddress, int port, MessageRecievedHandler handler)
	: m_ipAddress(ipAddress), m_port(port), MessageReceived(handler)
{

}

CTcpListener::~CTcpListener()
{
	Cleanup();
}

// Send a message to the specified client
void CTcpListener::Send(int clientSocket, std::string msg)
{
	send(clientSocket, msg.c_str(), msg.size() + 1, 0);
}

void CTcpListener::SendToAll(std::string msg)
{


}

// Initialize winsock
bool CTcpListener::Init()
{
	WSAData data;
	WORD ver = MAKEWORD(2, 2);

	int wsInit = WSAStartup(ver, &data);
	// TODO: Inform caller the error that occured

	return wsInit == 0;
}

// The main processing loop
void CTcpListener::Run()
{
	char buf[MAX_BUFFER_SIZE];
	bool running = true;
	while (running)
	{
		// Create a listening socket
		listening = CreateSocket();

		if (listening == INVALID_SOCKET)
		{
			//break;
		}

		fd_set copy = master;
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];

			SOCKET client;

			if (sock == listening)
			{
				client = WaitForConnection(listening);
			}
			else
			{
				//closesocket(listening);

				int bytesReceived = 0;
				do
				{
					ZeroMemory(buf, MAX_BUFFER_SIZE);

					bytesReceived = recv(client, buf, MAX_BUFFER_SIZE, 0);
					if (bytesReceived > 0)
					{
						// Check to see if it's a command. \quit kills the server
						if (buf[0] == '\\')
						{
							// Is the command quit? 
							std::string cmd = std::string(buf, bytesReceived);
							if (cmd == "\\quit")
							{
								//running = false;
								break;
							}

							// Unknown command
							continue;
						}
						if (MessageReceived != NULL)
						{
							MessageReceived(this, client, std::string(buf, 0, bytesReceived));
							for (int i = 0; i < master.fd_count; i++)
							{
								SOCKET outSock = master.fd_array[i];
								if (outSock != listening && outSock != sock)
								{
									std::ostringstream ss;
									ss << "SOCKET #" << sock << ": " << buf << "\r\n";
									std::string strOut = ss.str();

									send(outSock, strOut.c_str(), strOut.size() + 1, 0);
								}
							}
						}
					}

				} while (bytesReceived > 0);

				closesocket(client);
				FD_CLR(sock, &master);
			}

		}
	}
	FD_CLR(listening, &master);
	closesocket(listening);

	std::string msg = "Server closed\r\n";

	while (master.fd_count > 0)
	{
		// Get the socket number
		SOCKET sock = master.fd_array[0];

		// Send the goodbye message
		send(sock, msg.c_str(), msg.size() + 1, 0);

		// Remove it from the master file list and close the socket
		FD_CLR(sock, &master);
		closesocket(sock);
	}
	Cleanup();
}

void CTcpListener::Cleanup()
{
	WSACleanup();
}

// Create a socket
SOCKET CTcpListener::CreateSocket()
{
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening != INVALID_SOCKET)
	{
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(m_port);
		inet_pton(AF_INET, m_ipAddress.c_str(), &hint.sin_addr);

		int bindOk = bind(listening, (sockaddr*)&hint, sizeof(hint));
		if (bindOk != SOCKET_ERROR)
		{
			int listenOk = listen(listening, SOMAXCONN);
			if (listenOk == SOCKET_ERROR)
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
		FD_ZERO(&master);
		FD_SET(listening, &master);

	}

	return listening;
}

// Wait for a connection
SOCKET CTcpListener::WaitForConnection(SOCKET listening)
{
	SOCKET client = accept(listening, NULL, NULL);
	FD_SET(client, &master);

	std::string welcomeMsg = "Welcome to the Den\r\n";
	send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);

	return client;
}