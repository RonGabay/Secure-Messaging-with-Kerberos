import datetime
from base64 import b64decode, b64encode

from data_structures import *


def log(title: str, message: str):
    str_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    print(f"{str_time} [{title}] : {message}")
    return


def to_str(buf: bytearray):
    return buf.decode("utf-8").replace('\x00', '')


class Configuration:
    def __init__(self):
        self.auth_server_ip = AUTH_SERVER_IP
        self.auth_server_port = AUTH_SERVER_PORT
        self.msg_server = MessagingServer()
        self.registered = False

    def read(self):
        # read port file
        try:
            file = open(PORT_FILE_NAME, "r")
            port = file.read()
            self.msg_server.port = int(port)
            file.close()
        except Exception:
            self.msg_server.port = MSG_SERVER_PORT

        # read svr.info file
        try:
            file = open(SVR_FILE_NAME, "r")
            line = file.readline()
            tokens = line.strip().split(':')
            self.auth_server_ip = tokens[0].encode()
            self.auth_server_port = int(tokens[1].encode())
            file.close()
        except FileNotFoundError:
            log("Configuration", "svr.info file not found.")

        # read msg.info file
        try:
            file = open(MSG_FILE_NAME, "r")
            line = file.readline()
            tokens = line.strip().split(':')
            self.msg_server.ip = tokens[0]
            self.msg_server.name = tokens[2]
            self.msg_server.id = tokens[3]
            self.msg_server.aes_key = b64decode(tokens[4])
            self.registered = True
            file.close()
        except Exception:
            log("Configuration", "msg.info file not found.")

    def save(self):
        # write msg.info file
        try:
            file = open(MSG_FILE_NAME, "w")
            encoded_aes_key = to_str(b64encode(self.msg_server.aes_key))
            line = "{}:{}:{}:{}:{}\n" \
                .format(self.msg_server.ip,
                        self.msg_server.port,
                        self.msg_server.name,
                        self.msg_server.id,
                        encoded_aes_key)
            file.write(line)
            file.close()
        except Exception:
            log("Configuration", "msg.info file not saved.")
