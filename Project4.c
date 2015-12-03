#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/delay.h>

//Configure timers/USART/interrupts/etc

#define  16000000 // 16MHz
#define BAUD 31250 // not sure how to find Baud
#define MYUBRR F_CPU/16/BAUD-1

// -- Header Functions --
// Usart Functions
void usart_init(uint16_t);
void usart_putchar(char);
char usart_getchar(void);
// EEPROM
void eeprom_write(uint_t);
void eeprom_read(uint_t);

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
  int Timer1 = 0;

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

  //initialize USART
  usart_init(MYUBBR);

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

// -- usart functions --
void usart_init(uint16_t ubrr) {
  // Set baud rate
  UBRR = ubrr;
  /* Asynchronous mode
   * No Parity
   * 1 Stop Bit
   * char size 8 bits
   */

  // Set frame format: 8 bit data, 1stop bit
  UCSRC=(1<<URSEL)|(3<<UCSZ0);

  // Enable receiver and transmitter
  UCSRB = (1<<RXEN)|(1<<TXEN);

}

void usart_putchar(char data) {
  while ( !(UCSRA & (_BV(UDRE))) ); // waits for transmit buffer to be empty, checks UDRE==1 and Data registry
  UDR = data; //transmits the data
}

char usart_getchar() {
  while ( !(UCSRA & (_BV(RXC))) ); // Waits for RXC==1 (receive complete)
  return UDR;
}

void flushUsart() {
  while ( !(UCSRA & (_BV(UDRE))) ); // Waits for buffer to be empty.
}

void eeprom_write(uint_t *to) {
  eeprom_write_block(to, data, 100);
}

void eeprom_read(uint_t *from) {
  eeprom_read_block(from, data, 100);
}
