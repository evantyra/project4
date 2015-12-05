// Purpose of testUsart is to receive and send midi signals. Debubugged with LEDS
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/delay.h>

#define CF_CPU 4000000 // 4MHz
#define BAUD 31250 // not sure how to find Baud
#define MYUBRR 7//F_CPU/16/BAUD-1


// -- Header Functions --
// Usart
void usart_init(uint16_t);
void usart_putchar(uint8_t);
uint8_t usart_getchar(void);
void usart_flush(void);

// EEPROM
void EEPROM_write(unsigned int, unsigned char);
uint8_t EEPROM_read(unsigned int);

//static uint8_t data[100] EEMEM;

int main(int argc, char *argv[]) {
  int record = 0, playback = 0, Timer1 = 0;

  DDRD = 0b00000010; // pin 1: output midi out, pin 0: midi in for storing
  DDRB = 0b11111111; // LEDs for debugging
  DDRA = 0b00000000;
  // PORTA0 - photo
  // PORTA1 - record
  // PORTA2 - playback

  //initialize USART
  usart_init(MYUBRR);

  //uint8_t* index;
  int index = 0;
  
  uint8_t status;
  uint8_t note;
  uint8_t velocity;

  // -- Start Listening --
  while (1) {
    
  	record = bit_is_set(PINA,1);
	playback = bit_is_set(PINA,2);

	if ((record && playback) || (!record && !playback)) {
	  usart_flush();
	  continue;
	}

    if (record) {

    	status = usart_getchar();
    	note = usart_getchar();
		velocity = usart_getchar();
		usart_flush();
		usart_flush();
		usart_flush();

		// This fixes when it gets stuck in the getchar loops, and record
		// is switched during that time.
		if (!bit_is_set(PINA,1))
			continue;

		EEPROM_write((uint8_t*)index, status);
		index++;
		EEPROM_write((uint8_t*)index, note);
		index++;
		EEPROM_write((uint8_t*)index, velocity);
		index++;

		PORTB = note; // for debugging purposes
	}

	if (playback) {
		int playbackIndex = 0;
		uint8_t byteToPlay;
		while (playbackIndex < index) {
			byteToPlay = EEPROM_read((uint8_t*)playbackIndex);
			usart_putchar(byteToPlay);

			playbackIndex++;
			
			//if(playbackIndex % 3 == 0)
		 		//_delay_ms(2000);
			_delay_ms(1000);
			PORTB = playbackIndex; // for debugging purposes
	   	}
    }

	usart_flush();
  }
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
  while ((UCSRA & (1 << RXC)) == 0);
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
