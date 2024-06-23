#pragma once
#include <cstdint>

// request header
#pragma pack(push, 1)
struct RequestHeader {
	char requester_id[16];
	uint8_t version;
	uint16_t code;
	uint32_t payload_size;
};
#pragma pack(pop)

// respond header
#pragma pack(push, 1)
struct RespondHeader {
	uint8_t version;
	uint16_t code;
	uint32_t payload_size;
};
#pragma pack(pop)

// messaging server info
#pragma pack(push, 1)
struct MessagingServer {
	char id[16];
	char name[255];
	uint32_t ip;
	uint16_t port;
};
#pragma pack(pop)

// ticket data structure
#pragma pack(push, 1)
struct Ticket {
	uint8_t version;
	char client_id[16];
	char server_id[16];
	uint32_t timestamp;
	char iv[16];
	char aes_key[32];
	uint32_t expiration;
};
#pragma pack(pop)

// authentication data structure
#pragma pack(push, 1)
struct Authenticator {
	uint8_t version;
	char client_id[16];
	char server_id[16];
	uint64_t creation_time;
};
#pragma pack(pop)