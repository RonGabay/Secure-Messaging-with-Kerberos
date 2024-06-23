#include <osrng.h>
#include <base64.h>
#include <files.h>
#include <fstream>

#include "cryptlib.h"
#include "filters.h"
#include "files.h"
#include "modes.h"
#include "hex.h"
#include "aes.h"

#include <vector>
#include <random>
#include <climits>
#include <algorithm>
#include <functional>

#include "cryptoUtils.h"

using namespace CryptoPP;

CryptoUtils::CryptoUtils()
{

}

CryptoUtils::~CryptoUtils()
{

}

string CryptoUtils::generateNonce(const int len)
{
	string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

	random_device rd;
	mt19937 generator(rd());

	shuffle(str.begin(), str.end(), generator);

	return str.substr(0, len);
}

string CryptoUtils::convertToHexString(string data)
{
	string hexString = "";
	StringSource ss(data, true,
		new HexEncoder(new StringSink(hexString)
		));
	return hexString;
}

string CryptoUtils::convertFromHexString(string data)
{
	string converted = "";
	StringSource ss(data, true,
		new HexDecoder(new StringSink(converted)
		));
	return converted;
}

string CryptoUtils::calculateSHA256(string data)
{
	byte const* pData = (byte*)data.data();
	size_t nDataLen = data.length();
	byte digest[SHA256::DIGESTSIZE];

	SHA256().CalculateDigest(digest, pData, nDataLen);

	return string((char*)digest, SHA256::DIGESTSIZE);
}

string CryptoUtils::encrypt_AES_CBC_256(string iv, string aesKey, string plain)
{
	byte szKey[AES::MAX_KEYLENGTH];
	byte szIV[AES::BLOCKSIZE];

	memcpy(szKey, aesKey.c_str(), aesKey.length());
	memcpy(szIV, iv.c_str(), AES::BLOCKSIZE);

	AES::Encryption encryption(szKey, AES::MAX_KEYLENGTH);
	CBC_Mode_ExternalCipher::Encryption cbcEncryption(encryption, szIV);

	size_t bufLen = 0;
	if ((plain.length() % AES::BLOCKSIZE) == 0)
		bufLen = plain.length();
	else
		bufLen = (plain.length() / AES::BLOCKSIZE + 1) * AES::BLOCKSIZE;

	char* buf = new char[bufLen];
	memset(buf, 0, bufLen);
	memcpy(buf, plain.c_str(), plain.length());

	string result;
	StreamTransformationFilter encryptor(cbcEncryption, 
		new StringSink(result), 
		BlockPaddingSchemeDef::BlockPaddingScheme::NO_PADDING);
	encryptor.Put(reinterpret_cast<const unsigned char*>(buf), bufLen);
	encryptor.MessageEnd();

	return result;
}

string CryptoUtils::decrypt_AES_CBC_256(string iv, string aesKey, string cipher)
{
	// get aes key
	byte szKey[AES::MAX_KEYLENGTH];
	byte szIV[AES::BLOCKSIZE];

	memcpy(szKey, aesKey.c_str(), aesKey.length());
	memcpy(szIV, iv.c_str(), AES::BLOCKSIZE);

	AES::Decryption aesDecryption(szKey, AES::MAX_KEYLENGTH);
	CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, szIV);

	string result;
	StreamTransformationFilter decryptor(cbcDecryption, 
		new StringSink(result), 
		BlockPaddingSchemeDef::BlockPaddingScheme::NO_PADDING);
	decryptor.Put(reinterpret_cast<const unsigned char*>(cipher.c_str()), cipher.size());
	decryptor.MessageEnd();

	return result;
}

