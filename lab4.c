#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <avr/eeprom.h>

 //Configure timers/USART/interrupts/etc

// Header Functions
void receive(void);
void transmit(void);
void flushUsart(void);
void readE2PROM(void);
void writeE2PROM(void);

}

//eeprom
//eeprom_read_block();
//  uint8_t byteRead = eeprom_read_byte((uint8_t*)23); // read the byte in location 23
//eeprom_write_block();
//  eeprom_write_byte ((uint8_t*) 23, 64); //  write the byte 64 to location 23 of the EEPROM

int main(int argc, char *argv[]) {
  //initialize USART
  //Enable Receiver and transmitter
  //initialize to 8 bits, 1 start, 1 stop bit

  //Storage DataEEPROM
  //Communication Usart
  //Counter Timer1

  while (1) {
    if (record) {
      //USART_Read();
      //Compress();
      //EEPROM_write();
    }
    if (playback) {
      //EEPROM_read();
      //Decompress();
      //USART_write();
    }
  }
}

void receive() {

}

void transmit() {

}

void flushUsart() {

}

void readE2PROM() {

}

void writeE2PROM() {

}
