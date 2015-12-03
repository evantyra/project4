#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/delay.h>

 //Configure timers/USART/interrupts/etc

#define CLOCK 16000000
#define BAUD 31250 // not sure how to find Baud

// Header Functions
void receive(void);
void transmit(void);
void flushUsart(void);
void readE2PROM(void);
void writeE2PROM(void);

void init_usart(void);
//eeprom
//eeprom_read_block();
//  uint8_t byteRead = eeprom_read_byte((uint8_t*)23); // read the byte in location 23
//eeprom_write_block();
//  eeprom_write_byte ((uint8_t*) 23, 64); //  write the byte 64 to location 23 of the EEPROM

  //Storage DataEEPROM
 	//Communication Usart
 	//Counter Timer1

int main(int argc, char *argv[]) {
  int record = 0, playback = 0;
	//initialize USART

  DDRD = 0b00000010; // pin 1: output midi out, pin 0: midi in for storing
  DDRB = 0b11111111; // LEDs for debugging

  // -- LEDS light up to display startup --
  PORTB = 0b00000001;
  _delay_loop_2(1000); //delay of 1s
  PORTB = 0b00000011;
  _delay_loop_2(1000);
  PORTB = 0b00000111;
  _delay_loop_2(1000);
  PORTB = 0b00000000;

  // -- Start Listening --
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
