#!/usr/bin/python3

#Python based EEPROM Reader/Writer using the serial port

import serial
import time
import os

sport = "/dev/ttyUSB0"
baud = 115200
t_o = 0.5
bufsize = 64
MAX_ROMADDR = 0x7fff  # (32767) => The highest address-index of an 28C256-EEPROM which has 15 addresslines (0-14)


# Print iterations progress
def printProgressBar (iteration, total, prefix='', suffix='', decimals=1, length=100, fill='█', printEnd = "\r"):
    """
    Call in a loop to create terminal progress bar
    @params:
        iteration   - Required  : current iteration (Int)
        total       - Required  : total iterations (Int)
        prefix      - Optional  : prefix string (Str)
        suffix      - Optional  : suffix string (Str)
        decimals    - Optional  : positive number of decimals in percent complete (Int)
        length      - Optional  : character length of bar (Int)
        fill        - Optional  : bar fill character (Str)
        printEnd    - Optional  : end character (e.g. "\r", "\r\n") (Str)
    """
    percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
    filledLength = int(length * iteration // total)
    bar = fill * filledLength + '-' * (length - filledLength)
    print('\r%s |%s| %s%% %s (Bytes: %s / %s)' % (prefix, bar, percent, suffix, str(iteration), str(total)), end=printEnd)
    # Print New Line on Complete
    if iteration == total:
        print()

def writeAddr(address, data):
    """ Writes a value to the EEPROM at a specific address """
    print("Writing \"" + str(data) + "\" to address \"" + str(address) + "\"")
    maxretries = 10
    retry = 0
    receive = b''
    # Write a specific address => [command (1-Byte)][Address (2-Bytes)][Value (1-Byte)]
    # msg = b'\x77' + int.to_bytes(address, 2, 'big', signed=False) + data
    msg = b'\x77' + address + data
    ser.write(msg)
    while not receive == b'\x6F':  # Retry as long as the answer is not 0x6F indicating OK
        receive = ser.read(1)  # We await one byte here
        if retry == maxretries:
            break  # Max retries exceeded. Abort
        else:
            retry += 1
    return bytes(receive)

def writeBlock(baseaddress, data):
    """ Writes a block of max. 63 Bytes to the EEPROM """
    print("Writing \"" + str(data) + "\" to EEPROM starting at address \"" + str(baseaddress) + "\"")  # DEBUG
    bytestosend = len(data)
    if bytestosend <= 63:
        maxretries = 10
        retry = 0
        receive = b''
        # Write a block => [command (1-Byte)][Baseddress (2-Bytes)][Data to write (max. 63 Bytes)]
        msg = b'\x57' + baseaddress + data
        ser.write(msg)
        #while not receive == b'\x6F':  # Retry as long as the answer is not 0x6F indicating OK
        while receive == b'':  # Retry as long as the answer is not 0x6F indicating OK
            receive = ser.read(68)  # DEBUG
            #receive = ser.read(1)  # We await one byte here
            if retry == maxretries:
                break  # Max retries exceeded. Abort
            else:
                retry += 1
        return bytes(receive)
    return b'\x6E'  # Error


def readAddress(address):
    """ Reads a specific address from the EEPROM """
    maxretries = 10
    retry = 0
    ser.write(b'\x72' + address)  # Read a specific address => [command (1-Byte)][Address (2-Bytes)]
    # receive = ser.read(bufsize)
    receive = b''
    while receive == b'':  # Retry as long as no bytes have been received..
        receive = ser.read(1)  # We await one byte here
        if retry == maxretries:
            break  # Max retries exceeded. Abort
        else:
            retry += 1
    return bytes(receive)


def readBlock(baseaddress, count):
    """ Reads a whole from the EEPROM """
    if count >= 0 and count <= 255:  # Must be 1 to 256 Bytes (Index: 0-255)
        maxretries = 10
        retry = 0
        countbyte = int.to_bytes(count, 1, 'big', signed=False)  # Convert count to a an 8 Bit unsigned byte-value
        ser.write(b'\x52' + baseaddress + countbyte)  # Read a specific address => [command (1-Byte)][Address (2-Bytes)]
        receive = b''
        while receive == b'':  # Retry as long as bytes have been received..
            receive = ser.read(count+1)  # We await an answer as long as count + 1 as its index starts a 0
            if retry == maxretries:
                break  # Max retries exceeded. Abort
            else:
                retry += 1
        return bytes(receive)  # This will return the data read from the EEPROM or all 0x6E in case of an error
    return b'\x6E'  # NOK


def flashImage(file):
    """ Will flash a ROM-File to the EEPROM """
    if not os.path.exists(file):
        print("File \"" + file + "\" not found. Aborting!")
        return
    highestbyte = os.path.getsize(file)  # The the file's size in bytes
    blocksize = 63

def dumpEEPROM(highestbyte, file, deleteExisting=False):
    """ dumps the contents of the rom to a file"""
    """ everything here starts with index 0 """
    if deleteExisting:  # if an exisiting target file should be removed beford dumping
        if os.path.exists(file):  # check if the file already exists
            os.remove(file)  # if it exists, remove it
    current_baseaddress = 0
    #initial_blocksize = 127  # DEBUG
    initial_blocksize = 255
    current_blocksize = initial_blocksize
    remainingbytes = highestbyte - current_baseaddress  # Bytes that are left to read
    bytesread = 0  # Bytes that already had been read
    if highestbyte < initial_blocksize:  # blocksize must not be smaller than sizetoread
        current_blocksize = remainingbytes  # adjust blocksize to the number of bytes that should be read
    # iter = 0
    # Dumping data:
    print("Dumping Bytes 0 to " + str(highestbyte) + " from EEPROM to \"" + file + "\"")
    with open(file, 'wb') as outfile:  # Open file in binary-write-mode
        printProgressBar(bytesread, highestbyte, prefix='Progress:', suffix='Complete', length=50)
        while remainingbytes > 0:
            """
            iter += 1            
            # DEBUG
            print("")
            print("Iteration: " + str(iter))
            print("Bytes read: " + str(bytesread))
            print("Bytes Remaining: " + str(remainingbytes))
            print("Blocksize: " + str(current_blocksize))
            print("Current base address: " + str(current_baseaddress))
            # DEBUG END
            """
            ba = int.to_bytes(current_baseaddress, 2, 'big', signed=False)  # Convert the base address to a byte value
            current_datablock = readBlock(ba, current_blocksize)  # Read a block of data bytes from thee EEPROM
            bytesreceived = len(current_datablock)  # Determine how many bytes had been received via serial
            """
            # DEBUG
            print("Bytes received: " + str(bytesreceived))
            # DEBUG END
            """
            bytesread += bytesreceived  # increment bytesread by blocksize

            remainingbytes = highestbyte - bytesread  # Bytes that are left to read

            if remainingbytes < current_blocksize:  # If there is not a whole block left ...
                # ...adjust blocksize to match remainingbytes
                # as blocksize indicate the highest address (index starts at zero) and remainingbytes tells use the
                # remaining bytes (index starts a 1) we have to decrement 1
                current_blocksize = remainingbytes - 1
            # iterate base address to the next unread address
            current_baseaddress += bytesreceived
            """
            # DEBUG
            print("Next base address: " + str(current_baseaddress))
            #DEBUG END
            """
            # Update Progress Bar
            printProgressBar(bytesread, highestbyte, prefix='Progress:', suffix='Complete', length=50)
            # Check if the date in the current block is fine...
            # ... Arduino will answer with an array of all 0x6e in case of an error
            allerr = False
            for b in current_datablock:  # iterate each byte in data
                if b == 0x6e:  # if it consists af all error-codes. Abort
                    allerr = True
                else:
                    allerr = False
            if allerr:
                print("Error on block " + str(current_baseaddress) + ". Aborting!")
                break
            else:
                # Block is fine
                outfile.write(current_datablock)  # write it to the file
            time.sleep(0.1)
    outfile.close()
    print("\r\nDumping to \"" + file + "\" ended.")


# Main:
print("EEPROMTOOL")
try:
    with serial.Serial(sport, baudrate=baud, timeout=t_o) as ser:
        time.sleep(2)  # Wait a few seconds to intialize
        if ser.isOpen():
            print("Port \"" + ser.name + "\" opened.")
            while True:
                """
                value = readAddress(b'\x0f\x43')  # test
                # value = readBlock(b'\x0f\x40', 5)  # test
                print(value)
                dumpEEPROM(32767, "./28C256_dump.bin")
                time.sleep(1)
                """
                break
            # dumpEEPROM(MAX_ROMADDR, "../28C256_dump.bin", True)
            # writeAddr(b'\x00\x00', b'\x00')

            print("0x00: " + str(readAddress(b'\x00\x00')))
            print("0x01: " + str(readAddress(b'\x00\x01')))
            print("0x02: " + str(readAddress(b'\x00\x02')))
            print("0x03: " + str(readAddress(b'\x00\x03')))
            print("0x04: " + str(readAddress(b'\x00\x04')))
            print("0x05: " + str(readAddress(b'\x00\x05')))

            #writeBlock(b'\x00\x00', b'\xaa\xbb\xcc\xdd\xee\xff')
            """
            print("0x00: " + str(readAddress(b'\x00\x00')))
            print("0x01: " + str(readAddress(b'\x00\x01')))
            print("0x02: " + str(readAddress(b'\x00\x02')))
            """
            dumpEEPROM(63, "../28C256_good.bin", True)
            ser.close()
            if not ser.isOpen:
                print("Port \"" + ser.name + "\" Closed.")
                exit(0)
except serial.SerialException as e:
    print("An error with the serial interface happened. => " + str(e))
    print("Aborting!")
    exit(1)




"""
TEST- EEPROM'S contents
...
0x0f00:	 4e 6f 6e 65 2e 2e 00 00   63 01 8d 5d c9 e0 8d 1e  
0x0f10:  e5 40 13 00 00 00 00 00   00 00 00 00 00 00 00 00 
0x0f20:	 00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00   
0x0f30:  00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
0x0f40:	 32 00 ca 85 ab 82 db 83   f3 81 d5 84 f7 81 f8 83
0x0f50:  cb 81 8c 85 e3 81 18 84   ca 01 9f 86 db 01 bd 87
...
"""