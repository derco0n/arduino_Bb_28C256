/*
 * 
 * This Arduino-Based EEPROM-Writer is heavily inspired by the work of Ben Eater:
 * https://www.youtube.com/watch?v=K88pgWhEb1M
 * https://eater.net/
 * 
 * This project however is a modified approach, which targets another (greater) ROM
 * 
 * D. Marx (derco0n) 2020/03
  */

#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define MAX_ROMADDR 32767 //The ROM targeted is an AT28C256 which can hold 256kBits / 32kBytes. Our addresses therefore are 0-32767 (0x00 - 0x7fff).
#define SR_INV false
#define EEPROM_D0 5
#define EEPROM_D7 12
#define BLOCK_SIZE 32
#define WRITE_EN 13


const unsigned int MAX_PAYLOAD = 63;
const unsigned int DELAY_US = 10;

void setup() {
  digitalWrite(WRITE_EN, HIGH); //Set Write-Enable to HIGH will disable Writing to the ROM by Default and avoid accidential writes to the EEPROM
  digitalWrite(SHIFT_CLK, LOW);  
  
  //Set the Pins 2 to 4 (SHIFT_ DATA/CLK/LATCH ) as Outputs
  for (int pin=2;pin<=4;pin++){
    pinMode(pin, OUTPUT);
  }
  Serial.begin(57600);
  //delay(1000);
  //testRead();
}

void testRead(){ //Can be used to test if reading data is working
  Serial.println("");
  Serial.println("Reading EEPROM");
  Serial.println("##############");
  int bs=BLOCK_SIZE;
  int maxrom=MAX_ROMADDR;  
  
  //DEBUG
  maxrom=4096;
  char buf[200];
  sprintf(buf, "Iterating from 0 to %d", maxrom);
  Serial.println(buf);
  //DEBUG END
  
  for (int base = 0; base < (maxrom-bs); base += bs) { //Fetch a bunch of bytes per line
    
    byte data[bs];
    char buf[200]; //Serial Output buffer
    char ascii[30];
    
    //sprintf(buf, "0x%04x (%d):\t", base, base);    
    sprintf(buf, "0x%04x:\t", base);    

    int bytegapcounter=0; //needed to fill in a a gap at each byte 
    for (int offset=0;offset <= (bs-1); offset +=1){
      //delay(500);
      data[offset] = readEEPROM(base + offset);
      sprintf(buf, "%s %02x", buf, data[offset]);
      bytegapcounter++;
      if (bytegapcounter>7){
        sprintf(buf, "%s  ", buf); //Fill in a gap
        bytegapcounter=0; //reset coutner
        }
      
      }

      
     // sprintf(buf , "%s  %s", buf, ascii);
      sprintf(buf , "%s\t", buf);
    Serial.print(buf);
    for (int offset=0;offset <= (bs-1); offset +=1){
        Serial.write(data[offset] > 31 && data[offset] < 127 ? data[offset] : '.');
      }
    Serial.println();
    }
  }

void testWrite(){ //Can be used to test if writing data is working
  Serial.println("");
  Serial.println("Writing EEPROM");
  Serial.println("##############");
  int bs=BLOCK_SIZE;
  byte data = 0x00;
  char buf[200];
  for (int base = 0; base <= 31; base += 1) {// Iterate a few addresses
    if (data < 255){
      //increment data
      data++;
    }
    else {
      //Rollover
      data=0x00;
    }
    sprintf(buf, "0x%04x (%d): 0x%02x", base, base, data);
    Serial.println(buf);
    writeEEPROM(base, data);     
  }
}

byte readEEPROM(int address){
  setAddress(address, SR_INV, true);
  byte data = 0;
 for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++){
    //Set all Data-Pins as INPUT
    pinMode(pin, INPUT); 
    }
  
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin-=1) {
    //Read all bits at the given address
    data = (data << 1)+digitalRead(pin);
  }
  return data;
}


void writeEEPROMBlock(int address, byte *data, int len){ //Writes a data-block (max 63 Bytes) to the EEPROM
    digitalWrite(WRITE_EN, HIGH);
    Serial.println("Writing Block");
    
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++){
    //Set all Data-Pins as Output
    pinMode(pin, OUTPUT); 
    }

  setAddress(address, SR_INV, /*Output enable*/ false); //Set the address where we want to write.

  int maxaddr=address+len;
  int counter=0;

  while (address < maxaddr) {
    for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++){
        digitalWrite(pin, data[counter] & 1); //Writing out the right-most bit from our data-byte    
        data[counter] = data[counter] >> 1; 
        }
      delayMicroseconds(DELAY_US);
      
      //All data Pins are set. We need to trigger Write enable to write our data to the EEPROM
      digitalWrite(WRITE_EN, LOW);    
      delayMicroseconds(DELAY_US); 
      digitalWrite(WRITE_EN, HIGH);
      
      delayMicroseconds(DELAY_US); 
      address++;
      counter++;
    }
  }

void writeEEPROM(int address, byte data){ //Writes a single byte to the EEPROM
  digitalWrite(WRITE_EN, HIGH);
  setAddress(address, SR_INV, /*Output enable*/ false); //Set the address where we want to write.
  
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++){
    //Set all Data-Pins as Output
    pinMode(pin, OUTPUT); 
    }
  
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++){
    digitalWrite(pin, data & 1); //Writing out the right-most bit from our data-byte
    /*
     * ANDing with 1 will result in just the last (least significant) bit:
     * e.g.:
     * 
     * 10101010
     * AND
     * 00000001
     * Results in
     * 00000000
     * 
     * 01010101
     * AND
     * 00000001
     * Results in
     * 00000001    
    */
    //As we just got the least significant bit, we need to shift our data-byte right to get the next byte in the next round
    data = data >> 1; 
    }
    
    //All data Pins are set. We need to trigger Write enable to write our data to the EEPROM
    digitalWrite(WRITE_EN, LOW);
    //delay(50); //DEBUG
    delayMicroseconds(DELAY_US); //ROM-Datasheet says minimum Write-Pulse-Width = 100 µs but nothing about maximum. 1 Microsecond is the fastest arduino can do.
    digitalWrite(WRITE_EN, HIGH);
    delay(10); //Give the ROM some time to stabilize
    
  }

bool setAddress(int address, bool invert, bool outputEnable){ 
    /* Translates a given address to bytes which will then be shifted out
     * If invert is true, databits will be flipped before shifting out, as some shift-registers have inverted outputs.
     * 
     * The ROM targeted is an AT28C256 which can hold 256kBits / 32kBytes. Our addresses therefore are 0-32767.
     * This addressspace results in 15 Bits, leaving 1 Bit from 16 which we will use to toggle our output enable (Last output of the last shift-register).
     */
  
    if (address >= 0 && address <= MAX_ROMADDR){
      //Only shift out data if we're in the correct address-space     

      /*
       * Shifts out the first (top) 8 Bits of the address.
       * If outputEnable is true, OR it with zero resulting in an unchanged value.
       * If outputEnable is false, OR it with 0x80 so that the highest bit will be a one.
       * (0x80 => 1000 0000)
       * This way the highest bit can be used to steer the EEPROM to enable its output or not.
       * Note that the AT28C256 has an inverted OE Pin (22) which comes handy. If this Pin is low, output will be enabled, otherwise output is disabled.
      */

      if (invert){
        //Shift-registers have inverted outputs, invert outputEnable-behaviour as well
        shiftdataout((address >> 8) | (outputEnable ? 0x80 : 0x00), invert);   
      }
      else {
        shiftdataout((address >> 8) | (outputEnable ? 0x00 : 0x80), invert);   
      }
      
      
      shiftdataout(address, invert); //Shifts out the lower 8 Bits, as it ignores any higher bit.  

      /*
      //DEBUG
      char buf[200];
      sprintf(buf, "Address %04x has been set. outputEnable is %s", address, (outputEnable ? "true" : "false"));
      Serial.println(buf);
      //DEBUG END
      */
              
      return true;
  
      }
    
    return false;
  }

void shiftdataout(byte data, bool invert){ //Shifts data out to the Shift-Registers. If invert is true, databits will be flipped before shifting out
  byte realdata;
  if (invert) {    
    realdata=~data; //We have to invert the value bitwise, as the Shift-Registers seem to have inverted outputs
    }
  else {
    //Handle data as it is...
    realdata=data;
    }    

  /*
  //DEBUG
  char buf[200];
  sprintf(buf, "Shifting address-data %02x out", realdata);
  Serial.println(buf);
  //DEBUG END
  */
  
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, realdata); 
   
  clockLatch(); //Make a clock-pulse for the latch to put out the contents of the registers to their outputs
  }

void clockLatch(){ //Makes one clock-pulse to Latch
    digitalWrite(SHIFT_LATCH, LOW);
    digitalWrite(SHIFT_LATCH, HIGH);
    digitalWrite(SHIFT_LATCH, LOW);
    }

//######################

//int incomingByte = 0; // Für eingehende serielle Daten

void receive(){
  byte buf[64];  
  uint8_t pos = 0;  
  int bytesavail=Serial.readBytes(buf, 64); //Reads a maximum of 64 Bytes into buffer and return the bytecount that was read
  if (bytesavail == 3 && buf[0] == '\x72' /*Should read data*/ ){
    //[command (1-Byte)][Address (2-Bytes)]     
    
    //convert addressbytes to an integer:
    int addr = (buf[1]<<8)+buf[2];
    byte value = readEEPROM(addr); //Reads the EEPROM-value
    buf[0] = value;
    send(buf, 1); //returns the value that has just been read      
  }
  
  
  return;
  }

void send(byte *sendbuf, int len){
  Serial.write(sendbuf, len);
  return;  
}

void loop() {
  receive();

}
