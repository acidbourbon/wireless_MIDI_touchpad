/*
 Diese Library bassiert auf der Arduino RFM70 Library, ich habe sie lediglich so
 angepasst, dass man sie auf einem AVR und ohne zus�tzliche Libs von Arduino
 verwenden kann.

 Es sind nur die wichtigstens Funktionen implementiert!
 Bei Fragen oder Anregungen:
 Mein Blog: http:://projects.web4clans.com
 Meine Email: daniel.weber@web4clans.com

 */

//#define UART_BAUD_RATE 19200
#define UART_BAUD_RATE 31250 //midi baudrate
#define F_CPU 12e6

#define MIDI_CHAN	0
#define CC_X	16
#define CC_Y	(CC_X+1)
#define CC_Z	(CC_Y+1)

#define LED PD7
#define LEDDDR DDRD
#define LEDPORT PORTD

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//#include "lcd.h"
//#include "lcd.c"

#include "uart.c"
#include "rfm70.c"
#include "softspi.c"

// convert an unsigned integer to string
void my_uitoa(uint32_t zahl, char* string, uint8_t no_digits) {
	int8_t i; // schleifenzähler

	string[no_digits] = '\0'; // String Terminator
	for (i = (no_digits - 1); i >= 0; i--) {
		if (zahl == 0 && i < (no_digits - 1)) {
			string[i] = ' ';
		} else {
			string[i] = (zahl % 10) + '0';
		} // Modulo rechnen, dann den ASCII-Code von '0' addieren
		zahl /= 10;
	}

}

void set_poti_pos(uint8_t pos) {

	softspi_select();
	softspi_write(0b00000000);
	softspi_write(pos);
	softspi_deselect();

}

void send_midi_cc(uint8_t cc, uint8_t val) {

	uart_putc(0b10110000 | MIDI_CHAN); //send status byte with channel number
	uart_putc(cc & 0b01111111); //send controller number, MSB masked out for security
	uart_putc(val & 0b01111111); //send value, MSB masked for security
}

int main() {

	uint8_t buffer[10];
	char textbuffer[10];

	LEDDR |= 1 << LED;
	LEDPORT &= ~(1 << LED);

	sei();
	uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU));
	softspi_init();

	//DDRA |= (1<<PA4);
	_delay_ms(1000);
	uart_puts("System Ready\n");

	Begin();

	setMode(1); // set mode r
	setChannel(8);

	uint8_t x, y, z;

	while (1) {

		if (receivePayload(buffer)) {
			if (buffer[0] == 'F' && buffer[4] == '0') {
				static xmem, ymem, zmem = 0;
				x = buffer[1];
				y = buffer[2];
				z = buffer[3];

				set_poti_pos(x);
				if (0) { // UART text out
					my_uitoa(x, textbuffer, 3);
					uart_puts("x: ");
					uart_puts(textbuffer);
					my_uitoa(y, textbuffer, 3);
					uart_puts(" y: ");
					uart_puts(textbuffer);
					my_uitoa(z, textbuffer, 3);
					uart_puts(" z: ");
					uart_puts(textbuffer);

					uart_puts("\r\n");
				} else { // MIDI out

					if (x != xmem) {
						send_midi_cc(CC_X, x >> 1);
						xmem = x;
					}
					if (y != ymem) {
						send_midi_cc(CC_Y, y >> 1);
						ymem = y;
					}
					if (z != zmem) {
						send_midi_cc(CC_Z, z >> 1);
						zmem = z;
						if (z) {
							LEDPORT |= 1 << LED;
						} else {
							LEDPORT &= ~(1 << LED);
						}
					}

				}
			}
		}

	}
}
