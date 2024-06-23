#pragma once

constexpr auto CLIENT_VERSION = 24;

// constants for message code
constexpr auto REQUEST_REGISTER = 1024;
constexpr auto REQUEST_LIST_SERVERS = 1026;
constexpr auto REQQUEST_SYMMETRIC_KEY = 1027;
constexpr auto REQUEST_KEY_CONFIRM = 1028;
constexpr auto REQUEST_MESSAGE = 1029;

constexpr auto RESPOND_REGISTER_SUCCESS = 1600;
constexpr auto RESPOND_REGISTER_FAIL = 1601;
constexpr auto RESPOND_LIST_SERVERS = 1602;
constexpr auto RESPOND_SYMMETRIC_KEY = 1603;
constexpr auto RESPOND_KEY_CONFIRM = 1604;
constexpr auto RESPOND_MESSAGE_CONFIRM = 1605;
constexpr auto RESPOND_GENERAL_ERROR = 1609;

constexpr auto ME_FILE = "me.info";
constexpr auto SVR_FILE = "svr.info";

constexpr auto AUTHENTICATION_SERVER = 0;
constexpr auto MESSAGING_SERVER = 1;

