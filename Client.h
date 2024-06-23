#pragma once
#include <iostream>
#include <string.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "DataStructures.h"
#include "Configuration.h"

using namespace std;

void print_log(string title, string contents);

class Client
{
public:
	Client();
	~Client();

public:
	bool run();

private:
	bool connectServer(int serverNum);
	void sendToServer(SOCKET socket, RequestHeader header, char* payload);
	char* receiveFromServer(SOCKET socket, RespondHeader& header);

	bool registerClient();
	bool requestKeyFromAuthServer();
	bool confirmKeyWithMsgServer();
	bool sendMessage();
	bool listServers();
	
private:
	MessagingServer mMsgServer;
	Ticket mTicket;

	SOCKET mAuthSocket;
	SOCKET mMsgSocket;

	Configuration mConfig;
};

