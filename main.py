import ipaddress
import socket
import threading
import sys

from server import *


def thread_function(sock, address, conf):
    log("Connection", f"New client connected: {address}.")
    connected = True
    server = Server(sock, conf)
    while connected:
        try:
            # read request header
            buf = sock.recv(REQUEST_HDR_LENGTH)
            request = RequestHeader()
            request.unpack(buf)

            # read payload
            payload_buf = bytes()
            if request.payload_size == 0:
                server.process_request(request, payload_buf)
                continue

            while len(payload_buf) < request.payload_size:
                remains = request.payload_size - len(payload_buf)
                if remains > 0:
                    buf = sock.recv(remains)
                    if not buf:
                        break
                    payload_buf += buf
                else:
                    break

            if not server.process_request(request, payload_buf):
                break
        except Exception:
            break
    sock.close()


def register(conf, sock):
    # register to authentication server
    request = RequestHeader(version=MSG_SERVER_VERSION,
                            code=REQUEST_SERVER_REGISTER,
                            payload_size=293)
    payload = struct.pack("<IH255s32s",
                          int(ipaddress.ip_address(conf.msg_server.ip)),
                          conf.msg_server.port,
                          str.encode(conf.msg_server.name),
                          conf.msg_server.aes_key)
    sock.send(request.pack())
    sock.sendall(payload)

    try:
        # read respond head
        buf = sock.recv(RESPOND_HDR_SIZE)
        respond = RespondHeader()
        respond.unpack(buf)
        if respond.payload_size == 0:
            log("Registration", "Server Registration failed")
            sys.exit(0)

        # read server id
        buf = sock.recv(respond.payload_size)
        conf.msg_server.id = to_str(bytearray(buf)[0:16])

        conf.save()
        log("Registration", "Server Registration success")
    except:
        log("Registration", "Server Registration failed")
        sys.exit(0)


def main():
    # load server info
    conf = Configuration()
    conf.read()
    if not conf.registered:
        print("Messaging Server Name:")
        conf.msg_server.name = input()
        conf.msg_server.aes_key = generate_aes_key()

        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((conf.auth_server_ip, conf.auth_server_port))
            register(conf, sock)
        except Exception:
            log("Connection", "Cannot connect to the authentication server.")
            sys.exit(0)

    ip = conf.msg_server.ip
    address = (ip, conf.msg_server.port)

    server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_sock.bind(address)
    server_sock.listen()
    log("Starting", f"Messaging server running on {ip}:{conf.msg_server.port}")
    try:
        while True:
            connected_socket, address = server_sock.accept()
            thread = threading.Thread(target=thread_function, args=(connected_socket, address, conf))
            thread.start()

    except Exception as e:
        print(e)


if __name__ == "__main__":
    main()
