import socket

server_port = 2048
buffer_size = 1024  #Bytes

def main() -> None:
    
    """ Get IP Address of server """
    ip_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ip_sock.connect(('10.0.0.0', 0))
    (server_ip, _) = ip_sock.getsockname()
    ip_sock.close()

    """ Setup UDP Server """
    server_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Listen for all IP addresses trying to connect
    server_sock.bind(("", server_port))
    print(f"Server ({server_ip}:{server_port}) listening for connections...")
    
    print("\nCopy and paste the following code into the server_config.h header file:\n")
    print("#ifndef SERVER_CONFIG_H\n#define SERVER_CONFIG_H\n")
    print(f"#define SERVER_IP \"{server_ip}\"")
    print(f"#define SERVER_PORT {server_port}")
    print("\n#endif")

    while True:
        (message, client_addr) = server_sock.recvfrom(buffer_size)
        (client_ip, client_port) = client_addr

        print(f"Message from client: {message}")
        print(f"Client IP Address: {client_ip}:{client_port}")

        server_sock.sendto(f"Hello, {client_ip}:{client_port}!".encode('utf-8'), client_addr)

if __name__ == "__main__":
    main()