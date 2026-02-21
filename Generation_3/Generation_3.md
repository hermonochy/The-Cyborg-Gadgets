*SPDX-License-Identifier: CC-BY-NC-SA-4.0*

# Generation 3

Third generation watches make a significant leap forward, being the first to incorporate an MCU (microcontroller unit) board. The most common MCU used is the Arduino Pro Mini for its compact size and low price. 

## Devices

### Watches

- [Watch 3.0](./Watch3.0/Watch3.0.ino) was the first 3th generation watch to be built. It uses the inbuilt LED to flash sequences at the user.
- [Watch 3.1](./Watch3.1/Watch3.1.ino) uses a neopixel for communication, flashing various colours and sequences for various meanings. 
- [Watch 3.2](./Watch3.2/Watch3.2.ino) introducess a more advanced SSD1306 I2C OLED display, allowing for more advanced functions such as calculators or games.
- [Watch 3.4](./Watch3.4/Watch3.4.ino) is an adapted version of [Watch 3.2](./Watch3.2/Watch3.2.ino), using a 128x32px display rather than a 128x64px one.
- [Watch 3.5](./Watch3.5/Watch3.5.ino) is packed with sensors for data collection, however only has one output.
- [Watch 3.6](./Watch3.6/Watch3.6.ino) is a computation‑focused watch that consists only of a Pro Mini, an OLED display, but has 6 push‑buttons. It provides on‑device tools such as a calculator, unit converter, random number generator, score tracker, etc, while omitting any additional sensors or output peripherals.

## Installation

1. Gather the required hardware.
2. Download or clone this repository.
3. Install the Arduino IDE.
3. In the Arduino IDE, install any required libraries for your watch model.
5. Open the .ino file for your chosen gadget from the Generation_3 directory.
6. Connect your hardware according to the pin definitions at the top of the chosen file.
7. Select the appropriate port and arduino (usually a pro mini).
7. Upload the code to the arduino.