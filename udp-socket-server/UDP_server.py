import socket
import matplotlib.pyplot as plt

server_port = 2048
buffer_size = 4096  #Bytes

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
    print(f"#define SERVER_BUFF_SIZE {buffer_size} \\\\ in Bytes")
    print("\n#endif")

    display_once = False
    image = []
    while True:
        (message, client_addr) = server_sock.recvfrom(buffer_size)
        (client_ip, client_port) = client_addr

        if message == b"DONE" :
            if not display_once:
                # print(len(image))
                # print(image[0:2])
                plt.imshow(image)
                plt.show()
                # pass
            server_sock.sendto(f"OK".encode('utf-8'), client_addr)
            display_once = True
            continue

        try:
            message = message.decode("ASCII")
            row = eval(message)
            image.append([(int(red*256/(1<<5)), int(green*256/(1<<6)), int(blue*256/(1<<5))) for (red, green, blue) in row])
            
            server_sock.sendto(f"OK".encode('utf-8'), client_addr)
        except Exception as e:
            # Could not eval
            print("Does it reach here")
            server_sock.sendto(f"RESEND".encode('utf-8'), client_addr)
            # raise e

        # print(f"Message from client: {message}")
        # print(f"Client IP Address: {client_ip}:{client_port}")

        server_sock.sendto(f"Hello, {client_ip}:{client_port}!".encode('utf-8'), client_addr)

if __name__ == "__main__":
    main()