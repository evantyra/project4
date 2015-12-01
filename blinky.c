#include <avr/io.h> // Standard AVR header
#include <avr/delay.h> // Delay loop functions

int main(void)
{
  DDRA = 0xFF; // PORTA is output
  while (1) {
     for (int i=1; i<=128; i*=2) {
       PORTA = i;
       _delay_loop_2(30000);
        }
      for (int i=128; i>1; i/=2) {
        PORTA = i;
        _delay_loop_2(30000);
         }
  } // end while
}
