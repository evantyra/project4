// Purpose of testUsart is to receive and send midi signals. Debubugged with LEDS
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/delay.h>

#define  16000000 // 16MHz
#define BAUD 31250 // not sure how to find Baud
#define MYUBRR F_CPU/16/BAUD-1

// -- Header Functions --
// Usart Functions
void usart_init(uint16_t);
void usart_putchar(char);
char usart_getchar(void);
// EEPROM
void readE2PROM(void);
void writeE2PROM(void);

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
	char led;
  while (1) {
    // Test USART receive
    led = usart_getchar(); // Not sure if it is blocking
    PORTB = led;
    _delay_loop_2(5000); // LEDS light for 5s
    PORTB = 0x00; // Clear Lights

    // Test USART send
    usart_putchar('c');
    _delay_loop_2(5000);

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

