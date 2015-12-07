// Purpose of testUsart is to receive and send midi signals. Debubugged with LEDS
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/delay.h>

#define MYUBRR 7//F_CPU/16/BAUD-1


// -- Header Functions --
// Usart
void usart_init(uint16_t);
void usart_putchar(uint8_t);
uint8_t usart_getchar(void);
void usart_flush(void);
int usart_hasdata(void);

// EEPROM
void EEPROM_write(unsigned int, unsigned char);
uint8_t EEPROM_read(unsigned int);

// ADC
void ADC_init(void);

//static uint8_t data[100] EEMEM;

int main(int argc, char *argv[]) {
  // Some form of main
}

void ADC_init() {
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

  ADMUX |= (1 << REFS0); // Set ADC reference to AVCC
  ADMUX |= (1 << ADLAR); // Left adjust ADC result to allow easy 8 bit reading

  // No MUX values needed to be changed to use ADC0

  ADCSRA |= (1 << ADFR);  // Set ADC to Free-Running Mode
  ADCSRA |= (1 << ADEN);  // Enable ADC

  ADCSRA |= (1 << ADIE);  // Enable ADC Interrupt
  sei();  // Enable Global Interrupts

  ADCSRA |= (1 << ADSC);  // Start A2D Conversions
}

// -- usart functions --
void usart_init(uint16_t ubrr) {
  // Set baud rate
  UBRRH = (ubrr >> 8);
  UBRRL = ubrr;

  /* Asynchronous mode
   * No Parity
   * 1 Stop Bit
   * char size 8 bits
   */

  // Set frame format: 8 bit data, 1 stop bit
  UCSRC = (1<<URSEL)|(3<<UCSZ0);

  // Enable receiver and transmitter
  UCSRB = (1<<RXEN)|(1<<TXEN);

}

void usart_putchar(uint8_t data) {
  while ((UCSRA & (1 << UDRE)) == 0); // waits for transmit buffer to be empty, checks UDRE==1 and Data registry
  UDR = data; //transmits the data
}

uint8_t usart_getchar() {
  while ((UCSRA & (1 << RXC)) == 0); // Waits for RXC==1 (receive complete)
  return UDR;
}

void usart_flush() {
  while ((UCSRA & (1 << UDRE)) == 0); // Waits for buffer to be empty.
}

int usart_hasdata() {
  if (UCSRA & (1<<RXC))
  	return 1;
  else
    return 0;
}

void EEPROM_write(unsigned int uniAddress, unsigned char ucData) {
	while(EECR & (1 << EEWE)); //wait for write to clear

	EEAR = uniAddress; // Set up addr and data reg
	EEDR = ucData;

	EECR |= (1<<EEMWE); // Write logical one to EEMWE
	EECR |= (1<<EEWE); // Start eeprom write by setting EEWE
}

uint8_t EEPROM_read(unsigned int uniAddress) {
	while(EECR & (1<<EEWE)); // Wait for completion of write

	EEAR = uniAddress; // Set up addr
	EECR |= (1<<EERE); // Start eeprom read by writing EERE

	return EEDR;
}
