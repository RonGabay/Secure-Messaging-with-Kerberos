from configuration import *
from crypto_utils import *


class Server:
    def __init__(self, sock, conf):
        self.socket = sock
        self.conf = conf
        self.request_header = RequestHeader()
        self.request_payload = bytearray()
        self.symmetric_key = bytes()

    def process_request(self, header, buf):
        self.request_header = header
        self.request_payload = bytearray(buf)
        if self.request_header.code == REQUEST_KEY_CONFIRM:
            return self.do_key_confirm()
        elif self.request_header.code == REQUEST_MESSAGE_SENDING:
            return self.do_printing()
        else:
            return self.send_error()

    def do_key_confirm(self):
        respond = RespondHeader()

        # ticket
        ticket_version = struct.unpack("<B", self.request_payload[0:1])[0]
        ticket_client_id = to_str(self.request_payload[1:17])
        ticket_server_id = to_str(self.request_payload[17:33])
        ticket_timestamp = struct.unpack("<I", self.request_payload[33:37])[0]
        ticket_iv = self.request_payload[37:53]
        self.symmetric_key = aes_decrypt(ticket_iv,
                                         self.conf.msg_server.aes_key,
                                         self.request_payload[53:85])
        t_expiration = struct.unpack("<I", self.request_payload[85:89])[0]

        # authenticator
        auth_version = struct.unpack("<B", self.request_payload[89:90])[0]
        auth_client_id = aes_decrypt(ticket_iv, self.symmetric_key, self.request_payload[90:106])
        auth_server_id = aes_decrypt(ticket_iv, self.symmetric_key, self.request_payload[106:122])
        auth_creation = struct.unpack("<Q", self.request_payload[122:130])[0]

        # check if valid request
        valid = True
        if ticket_version != auth_version \
                or ticket_client_id != auth_client_id.decode() \
                or ticket_server_id != auth_server_id.decode():
            valid = False

        if valid:
            respond.code = RESPOND_CONFIRM_KEY
            log("Key Confirm", "Symmetric key confirmation success.")
        else:
            respond.code = RESPOND_GENERAL_ERROR
            log("Key Confirm", "Symmetric key confirmation failed.")

        respond.payload_size = 0
        self.socket.send(respond.pack())

        return valid

    def do_printing(self):
        iv = self.request_payload[4:20]
        msg = aes_decrypt(iv, self.symmetric_key, self.request_payload[20:])
        log("Message", msg.decode())

        self.socket.send(RespondHeader(code=RESPOND_CONFIRM_MESSAGE, payload_size=0).pack())
        return True

    def send_error(self):
        self.socket.send(RespondHeader(code=RESPOND_GENERAL_ERROR).pack())
        return False
