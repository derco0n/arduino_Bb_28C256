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
const unsigned int DELAY_US = 100;
bool writemode=false; // indicates the state of the I/O_pins

void setup() {
  digitalWrite(WRITE_EN, HIGH); //Set Write-Enable to HIGH will disable Writing to the ROM by Default and avoid accidential writes to the EEPROM
  digitalWrite(SHIFT_CLK, LOW);  
  
  //Set the Pins 2 to 4 (SHIFT_ DATA/CLK/LATCH ) as Outputs
    for (int pin=2;pin<=4;pin++){
    pinMode(pin, OUTPUT);
  }  
  Serial.begin(115200);
  digitalWrite(WRITE_EN, HIGH); //Set Write-Enable to HIGH will disable Writing to the ROM by Default and avoid accidential writes to the EEPROM

  /*
  delay(1000);
  byte test[6]={0x60, 0x61, 0x66, 0x63, 0x64, 0x65};
  writeEEPROMBlock(0x04, test, 6);
  delay(1000);
  testRead();
  */

}

void testRead(){ //Can be used to test if reading data is working
  Serial.println("");
  Serial.println("Reading EEPROM");
  Serial.println("##############");
  int bs=BLOCK_SIZE;
  int maxrom=MAX_ROMADDR;  
  
  //DEBUG
  maxrom=256;
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

void readEEPROMbl(byte *buf /*pointer to an initialized buffer*/, int base /*baseaddress*/, int len /*bytecount to be read*/){ //Reads a whole block from the EEPROM and puts its data to buf
  int maxbyte=255;  //The highest byte-index we can use
  int i=0; //itaration counter
  if (len >= 3 && len <=maxbyte){ // Must be 4 to 256 Bytes (Index: 3-255)
      //Initialize our buffer with 0x00
      for (i=0; i<len; i++){
        //Fill the buffer with zeroes
        buf[i] = '\x00';        
        }

      //Read data from EEPROM      
      for (i=0; i<=len; i++){ //Iterate through all bytes that should be read...        
        //buf[i] = len; //DEBUG        
        buf[i] = readEEPROM(base + i);
        }       

      return; //everything fine up to here
      }

    //Some error occured and the requesting computer should be informed about this    
    for (i=0; i<=maxbyte; i++){
      //Fill the buffer with status code NOK
      buf[i] = '\x6e';
    }
    return; //something went wrong    
  }

byte readEEPROM(int address){  
  // This reduce the time to switch the data-pins between INPUT and OUTPUT
  if (writemode) { //We are in write mode at the moment and should set the data-pins as input      
     for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++){
        //Set all Data-Pins as INPUT
        pinMode(pin, INPUT); 
        }
     writemode=false;   
    }

  setAddress(address, SR_INV, true);
  /*
  if (!setAddress(address, SR_INV, true)) {//Set-address
    //If address could not be set...
    return 0x75; // ...Return error
  }
  */
  byte data = 0;
    
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin-=1) {
    //Read all bits at the given address
    data = (data << 1)+digitalRead(pin);
  }
  return data;
}


int writeEEPROMBlock(int address, byte *data, int len){    
   /* 
    *  This will write a block of bytes to the chip utilizing existing functions
    */    
    
    digitalWrite(WRITE_EN, HIGH); //Set WRITE-EN to HIGH which disables writing    
    int maxaddr=address+len;    
    int cnt=0;
    if (len < 255) { //If there a max. 254 Bytes to be written
      for (cnt=0; cnt <= len; cnt++){
          if (!writeEEPROM(address, data[cnt])){
            break; //if the Byte could not be written, abort the loop as this indicates the maximum ROM-Size has beeen exceeded
          }
          else {
            address++; //iterate by 1 as another byte is written
          }
      }
    }        
    delay(10);
    return cnt; //Return the amount of bytes that had been written to the chip
  }

bool writeEEPROM(int address, byte data){ //Writes a single byte to the EEPROM
  digitalWrite(WRITE_EN, HIGH);

  // This reduces the time to switch the data-pins between INPUT and OUTPUT
   if (!writemode) { //We are in read mode at the moment and should set the data-pins as output      
     for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++){
        //Set all Data-Pins as Output
        pinMode(pin, OUTPUT); 
        }
     writemode=true;   
    }
    
  if (!setAddress(address, SR_INV, /*Output enable*/ false)){ //Set the address to where we want to write.
    return false; //Abort if address could not be set (address exceeds ROM-Size)
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
    //As we just got the least significant bit, we need to shift our data-byte right to get the next bit of this byte in the next round
    data = data >> 1; 
    }
    
    //All data Pins are set. We need to trigger Write enable to write our data to the EEPROM
    digitalWrite(WRITE_EN, LOW);    
    delayMicroseconds(DELAY_US); //ROM-Datasheet says minimum Write-Pulse-Width = 100 µs. 1 Microsecond is the fastest arduino can do.
    digitalWrite(WRITE_EN, HIGH);
    delay(15); //Give the ROM some time to stabilize
    
    return true; //Byte had been written. OK
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
  byte buf[260];  
  //uint8_t pos = 0;  
  int bytesavail=Serial.readBytes(buf, 260); //Reads a maximum of 260 Bytes into buffer and return the bytecount that was read
  // Command-cases:
  if (bytesavail == 3 && buf[0] == '\x72' /*r*/ ){
     /*Should read a single address
     *[command (1-Byte)][Address (2-Bytes)]
     */    
    //convert addressbytes to an integer:
    int addr = (buf[1]<<8)+buf[2];
    byte value = readEEPROM(addr); //Reads the EEPROM-value
    buf[0] = value; //Put the value that hast been read to the serial buffer
    send(buf, 1); //send the value that has just been read      
    return;
  }
  else if (bytesavail == 4 && buf[0] == '\x52' /*R*/ ) {
    /*  Should read a block
     *[command (1-Byte)][Baseaddress (2-Bytes)][Blockcount (0 - 255) (1-Byte)]
     */    
    //convert addressbytes to an integer:    
    int baseaddr = (buf[1]<<8)+buf[2];
    int index = buf[3]; // Get the highest index-value
    int count = index + 1; //index gives the highest index. e.g 255, which means we ne a byte-array that can hold 256 Bytes. therefore increment by one
    byte databuf[count]; // declare a byte-array with the size from the serial-line. as the byte's index starts with zero we need 1 more field
    readEEPROMbl(databuf, baseaddr, index); //Reads the EEPROM-value and put the value into databuf
    send(databuf, count); //send the value that has just been read          
    return;
  }
  else if (bytesavail == 4 && buf[0] == '\x77' /*w*/) {
     /*  Should write a single address
     *[command (1-Byte)][Address (2-Bytes)][Value (1-Byte)]
     */  
     //convert addressbytes to an integer:
     int addr = (buf[1]<<8)+buf[2]; 
     writeEEPROM(addr, buf[3]);  //Write data back
     buf[0] = '\x6F'; //Send OK back
     send(buf, 1); //send the value...
     return;
    }  
  else if (buf[0] == '\x57' && bytesavail >= 4 && bytesavail <= 258) // 'W'
    {
    /*  Should write a block
     *[command (1-Byte)][Baseddress (2-Bytes)][Data to write (max. 255 Bytes)]
     */
     //convert addressbytes to an integer:    
    int baseaddr = (buf[1]<<8)+buf[2];
    int bytestowrite = bytesavail - 3; //All bytes available - command and baseaddress

    int count = bytestowrite + 1;
    byte databuf[count];
    
    int i=0;
    int j=3;
    for (i=0; i<=bytestowrite;i++){      
      databuf[i] = buf[j];      
      j++;
      }
   //send(databuf, i); //DEBUG.
   /*
   if (writeEEPROMBlock(baseaddr, databuf, bytestowrite)){  //Write block and return ok if everything is fine
      buf[0] = '\x6F'; //Send OK back      
      }
   else {
      buf[0] = '\x6E'; //Send NOK back 
      }
   */
   int byteswritten = 0x75;
   byteswritten = writeEEPROMBlock(baseaddr, databuf, bytestowrite);
   //byte bwritten = 0x00;
   //if (byteswritten >=0 && byteswritten <= 255){  // value is between 0 and 255
   byte bwritten = byteswritten & 0xFF; // AND the integer with 0xFF (all Ones in binary) to get the correct value
   //     }
   buf[0] = bwritten;

    send(buf, 1); //send the value...
    //send(buf, bytestowrite); //DEBUG.
    return;
    }      
  else if(bytesavail >= 1) 
    {
      // Unhandled command or invalid message
      buf[0] = '\x75' ; //indicate unknown command Error 'u'
      send(buf, 1); //Send
      return;
    }
//This part will always be executed, even if there is not data available on the serial

return;
} 

void send(byte *sendbuf, int len){
  Serial.write(sendbuf, len);
  return;  
  }

void loop() {
  receive();
  }
