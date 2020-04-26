# Arduino / Breadboard based EEPROM-Programmer and -reader

This Arduino-Based EEPROM-Writer is inspired by the work of Ben Eater:
- https://www.youtube.com/watch?v=K88pgWhEb1M
- https://eater.net/

This project however is a modified approach, which targets the, greater 28C256 EEPROM and implements a serial protocol and utility to dump data from the chip to a PC and vice versa.

## Needed stuff
- A PC which is able to run Python 3.x (only tested on Linux Mint) with a free USB Port
- Arduino Studio
- An Arduino Nano
- A high quality Breadboard (not those cheap chinese thingies)
- Some wires
- At least one Atmel 28C256 EEPROM
- 2x 8-Bit Shift registers (Type: 74HC595)
- Basic knowledge about electronics, Bytes, Python and command line usage.
