import random
import socket
import time

random.seed(2024 >> 2 ^ 2024 << 2)

def main() -> None:
    """ Set server details """
    server = ( "172.20.10.2", 4200)
    (server_ip, server_port) = server

    """ Set up client """
    client_sock = socket.socket()
    client_sock.connect(server)

    while (True):
        # client_sock.connect(server)
        angle1 = input('Enter an angle (360): ')
        angle2 = input('Enter an angle (180): ')
        
        client_sock.send(f"{angle1},{angle2}".encode('utf-8')) # deg,{int(random.random() * 180)}deg
        print(f"Angles sent: {angle1},{angle2}!")

    client_sock.close()

if __name__ == "__main__":
    main()