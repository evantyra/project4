// Purpose of testUsart is to receive and send midi signals. Debubugged with LEDS
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/delay.h>
#include <math.h>

//#define CF_CPU 4000000 // 4MHz
//#define BAUD 31250 // not sure how to find Baud
//#define MYUBRR 7//F_CPU/16/BAUD-1

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

//static uint8_t data[100] EEMEM;

int main(int argc, char *argv[]) {
  	DDRD = 0b00000010; // pin 1: output midi out, pin 0: midi in for storing
  	DDRB = 0b11110000; // LEDs for debugging -- TODO THIS IS NOW HEXA SWITCH
  	DDRA = 0b00000000;

  	// PORTA0 - photo1
  	// PORTA1 - photo1
  	// PORTA2 - photo1
  	// PORTA3 - record
  	// PORTA4 - playback

  	usart_init(7); // MYUBRR

	// Will be used to hold values of PINS for switches
	int record, playback, i;

	// Used to keep track of values for Compression and Decompression
	int notesRecorded = 0;
	int notesToPlay = 0;
	uint8_t uniqueNotesRecorded = 0;
	uint8_t uniqueNotesToPlay = 0;
	uint8_t *playOrder = malloc(1024*sizeof(uint8_t));
	uint8_t *uniqueNotesDictR = malloc(128*sizeof(uint8_t));
	uint8_t *uniqueNotesDictP = malloc(128*sizeof(uint8_t));

  	// Temporary variables to hold data from USART
  	uint8_t status, note, velocity;

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
		while (record & (record ^ playback)) {
			// Reassigns and updates record and playback		
			record = bit_is_set(PINA, 5);
			playback = bit_is_set(PINA, 6);
			PORTB = 0;

			// EEPROM Overflow check
			if ((int)notesRecorded > 1023) {
				usart_flush();
				continue;
			}

			status = usart_getchar();
			if (status == 0x90) { // Status = note on
   				note = usart_getchar();
				velocity = usart_getchar();

				if (bit_is_set(PINA, 5))
					break;

				// Records data about note into arrays
				uint8_t note_is_unique = 1;
				for (i = 0; i < uniqueNotesRecorded; i++) {
					// If note is found add current index to playOrder
					if (uniqueNotesDictR[i] == note) {
						playOrder[notesRecorded] = i;
						notesRecorded += 1;
						note_is_unique = 0;
					}
				}

				// If the note is unique, append to dictionary and increment
				// number of unique notes recorded in this session
				if (note_is_unique == 1) {
					uniqueNotesDictR[uniqueNotesRecorded] = note;
					playOrder[notesRecorded] = uniqueNotesRecorded;
					notesRecorded += 1;
					uniqueNotesRecorded += 1;
				}
			}
			
			// Signals end of recording, when this happens
			// we can run our compression algorithm and push to EEPROM
			if (!(record ^ playback)) {
				PORTB = 160;
				// Compression
				uint8_t storageLength = 0;
				if (uniqueNotesRecorded < 2)
					storageLength = 1;
				else if (uniqueNotesRecorded < 4)
					storageLength = 2;
				else if (uniqueNotesRecorded < 8)
					storageLength = 3;
				else if (uniqueNotesRecorded < 16)
					storageLength = 4;
				else if (uniqueNotesRecorded < 32)
					storageLength = 5;
				else if (uniqueNotesRecorded < 64)
					storageLength = 6;
				else
					storageLength = 7;

				uint8_t currentBitIndex = 0;
				int storageByte = 0, storageByteIndex = 0;

				for (i = 0; i < notesRecorded; i++) {
					if (currentBitIndex + storageLength >= 8) {
						if (currentBitIndex != 0)
							storageByte = storageByte + (playOrder[i] << currentBitIndex);
						else
							storageByte = storageByte + playOrder[i];
						EEPROM_write(storageByte, storageByteIndex);
						storageByteIndex++;
						currentBitIndex = (currentBitIndex + storageLength) % 8;
						if (currentBitIndex != 0)
							storageByte = storageByte + (playOrder[i] >> (storageLength - currentBitIndex));
					}
					else {
						if (currentBitIndex != 0)
							storageByte = storageByte + (playOrder[i] << currentBitIndex);
						else
							storageByte = storageByte + playOrder[i];
						currentBitIndex = currentBitIndex + storageLength;
					}
				}

				// Assign to playback variables and reset record variables
				for (i = 0; i < uniqueNotesRecorded; i++) {
					uniqueNotesDictP[i] = uniqueNotesDictR[i];
					uniqueNotesDictR[i] = 0;
				}
				
				notesToPlay = notesRecorded;
				notesRecorded = 0;
				uniqueNotesToPlay = uniqueNotesRecorded;
				uniqueNotesRecorded = 0;
			}
		}

		// Playback logic
		while (playback & (record ^ playback)) {
			PORTB = 127;
			if (notesToPlay == 0) {
				// Reassigns and updates record and playback		
				record = bit_is_set(PINA, 5);
				playback = bit_is_set(PINA, 6);
				continue;
			}

			uint8_t noteToPlay;

			// Length of data holding each note
			uint8_t storageLength = 0;
			
			if (uniqueNotesToPlay < 2)
				storageLength = 1;
			else if (uniqueNotesToPlay < 4)
				storageLength = 2;
			else if (uniqueNotesToPlay < 8)
				storageLength = 3;
			else if (uniqueNotesToPlay < 16)
				storageLength = 4;
			else if (uniqueNotesToPlay < 32)
				storageLength = 5;
			else if (uniqueNotesToPlay < 64)
				storageLength = 6;
			else
				storageLength = 7;

			int byteIndex = 0;
			uint8_t compareBits = pow(2, storageLength) - 1;
			uint8_t currentByteRead = EEPROM_read((uint8_t)byteIndex);
			uint8_t currentBitIndex = 0;

			byteIndex++;

			for(i = 0; i < notesToPlay; i++) {
				// Adjusts read byte and logical AND it with the current read byte
				if (currentBitIndex != 0)
					noteToPlay = compareBits & (currentByteRead >> currentBitIndex);
				else
					noteToPlay = compareBits & currentByteRead;

				// If the whole note wasn't read, grab new byte from EEPROM
				if (currentBitIndex + storageLength >= 8) {
					currentByteRead = EEPROM_read((uint8_t)byteIndex);
					currentBitIndex = (currentBitIndex + storageLength) % 8;
					byteIndex++;

					if (currentBitIndex != 0)
						noteToPlay = noteToPlay + 
									(((int)(pow(2, currentBitIndex + 1) - 1) & currentByteRead) <<
									(storageLength - currentBitIndex));
				}
				else {
					currentBitIndex = currentBitIndex + storageLength;
				}
			
				// Push in Note On
				usart_putchar(0x90);  
				usart_putchar(uniqueNotesDictP[i]);
				usart_putchar(0x64);

				_delay_ms(1000);

				// Push in Note off of same note
				usart_putchar(0x80);  
				usart_putchar(uniqueNotesDictP[i]);
				usart_putchar(0x40);
			
				// Hexaswitch controls how fast notes play back
				_delay_ms(1000); 
	   		}
	
			_delay_ms(5000);
			// Reassigns and updates record and playback		
			record = bit_is_set(PINA, 5);
			playback = bit_is_set(PINA, 6);
		}
  	}
}

// -- USART Functions --
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
  while ((UCSRA & (1 << RXC)) == 0) {
  	if (bit_is_set(PINA, 5) == 0)
		return -1;
  }; // Waits for RXC==1 (receive complete)
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

// -- EEPROM Functions --
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
