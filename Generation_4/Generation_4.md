*SPDX-License-Identifier: CC-BY-NC-SA-4.0*

# Generation 4

Fourth generation devices are electronically identical to the Third Generation: they use the same hardware design and have the same firmware capabilities. The distinguishing feature of Generation 4 is the use of customised PCBs and mechanical design refinements that drastically improve aesthetics and reliability. They adopt both the Arduino Pro Minis from the previous generation and introduce some new ATTiny chips to shrink size and cost.

## Devices

- [Watch 4.0](./Watch4.0/Watch4.0.ino) is the first gadget of the 4th generation, using an almost identical program and components to [Watch 3.0](../Generation_3/Watch3.0/Watch3.0.ino).
- [Watch 4.1](./Watch4.1/Watch4.1.ino) uses some buttons and a potentiometer to give a second generation like extension the capability to blink at different speeds.
- [Watch 4.2](./Watch4.2/Watch4.2.ino) is almost identical to Watch 3.2, however uses a PCB to improve stability.
- [Watch 4.3](./Watch4.3/Watch4.3.ino) is the first fully independent gadget, featuring an RTC chip to become a fully functional watch.
- [Watch 4.5](./Watch4.5/Watch4.5.ino) is designed for expeditions, equipped many useful navigation features, most notably a GPS module.

## Installation

1. Gather the required hardware.
2. Download or clone this repository.
3. Install the Arduino IDE.
4. Download the board manager for the appropriate chip.
5. Open the .ino file for your chosen gadget from the Generation_4 directory.
6. Connect your hardware according to the pin definitions at the top of the chosen file.
7. Select the appropriate port and chip.
8. Upload the code to the arduino.