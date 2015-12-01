#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

 //Configure timers/USART/interrupts/etc

// Header Functions
void receive(void);
void transmit(void);
void flushUsart(void);
void readE2PROM(void);
void writeE2PROM(void);

}
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
