import socket
import matplotlib.image
import matplotlib.pyplot as plt
import matplotlib
import numpy as np
import cv2 as cv
from PIL import Image
import io


server_port = 2048
buffer_size = 1 << 13  # Bytes

def format_pixels(width: int, height: int, length: int, raw_data):
    if len(raw_data) != length:
        return []
    
    pixels = []

    for row in range(height):
        temp = []
        for col in range(width):
            byte_1 = raw_data[(row * width + col) * 2 + 0]
            byte_2 = raw_data[(row * width + col) * 2 + 1]
            
            red = (byte_1 >> 3)
            blue = (byte_1 & 0x07) << 3 | byte_2 >> 5
            green = byte_2 & 0x1F

            # convert to 8 bit int
            red = int(red / (1 << 5) * 256)
            blue = int(blue / (1 << 6) * 256)
            green = int(green / (1 << 5) * 256)
            temp.append((green, blue, red))
        pixels.append(temp)
    
    # print(pixels)
    return np.array(pixels, dtype=np.uint8)


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
    print(f"#define SERVER_BUFF_SIZE {buffer_size-36} // in Bytes")
    print("\n#endif")

    display_once = False
    get_dims = True
    x_dim, y_dim, arr_len = 0, 0, 0
    image = []
    idk: None | matplotlib.image.AxesImage = None
    while True:
        (message, client_addr) = server_sock.recvfrom(buffer_size)
        (client_ip, client_port) = client_addr

        if message == b"DONE" :
            if not idk:
                # print()
                if len(image) == arr_len and len(image) > 0:
                    byte_stream = io.BytesIO(bytearray(image))
                    img = Image.open(byte_stream)
                    img = list(img.getdata())
                    what = max([max(i) for i in img])
                    img = [tuple([max(min(int(x/what * 256), 255), 0) for x in i][::-1]) for i in img]
                    img = [img[i * x_dim:(i + 1) * x_dim] for i in range(y_dim)]
                    img_data: np.ndarray = np.array(img, dtype=np.uint8)
                    # img_data = img_data.reshape((x_dim, y_dim, 3))
                    # print(x_dim, y_dim, arr_len, image)
                    # formatted = format_pixels(x_dim, y_dim, arr_len, image)
                    # if len(formatted) != 0:
                        # img = Image.frombytes("")
                    # print(len(image), arr_len)
                    cv.imshow("camera", img_data)
                    cv.waitKey(100)
                    # plt.imshow(formatted)
                    # print(idk)
                    # print(type(idk))
                    # plt.show()
            else:
                print("Attempting to update image")
                idk.set_data(format_pixels(x_dim, y_dim, arr_len, image))
                plt.draw()
            image = []
            # server_sock.sendto(f"OK".encode('utf-8'), client_addr)
            display_once = True
            get_dims = True
            continue

        try:
            if get_dims:
                message = message.decode("ASCII")
                dims_n_len = message.split(" ")
                arr_len = int(dims_n_len[1])
                dims = dims_n_len[0].split("x")
                x_dim = int(dims[0])
                y_dim = int(dims[1])
                get_dims = False

            else:
                # row = eval(message)
                image.extend(list(message))
            
            # server_sock.sendto(f"OK".encode('utf-8'), client_addr)
        except Exception as e:
            # Could not eval
            # server_sock.sendto(f"RESEND".encode('utf-8'), client_addr)
            print(e)

        # server_sock.sendto(f"OK".encode('utf-8'), client_addr)

if __name__ == "__main__":
    main()