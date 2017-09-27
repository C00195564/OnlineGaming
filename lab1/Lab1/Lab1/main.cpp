#include <winsock2.h>
#include "Net.h"

void playerA(Net* net)
{
	int port = 28000;
	char* ip = "127.0.0.1";
	net->setupUDP(port, ip);
	std::cout << "ip: " << ip << std::endl;
	std::cout << "port: " << net->portNum << std::endl;
	std::cout << std::endl;
	std::cout << "Type in a message for the other client" << std::endl;


	char message[200];
	std::cin >> message;
	std::cout << message << std::endl;
	net->sendData(ip, 28001, message);
}

void playerB(Net* net)
{
	int port = 28001;
	char* ip = "127.0.0.1";
	net->setupUDP(port, ip);
	std::cout << "ip: " << ip << std::endl;
	std::cout << "port: " << net->portNum << std::endl;
	std::cout << std::endl;
	std::cout << "Type in a message for the other client" << std::endl;


	char message[200];
	std::cin >> message;
	net->sendData(ip, 28000, message);
}

int main()
{
	Net* myNet = new Net();

	myNet->initialise();

	std::string input;
	std::cout << "Press 'A' for player A or 'B' for player B" << std::endl;
	std::cin >> input;
	std::cout << std::endl;

	if (input == "A" || input == "a")
	{
		playerA(myNet);
	}
	else if (input == "B" || input == "b")
	{
		playerB(myNet);
	}

	system("PAUSE");
	char message[200] = "";
	myNet->receiveData(message);

	std::cout << "Message recieved: "<< message << std::endl;
	std::cout << "from ip: " << myNet->getSenderIP() << std::endl;
	std::cout << "from port: " << myNet->getSenderPort() << std::endl;

}

