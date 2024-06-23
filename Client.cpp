#include <iostream>
#include <fstream>
#include <string>
#include <files.h>
#include <iomanip>
#include <ctime>

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "Constants.h"
#include "Client.h"
#include "CryptoUtils.h"

#pragma comment (lib, "Ws2_32.lib")

void print_log(string title, string contents)
{
	auto t = time(nullptr);
	auto tm = *localtime(&t);
	cout << put_time(&tm, "%Y-%m-%d %H:%M:%S ") 
		<< "[" << title << "] : "
		<< contents << endl;
}

Client::Client()
{
	mAuthSocket = NULL;
	mMsgSocket = NULL;

	if (!mConfig.load()) {
		print_log("Configuration", "Cannot read server information.");
		exit(0);
	}
}

Client::~Client()
{
	if (shutdown(mAuthSocket, SD_BOTH) == SOCKET_ERROR) {
		closesocket(mAuthSocket);
		WSACleanup();
	}
	if (shutdown(mMsgSocket, SD_BOTH) == SOCKET_ERROR) {
		closesocket(mAuthSocket);
		WSACleanup();
	}
}

void Client::sendToServer(SOCKET socket, RequestHeader header, char* payload)
{
	size_t headerBufLen = sizeof(RequestHeader) + 1;
	char* headerBuf = new char[headerBufLen];
	memset(headerBuf, 0, headerBufLen);
	memcpy(headerBuf, (char*)&header, sizeof(RequestHeader));
	::send(socket, headerBuf, (int)headerBufLen, SOCK_STREAM);
	delete[] headerBuf;

	if (header.payload_size != 0 && payload != NULL) {
		size_t payloadBufLen = header.payload_size + 1;
		char* payloadBuf = new char[payloadBufLen];
		memset(payloadBuf, 0, payloadBufLen);
		memcpy(payloadBuf, payload, header.payload_size);

		::send(socket, payloadBuf, header.payload_size + 1, SOCK_STREAM);
		delete [] payloadBuf;
	}
}

char* Client::receiveFromServer(SOCKET socket, RespondHeader& header)
{
	::recv(socket, (char*)&header, sizeof(RespondHeader), 0);
	if (header.payload_size == 0)
		return NULL;
	char* payload = new char[header.payload_size + 1];
	memset(payload, 0, (size_t)(header.payload_size + 1));
	::recv(socket, payload, header.payload_size, 0);
	return payload;
}

bool Client::connectServer(int serverNum) {

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	SOCKADDR_IN addr{};
	int addrLen = sizeof(addr);
	SOCKET sock;

	string ip;
	int port;

	if (serverNum == AUTHENTICATION_SERVER) {
		ip = mConfig.getAuthServerIP();
		port = mConfig.getAuthServerPort();
	}
	else {
		ip = mConfig.getMsgServerIP();
		port = mConfig.getMsgServerPort();
	}

	inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr));
	addr.sin_port = htons((u_short)port);
	addr.sin_family = AF_INET;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect(sock, (SOCKADDR*)&addr, addrLen) == 0)
	{
		if (serverNum == AUTHENTICATION_SERVER)
			mAuthSocket = sock;
		else
			mMsgSocket = sock;
		return true;
	}
	else
	{
		closesocket(sock);
		WSACleanup();
		return false;
	}
}

bool Client::run()
{
	if (!connectServer(AUTHENTICATION_SERVER)) {
		print_log("Connection", "Cannot connect to authentication server.");
		return false;
	}
	print_log("Connection", "Connected to authentication server.");

	if (!mConfig.isRegistered()) {
		if (!registerClient()) {
			print_log("Registration", "Registration failed.");
			return false;
		}
		print_log("Registration", "Registration success.");
	}

	if (!listServers()) {
		print_log("Listing Servers", "Listing servers failed.");
		return false;
	}
	if (!requestKeyFromAuthServer()) {
		print_log("Request Key", "Requesting symmetric key failed.");
		return false;
	}
	print_log("Request Key", "Requesting symmetric key success.");

	if (!connectServer(MESSAGING_SERVER)) {
		print_log("Connection", "Cannot connect to message server.");
		return false;
	}
	print_log("Connection", "Connected to message server.");

	if (!confirmKeyWithMsgServer()) {
		print_log("Confirm Key", "Symmetric key confirmation failed.");
		return false;
	}
	print_log("Confirm Key", "Symmetric key confirmation success.");

	if (!sendMessage()) {
		print_log("Send Message", "Sending message failed.");
		return false;
	}

	print_log("Send Message", "Sending message success.");
	print_log("Terminate", "Application will be terminated.");

	return true;
}

bool Client::registerClient() {
	// input client name and password
	string line;
	cout << "Name: ";
	getline(cin, line);
	mConfig.setClientName(line);

	cout << "Password: ";
	string password;
	getline(cin, password);
	mConfig.setPasswordHash(CryptoUtils::calculateSHA256(password));

	// send request
	RequestHeader requestHeader{};
	memset(requestHeader.requester_id, ' ', 16);
	requestHeader.version = CLIENT_VERSION;
	requestHeader.code = REQUEST_REGISTER;
	requestHeader.payload_size = 510;

	char* payload = new char[requestHeader.payload_size];
	memset(payload, 0, requestHeader.payload_size);
	memcpy(payload, mConfig.getClientName().c_str(), mConfig.getClientName().length());
	memcpy(&payload[255], password.c_str(), password.length());
	sendToServer(mAuthSocket, requestHeader, payload);
	delete[] payload;
	payload = NULL;

	// receive respond
	RespondHeader respondHeader{};
	payload = receiveFromServer(mAuthSocket, respondHeader);
	if (respondHeader.code == RESPOND_REGISTER_SUCCESS && payload != NULL) {
		mConfig.setClientID(string(payload));
		mConfig.save();
		delete[] payload;
		return true;
	}
	else {
		if (payload != NULL)
			delete[] payload;
		return false;
	}
}

bool Client::listServers() {
	// send request
	RequestHeader requestHeader{};
	memcpy(requestHeader.requester_id, mConfig.getClientID().c_str(), 16);
	requestHeader.version = CLIENT_VERSION;
	requestHeader.code = REQUEST_LIST_SERVERS;
	requestHeader.payload_size = 0;
	sendToServer(mAuthSocket, requestHeader, NULL);

	// receive respond
	char* payload = NULL;
	RespondHeader respHdr{};
	payload = receiveFromServer(mAuthSocket, respHdr);
	if (respHdr.code == RESPOND_LIST_SERVERS && payload != NULL) {
		int nServerCount = respHdr.payload_size / sizeof(MessagingServer);
		print_log("Listing", "Messaging Servers:");
		for (int i = 0; i < nServerCount; i++) {
			MessagingServer* pMsgServers = (MessagingServer*)&payload[i * sizeof(MessagingServer)];

			char szIPAddress[16] = { 0 };
			snprintf(szIPAddress, sizeof(szIPAddress), "%d.%d.%d.%d",
				(pMsgServers->ip >> 24) & 0xFF, (pMsgServers->ip >> 16) & 0xFF,
				(pMsgServers->ip >> 8) & 0xFF, pMsgServers->ip & 0xFF);
			char id[17] = { 0 };
			memcpy(id, pMsgServers->id, 16);

			cout << " Server " << i + 1 << ":" << endl;
			cout << "    Name: " << pMsgServers->name << endl;
			cout << "    IP: " << szIPAddress << endl;
			cout << "    Port: " << pMsgServers->port << endl;
		}
		// select server
		int serverNum = 0;
		do {
			print_log("Selecting", "Input messaging server number: ");
			string strServerNum;
			getline(cin, strServerNum);
			try {
				serverNum = stoi(strServerNum);
			}
			catch (exception e) {
				serverNum = -1;
			}
			if (serverNum > 0 && serverNum <= nServerCount)
				break;
			print_log("Selecting", "Invalid messaging server number.");
			
		} while (true);

		memcpy(&mMsgServer, payload + sizeof(MessagingServer) * (serverNum - 1), sizeof(MessagingServer));
		char szIPAddress[16];
		snprintf(szIPAddress, sizeof(szIPAddress), "%d.%d.%d.%d",
			(mMsgServer.ip >> 24) & 0xFF, (mMsgServer.ip >> 16) & 0xFF,
			(mMsgServer.ip >> 8) & 0xFF, mMsgServer.ip & 0xFF);
		mConfig.setMsgServerIP(szIPAddress);
		mConfig.setMsgServerPort(mMsgServer.port);

		delete[] payload;
		return true;
	}
	else {
		if (payload != NULL)
			delete[] payload;
		return false;
	}
}

bool Client::requestKeyFromAuthServer() {
	// send request
	RequestHeader requestHeader{};
	memcpy(requestHeader.requester_id, mConfig.getClientID().c_str(), 16);
	requestHeader.version = CLIENT_VERSION;
	requestHeader.code = REQQUEST_SYMMETRIC_KEY;
	requestHeader.payload_size = 40;
	
	char* payload = new char[requestHeader.payload_size];
	memset(payload, 0, requestHeader.payload_size);
	memcpy(payload, mConfig.getClientID().c_str(), mConfig.getClientID().length());
	memcpy(&payload[16], mMsgServer.id, 16);
	memcpy(&payload[32], CryptoUtils::generateNonce(8).c_str(), 8);

	sendToServer(mAuthSocket, requestHeader, payload);

	delete[] payload;
	payload = NULL;

	// receive respond
	RespondHeader respondHeader;
	payload = receiveFromServer(mAuthSocket, respondHeader);
	if (respondHeader.code == RESPOND_SYMMETRIC_KEY && payload != NULL) {
		mConfig.setClientID(string(payload).substr(0, 16));
		
		char szIV[17] = { 0 };
		memcpy(szIV, &payload[16], 16);
		
		string strNonce = string(&payload[32]).substr(0, 8);
		
		char szEncrypted[33] = { 0 };
		memcpy(szEncrypted, &payload[40], 32);
		mConfig.setSymmetricKey(CryptoUtils::decrypt_AES_CBC_256(string(szIV), mConfig.getPasswordHash(), string(szEncrypted)));
		
		memcpy(&mTicket, &payload[72], sizeof(mTicket));
		
		delete[] payload;
		return true;
	}
	else {
		if (payload != NULL)
			delete[] payload;
		return false;
	}
}

bool Client::confirmKeyWithMsgServer() {
	// send request
	RequestHeader requestHeader{};
	memcpy(requestHeader.requester_id, mConfig.getClientID().c_str(), 16);
	requestHeader.version = CLIENT_VERSION;
	requestHeader.code = REQUEST_KEY_CONFIRM;
	requestHeader.payload_size = 130;

	char* payload = new char[requestHeader.payload_size];
	memset(payload, 0, requestHeader.payload_size);
	memcpy(payload, &mTicket, sizeof(mTicket));

	Authenticator authenticator{};
	authenticator.version = CLIENT_VERSION; 

	string strEncrypted = CryptoUtils::encrypt_AES_CBC_256(mTicket.iv, mConfig.getSymmetricKey(), mConfig.getClientID());
	memcpy(authenticator.client_id, strEncrypted.c_str(), 16); 

	char szServerID[17] = { 0 };
	memcpy(szServerID, mMsgServer.id, 16); 
	strEncrypted = CryptoUtils::encrypt_AES_CBC_256(mTicket.iv, mConfig.getSymmetricKey(), szServerID);
	memcpy(authenticator.server_id, strEncrypted.c_str(), 16);

	time_t t = time(0); 
	struct tm* tm;
	tm = gmtime(&t);
	authenticator.creation_time = t;

	memcpy(payload, &mTicket, sizeof(Ticket));
	memcpy(&payload[sizeof(Ticket)], &authenticator, sizeof(Authenticator));

	sendToServer(mMsgSocket, requestHeader, payload);

	delete[] payload;
	payload = NULL;

	// receive respond
	RespondHeader respondHeader;
	receiveFromServer(mMsgSocket, respondHeader);

	if (respondHeader.code == RESPOND_KEY_CONFIRM)
		return true;
	else
		return false;
}

bool Client::sendMessage() {
	// send request
	RequestHeader requestHeader{};
	memcpy(requestHeader.requester_id, mConfig.getClientID().c_str(), 16);
	requestHeader.version = CLIENT_VERSION;
	requestHeader.code = REQUEST_MESSAGE;

	print_log("Message", "Input your message to send:");
	string message;
	getline(cin, message);

	string iv = CryptoUtils::generateNonce(16);
	string strEncrypted = CryptoUtils::encrypt_AES_CBC_256(iv, mConfig.getSymmetricKey(), message);
	size_t msgLen = strEncrypted.length();
	requestHeader.payload_size = (uint32_t)(20 + msgLen);

	char* payload = new char[requestHeader.payload_size];
	memset(payload, 0, size_t(requestHeader.payload_size));
	memcpy(payload, &msgLen, 4);
	memcpy(&payload[4], iv.c_str(), 16);
	memcpy(&payload[20], strEncrypted.c_str(), msgLen);

	sendToServer(mMsgSocket, requestHeader, payload);

	delete[] payload;
	payload = NULL;

	// receive respond
	RespondHeader respond;
	receiveFromServer(mMsgSocket, respond);
	if (respond.code == RESPOND_MESSAGE_CONFIRM)
		return true;
	else
		return false;
}

