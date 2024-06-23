import struct

from constants import *


class MessagingServer:
    def __init__(self, **kwargs):
        if 'ip' in kwargs:
            self.ip = kwargs['ip']
        else:
            self.ip = MSG_SERVER_IP

        if 'port' in kwargs:
            self.port = kwargs['port']
        else:
            self.port = MSG_SERVER_PORT

        if 'name' in kwargs:
            self.name = kwargs['name']
        else:
            self.name = ''

        if 'id' in kwargs:
            self.id = kwargs['id']
        else:
            self.id = ''

        if 'aes_key' in kwargs:
            self.aes_key = kwargs['aes_key']
        else:
            self.aes_key = ''


class RequestHeader:
    def __init__(self, **kwargs):
        if 'requester_id' in kwargs:
            self.requester_id = kwargs['requester_id']
        else:
            self.requester_id = ""
        if 'version' in kwargs:
            self.version = kwargs['version']
        else:
            self.version = MSG_SERVER_VERSION

        if 'code' in kwargs:
            self.code = kwargs['code']
        else:
            self.code = 0

        if 'payload_size' in kwargs:
            self.payload_size = kwargs['payload_size']
        else:
            self.payload_size = 0

    def pack(self):
        return struct.pack("<16sBHI",
                           self.requester_id.encode(),
                           self.version,
                           self.code,
                           self.payload_size)

    def unpack(self, buf):
        buf_array = bytearray(buf)
        self.requester_id = buf_array[0:16].decode("utf-8")
        (self.version, self.code, self.payload_size) \
            = struct.unpack_from("<BHI", buf_array, 16)


class RespondHeader:
    def __init__(self, **kwargs):
        if 'version' in kwargs:
            self.version = kwargs['version']
        else:
            self.version = MSG_SERVER_VERSION

        if 'code' in kwargs:
            self.code = kwargs['code']
        else:
            self.code = 0

        if 'payload_size' in kwargs:
            self.payload_size = kwargs['payload_size']
        else:
            self.payload_size = 0

    def pack(self):
        return struct.pack("<BHI", self.version, self.code, self.payload_size)

    def unpack(self, buf):
        (self.version, self.code, self.payload_size) \
            = struct.unpack("<BHI", bytearray(buf))
