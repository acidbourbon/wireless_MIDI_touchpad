#define F_CPU 12e6

#define UART_BAUD_RATE      19200

#define PAD_INPUT_DOWN 5
#define PAD_INPUT_UP 2
#define PAD_INPUT_LEFT 4
#define PAD_INPUT_RIGHT 6
#define PAD_INPUT_ENTER 3
#define PAD_INPUT_BACK 1
#define PAD_INPUT_VOID 0

#define KEYPAD_MODE 0
#define RELATIVE_MODE 1
#define ABSOLUTE_MODE 2

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

	uart_puts("\n\n\nTouchpad Demo Application\n\r");

// set desired demo mode here!
//#define DEMO_MODE KEYPAD_MODE
//#define DEMO_MODE ABSOLUTE_MODE
#define DEMO_MODE RELATIVE_MODE

#if DEMO_MODE == KEYPAD_MODE

	uart_puts("you selected the keypad demo modus:\n\r");
	touchpad_set_abs_mode();
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

#elif DEMO_MODE == ABSOLUTE_MODE
	uart_puts("you selected the absolute position demo modus:\n\r");
	touchpad_set_abs_mode();
	while (1) {

		_delay_ms(20);
		// Register 0 auslesen
		touchpad_read();
		uart_puts("x_pos: ");
		my_uitoa(x_abs(),stringbuffer,4);
		uart_puts(stringbuffer);
		uart_puts("\ty_pos: ");
		my_uitoa(y_abs(),stringbuffer,4);
		uart_puts(stringbuffer);
		uart_puts("\tz_pressure: ");
		my_uitoa(z_pressure(),stringbuffer,4);
		uart_puts(stringbuffer);
		uart_puts("\r");

	}
#elif DEMO_MODE == RELATIVE_MODE

	uart_puts("you selected the relative position demo modus:\n\r");
	touchpad_set_rel_mode_100dpi();
//	touchpad_set_rel_mode_200dpi(); uncomment if you want double resolution
	uint8_t x, y = 0;
	int8_t dx, dy = 0;

	while (1) {

		_delay_ms(20);
		// Register 0 auslesen
		touchpad_read();

		dx = delta_x();
		dy = delta_y();

		if (x + dx > 255) {
			x = 255;
		} else if (x + dx < 0) {
			x = 0;
		} else {
			x = (x + dx);
		}

		if (y + dy > 255) {
			y = 255;
		} else if (y + dy < 0) {
			y = 0;
		} else {
			y = (y + dy);
		}

		uart_puts("x_pos: ");
		my_uitoa(x, stringbuffer, 4);
		uart_puts(stringbuffer);
		uart_puts("\ty_pos: ");
		my_uitoa(y, stringbuffer, 4);
		uart_puts(stringbuffer);
		uart_puts("\r");

	}

#endif

// end of main
	return 0;
}

