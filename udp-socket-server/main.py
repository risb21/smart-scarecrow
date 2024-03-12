# import random
import socket

def main():
    server_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_sock.bind(("", 3333))

    while True:
        # rngval = random.randint(0, 10)
        message, client_ip = server_sock.recvfrom(1024)
        server_sock.sendto(message, client_ip)


if __name__ == "__main__":
    main()