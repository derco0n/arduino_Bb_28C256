#Protocoldescription

##Read:
###PC sends:
3 Bytes: [command (1-Byte)][Address (2-Bytes)]

###Arduino answers
[EEPROM-Value of address (1-Byte)]

####Possible commands:
- 0x72 (r): read
- 0x77 (w): write

##Write:
###PC sends:
4 Bytes: [command (1-Byte)][Address (2-Bytes)][Value (1-Byte)]

###Arduino answers
[Return code (1-Byte)]

####Possible returncodes:
- 0x6F (o): OK
- 0x6E (n): not OK