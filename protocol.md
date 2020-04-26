# Protocoldescription

## Possible commands:
- 0x72 (r): read single address
- 0x77 (w): write single address
- 0x52 (R): read block (max 64 Bytes)
- 0x57 (W): write block (max 64 Bytes)

#### Possible returncodes:
- 0x6F (o): OK
- 0x6E (n): not OK
- 0x75 (u): unknown command

## Read:
#### Single address:
##### Request from PC: 
3 Bytes: [command (1-Byte)][Address (2-Bytes)]
##### Answer from Arduino
[EEPROM-Value of address (1-Byte)]
#### Whole block:
##### Request from PC:
4 Bytes: [command (1-Byte)][Baseddress (2-Bytes)][Blockcount (max. 256 => 0-255) (1-Byte)]
##### Answer from Arduino
[Array of bytes (max. 256 => 0-255) (size defined by request)]

## Write:
#### Single Address:
##### Request from PC
4 Bytes: [command (1-Byte)][Address (2-Bytes)][Value (1-Byte)]
### Answer from Arduino

#### Whole Block:
##### Request from PC:
4-66 Bytes: [command (1-Byte)][Baseddress (2-Bytes)][Data to write (max. 255 Bytes)]
##### Answer from Arduino
[Count of bytes that had been written (1-Byte)]