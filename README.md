# smart-scarecrow
Creating a smart scarecrow system to detect birds using computer vision and scare them away using directional speakers

## File structure

- The `./esp32-tasks` directory contains the code base for the functions carried out by the ESP32 CAM. These are further explained in the [README.md file](https://github.com/risb21/smart-scarecrow/tree/udp-test/esp32-tasks#readme) in that directory.
- The `./tcp-socket-client` directory contains the TCP client script that connects to the ESP32 CAM to transmit angle data for the speaker array
- The `./udp-socket-server` directory contains the UDP server script that receives camera data from the ESP32 CAM and displays it
- The `./lib` directory contains the ESP32 CAM libraries for the [`esp-idf`, version: `v4.4.7`](https://github.com/espressif/esp-idf/tree/1bdf0a866907ccfcb6e6c1f82470d6fbebf8f37c) (API to interact with the ESP32 CAM) and [`esp32-camera`](https://github.com/espressif/esp32-camera/tree/f0bb42917cddcfba2c32c2e2fb2875b4fea11b7a) (API with drivers for the on-board camera)

## Steps to install and use esp-idf

Navigate to the `esp-idf` directory in `lib`, then install the framework

```bash
cd lib/esp-idf
sh install.sh
```

Add the export shell script to `.bashrc` if you use bash, or `.zshrc`/`.zprofile` etc. if you use zsh

```bash
vi ~/.bashrc
```

Paste the following path to the export script in the file

```bash
export IDF_TOOLS_EXPORT_CMD="<repository location>/lib/esp-idf/export.sh"
```

<b>Upon opening a new terminal, to use the framework you must source the export script every time.</b>

```bash
source $IDF_TOOLS_EXPORT_CMD
```

## Wiring Diagram for the ESP32 CAM
![Smart-Scarecrow-Circuit](https://github.com/risb21/smart-scarecrow/assets/65121903/e1dd67ab-2349-4421-b6bc-9a9819f9bdb5)