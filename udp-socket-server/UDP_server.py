import socket
import matplotlib.image
import matplotlib.pyplot as plt
import matplotlib
import numpy as np
import cv2 as cv
from PIL import Image
import io
import os

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
    server_sock.bind(("", buffer_size))
    print(f"Server ({server_ip}:{server_port}) listening for connections...")
    
    currpath = os.getcwd()
    path_dirs: list[str] = [] 
    while True:
        temp_split = os.path.split(currpath)
        if temp_split[1] == '':
            break
        path_dirs.append(temp_split[1])
        currpath = temp_split[0]
    
    server_conf_path = ""
    try:
        idx = path_dirs.index("smart-scarecrow")
        server_conf_path = f"{'../' * idx}esp32-tasks/udp_client/"
    except:
        server_conf_path = input("Enter absolute path for smart-scarecrow: ")
        server_conf_path = os.path.join(server_conf_path, "esp32-tasks/udp_client/")
    finally:
        try:
            os.chdir(server_conf_path)
        except:
            print("Invalid Path!")

    with open("server_config.h", "w") as f:
        f.write("#ifndef SERVER_CONFIG_H\n#define SERVER_CONFIG_H\n\n")
        f.write(f"#define SERVER_IP \"{server_ip}\"\n")
        f.write(f"#define SERVER_PORT {server_port}\n")
        f.write(f"#define SERVER_BUFF_SIZE {int(buffer_size/2)} // in Bytes\n")
        f.write("\n#endif")
        
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
                    try:
                        byte_stream = io.BytesIO(bytearray(image))
                        img = Image.open(byte_stream)
                        img = list(img.getdata())
                        what = max([max(i) for i in img])
                        img = [tuple([max(min(int(x/what * 256), 255), 0) for x in i][::-1]) for i in img]
                        img = [img[i * x_dim:(i + 1) * x_dim] for i in range(y_dim)]
                        img_data: np.ndarray = np.array(img, dtype=np.uint8)
                        cv.imshow("camera", img_data)
                        cv.waitKey(6)
                    except Exception as e:
                        print(e)
            else:
                print("Attempting to update image")
                idk.set_data(format_pixels(x_dim, y_dim, arr_len, image))
                plt.draw()
            image = []
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
                image.extend(list(message))
            
        except Exception as e:
            print(e)

if __name__ == "__main__":
    main()