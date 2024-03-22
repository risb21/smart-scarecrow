import socket
import ipaddress

buffer_size = 1024
server_port = 2048

def main() -> None:
    client_sock = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    server_ip = input("Enter IPv4 Address of server: ")
    # Validate server IP Address format
    assert type(ipaddress.ip_address(server_ip)) == ipaddress.IPv4Address

    server_addr = (server_ip, server_port)
    print(f"Connecting to server at {server_ip}:{server_port}")

    client_sock.sendto(
        f"Hello, from the client!".encode("utf-8"),
        (server_ip, server_port)
    )

    (response, server_addr2) = client_sock.recvfrom(buffer_size)
    print(f"Is message from server: {server_addr == server_addr2}")
    print(f"Response from server: {response}")

if __name__ == "__main__":
    main()