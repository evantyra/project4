// Purpose of testUsart is to receive and send midi signals. Debubugged with LEDS
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/delay.h>

//#define CF_CPU 4000000 // 4MHz
//#define BAUD 31250 // not sure how to find Baud
//#define MYUBRR 7//F_CPU/16/BAUD-1

// -- Header Functions --
void delay_ms(uint16_t count);

// Usart
void usart_init(uint16_t);
void usart_putchar(uint8_t);
uint8_t usart_getchar(void);
void usart_flush(void);
int usart_hasdata(void);

// EEPROM
void EEPROM_write(unsigned int, unsigned char);
uint8_t EEPROM_read(unsigned int);

//static uint8_t data[100] EEMEM;

int main(int argc, char *argv[]) {
  /**
   * PA0 to PA2 - Photo #1, #2, #3
   * PA5 - Record Switch
   * PA6 - Playback Switch
   *
   * PB0 to PB3 - Hexadecimal Rotary (1,2,4,8) respectively
   *
   */

  	DDRD = 0b00000010; // pin 1: output midi out, pin 0: midi in for storing
  	DDRB = 0b11110000; // HEXA SWITCH
  	DDRA = 0b00000000;

  	usart_init(7); // MYUBRR

	// Will be used to hold values of PINS for switches
	int record, playback;

  	// Temporary variables to hold data from USART
  	uint8_t status, note, velocity;

	int index = 0;

  	// -- Start Listening --
  	while (1) {

  		record = bit_is_set(PINA, 5);
		playback = bit_is_set(PINA, 6);

		// Idle state logic
		while (!(record ^ playback)) {
			// Reassigns and updates record and playback
			record = bit_is_set(PINA, 5);
			playback = bit_is_set(PINA, 6);

			// Flushes data if there is any pushed in during this state, will not happen
			// if the record switch has been turned on
			if (usart_hasdata() & !record)
				usart_flush();
		}

		// Record logic
		while (record && (record ^ playback)) {
			// Reassigns and updates record and playback
			record = bit_is_set(PINA, 5);
			playback = bit_is_set(PINA, 6);

			if (usart_hasdata() && record) {
				// EEPROM Overflow check
				if (index > 1023) {
					usart_flush();
					continue;
				}

				status = usart_getchar();
				if (status == 0x90) { // Status = note on
    				note = usart_getchar();
					velocity = usart_getchar();

					// This fixes when it gets stuck in the getchar loops, and record
					// is switched during that time. --> Don't think this is needed
					// if (!bit_is_set(PINA,3))
					// 	continue;

					// Only need to write note to EEPROM
					EEPROM_write((uint8_t*)index, note);	// TODO, CHANGE THIS TO ARRAY COMPRESSION
					index++;

					// PORTB = note; // for debugging purposes
				}
			}

		}

		// Playback logic
		while (playback && (record ^ playback)) {

			int playbackIndex = 0;
			uint8_t byteToPlay;
			while (playbackIndex < index) {
				byteToPlay = EEPROM_read((uint8_t*)playbackIndex);

				// Need to double check the pins that correspond
				int hexaSwitch = 0, photo = 0;

				if (!bit_is_set(PINB, 3))
					hexaSwitch += 8;
				if (!bit_is_set(PINB, 2))
					hexaSwitch += 4;
				if (!bit_is_set(PINB, 1))
					hexaSwitch += 2;
				if (!bit_is_set(PINB, 0))
					hexaSwitch += 1;

				//int hexaSwitch = ((bit_is_set(PINB, 3) << 3) +
				//				  (bit_is_set(PINB, 2) << 2) +
				//				  (bit_is_set(PINB, 1) << 1) +
				//				  bit_is_set(PINB, 0));
				
				if (!bit_is_set(PINA, 2))
					photo += 4;
				if (!bit_is_set(PINA, 1))
					photo += 2;
				if (!bit_is_set(PINA, 0))
					photo += 1;

				//int lights = ((bit_is_set(PINA, 0) << 2) +
				//			  (bit_is_set(PINA, 1) << 1) +
				//			  bit_is_set(PINA, 2));

				

				// Push in Note On
				 do {

				 	while (bit_is_set(PINA, 0)); // pause - takes priority
	
					while (bit_is_set(PINA, 2)) { // plays a predefined song
						usart_putchar(0x90);
						usart_putchar(byteToPlay);
						usart_putchar(0x64);

						// PORTB = byteToPlay; // for debugging purposes

						delay_ms(1000);

						// Push in Note off of same note
						usart_putchar(0x80);
						usart_putchar(byteToPlay);
						usart_putchar(0x40);
						int ans = 1500 - (100*hexaSwitch); // - (hexaSwitch*100)
						delay_ms(ans);
					}
					usart_putchar(0x90);
					usart_putchar(byteToPlay);
					usart_putchar(0x64);

					// PORTB = byteToPlay; // for debugging purposes

					delay_ms(1000);

					// Push in Note off of same note
					usart_putchar(0x80);
					usart_putchar(byteToPlay);
					usart_putchar(0x40);
					int ans = 1500 - (100*hexaSwitch); // - (hexaSwitch*100)
					delay_ms(ans);
				} while (bit_is_set(PINA, 1)); // stutter effect


				playbackIndex++;
	   		}

			// Reassigns and updates record and playback
			record = bit_is_set(PINA, 5);
			playback = bit_is_set(PINA, 6);
		}
  	}
}


void delay_ms(uint16_t count) {
  while(count--) {
    _delay_ms(1);
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
  while ((UCSRA & (1 << RXC)) == 0); // Waits for RXC==1 (receive complete)
  int temp =  UDR;
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
