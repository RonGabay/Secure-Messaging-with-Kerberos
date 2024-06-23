#pragma once

#include <iostream>

using namespace std;

class Configuration
{
public:
	Configuration();
	~Configuration();

public:
	string getClientName() { return mClientName; }
	void setClientName(string str) { mClientName = str; }

	string getClientID() { return mClientID; }
	void setClientID(string str) { mClientID = str; }

	string getPasswordHash() { return mPasswordHash; }
	void setPasswordHash(string str) { mPasswordHash = str; }

	string getSymmetricKey() { return mSymmetricKey; }
	void setSymmetricKey(string str) { mSymmetricKey = str; }

	string getAuthServerIP() { return mAuthServerIP; }
	void setAuthServerIP(string str) { mAuthServerIP = str; }

	int getAuthServerPort() { return mAuthServerPort; }
	void setAuthServerPort(int num) { mAuthServerPort = num; }

	string getMsgServerIP() { return mMsgServerIP; }
	void setMsgServerIP(string str) { mMsgServerIP = str; }

	int getMsgServerPort() { return mMsgServerPort; }
	void setMsgServerPort(int num) { mMsgServerPort = num; }

	bool isRegistered() { return mRegistered; }
	void setRegistered(bool val) { mRegistered = val; }

	bool load();
	bool save();

private:
	string mClientName;
	string mClientID;
	string mPasswordHash;
	string mSymmetricKey;

	string mAuthServerIP;
	int mAuthServerPort;

	string mMsgServerIP;
	int mMsgServerPort;

	bool mRegistered;
};

