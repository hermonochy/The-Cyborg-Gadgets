*SPDX-License-Identifier: CC-BY-NC-SA-4.0*

# Generation 5

Fifth generation watches have a similar layout to the third generation but switch from AVR microcontrollers to ESP32 based boards, primarily the ESP32-C3 Super Mini. Some later 5th generation boards requiring more GPIO pins may use an ESP32-S3 Tiny instead. This change takes advantage of the built-in Wi-Fi and Bluetooth, better processing power and an even smaller board, however drastically increases costs and power usage.

## Devices

## Installation

1. Collect the required hardware.
2. Clone or download this repository.
3. Install the [Arduino IDE](https://www.arduino.cc/en/software) and set up ESP32 board support using the Board Manager.
4. Open the `.ino` file for the watch you want to build from the Generation_5 folder.
5. Install the required libraries for your watch model.
6. Connect the hardware according to the pin mapping in the file.
8. Upload the code to the board.