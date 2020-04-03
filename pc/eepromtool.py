#!/usr/bin/python3

#Python based EEPROM Reader/Writer using the serial port

import serial
import time

sport = "/dev/ttyUSB0"
baud = 57600
t_o = 1
bufsize = 64

print("EEPROMTOOL")

with serial.Serial(sport, baudrate=baud, timeout=t_o) as ser:
    time.sleep(2)  # Wait a few seconds to intialize
    if ser.isOpen():
        print("Port \"" + ser.name + "\" opened.")
        while True:
            ser.write(b'\x72\x0f\x43')  # Read a specific address => [command (1-Byte)][Address (2-Bytes)]
            #receive = ser.read(bufsize)
            receive = ser.read(1)  # We await one byte here
            if not receive == b'':
                print(bytes(receive))  # DEBUG
        ser.close()




"""
TEST- EEPROM'S contents
0x0f00:	 4e 6f 6e 65 2e 2e 00 00   63 01 8d 5d c9 e0 8d 1e   e5 40 13 00 00 00 00 00   00 00 00 00 00 00 00 00 
0x0f20:	 00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
0x0f40:	 32 00 ca 85 ab 82 db 83   f3 81 d5 84 f7 81 f8 83   cb 81 8c 85 e3 81 18 84   ca 01 9f 86 db 01 bd 87

"""