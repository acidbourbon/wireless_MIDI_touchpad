#define F_CPU 12e6

#define UART_BAUD_RATE      19200

#define PAD_INPUT_DOWN 5
#define PAD_INPUT_UP 2
#define PAD_INPUT_LEFT 4
#define PAD_INPUT_RIGHT 6
#define PAD_INPUT_ENTER 3
#define PAD_INPUT_BACK 1
#define PAD_INPUT_VOID 0


#include <util/delay.h>
#include <avr/interrupt.h>


char stringbuffer[16];

#include "touchpad_franky.c"
#include "uart.c"



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




int main(void) {

	uint8_t field_val = 0;


//###############inits!####################

	uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU));
	touchpad_init();

	_delay_ms(500);

	sei();
	touchpad_set_abs_mode();


	uart_puts("Touchpad Demo Application\n\r");


	while (1) {

		_delay_ms(20);
		// Register 0 auslesen

		touchpad_read();

		field_val = decode_field();

		switch (field_val) {

		case PAD_INPUT_LEFT:
			uart_puts("left\n\r");
			break;
		case PAD_INPUT_RIGHT:
			uart_puts("right\n\r");
			break;
		case PAD_INPUT_UP:
			uart_puts("up\n\r");
			break;
		case PAD_INPUT_DOWN:
			uart_puts("down\n\r");
			break;
		case PAD_INPUT_BACK:
			uart_puts("back\n\r");
			break;
		case PAD_INPUT_ENTER:
			uart_puts("enter\n\r");
			break;
		default:
			break;
		}



	}

// end of main
	return 0;
}

