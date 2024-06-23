#pragma once
#include <string>
using namespace std;

class CryptoUtils
{
public:
	CryptoUtils();

	~CryptoUtils();

public:
	static string generateNonce(const int len);

	static string convertToHexString(string data);

	static string convertFromHexString(string data);

	static string calculateSHA256(string data);

	static string encrypt_AES_CBC_256(string iv, string aesKey, string plain);

	static string decrypt_AES_CBC_256(string iv, string aesKey, string cipher);
};