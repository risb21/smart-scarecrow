import random
import socket
import time

random.seed(2024 >> 2 ^ 2024 << 2)

def main() -> None:
    """ Set server details """
    server_ip = input("Enter the server's IP address: ")
    server_port = input("Enter the server's port: ")
    server = (server_ip, server_port)

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