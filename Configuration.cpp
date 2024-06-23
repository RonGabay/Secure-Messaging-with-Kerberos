#include <iostream>
#include <fstream>
#include <string>
#include <files.h>

#include "Configuration.h"
#include "Constants.h"
#include "CryptoUtils.h"

Configuration::Configuration()
{
	mClientName = "";
	mClientID = "";
	mPasswordHash = "";
	mSymmetricKey = "";

	mAuthServerIP = "";
	mAuthServerPort = 0;

	mMsgServerIP = "";
	mMsgServerPort = 0;

	mRegistered = false;
}

Configuration::~Configuration()
{

}

bool Configuration::load()
{
	mRegistered = true;
	string line;
	ifstream meFile(ME_FILE);
	if (meFile.is_open()) {
		getline(meFile, mClientName);
		getline(meFile, line);
		mClientID = CryptoUtils::convertFromHexString(line);
		getline(meFile, line);
		mPasswordHash = CryptoUtils::convertFromHexString(line);
		meFile.close();
	}
	else
		mRegistered = false;
	// read svr.info
	ifstream infile(SVR_FILE);
	if (!infile.is_open())
		return false;

	if (!getline(infile, line))
		return false;
		
	size_t pos = line.find(':');
	mAuthServerIP = line.substr(0, pos);
	mAuthServerPort = stoi(line.substr(pos + 1, -1));

	return true;
}

bool Configuration::save()
{
	ofstream mefile(ME_FILE);
	mefile << mClientName << endl;
	mefile << CryptoUtils::convertToHexString(mClientID) << endl;
	mefile << CryptoUtils::convertToHexString(mPasswordHash) << endl;
	mefile.close();
	return true;
}