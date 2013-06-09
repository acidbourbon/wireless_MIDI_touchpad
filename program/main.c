
//######################################################################
// TM1001A touchpad demo project
// based on the work of Frank H. (franky1969) from the
// Mikrocontroller.net forum
//
// 2013 by Michael W. (acidbourbon)
// acidbourbon.wordpress.com
//######################################################################


// tested with an ATMega8 running at 12 MHz
// I suppose it should also run smoothly with 16 MHz
// set crystal freq here:
#define F_CPU 12e6

// set UART baudrate here
#define UART_BAUD_RATE      19200

#define KEYPAD_MODE 0
#define RELATIVE_MODE 1
#define ABSOLUTE_MODE 2

#include <util/delay.h>
#include <avr/interrupt.h>

// dummy string used for integer to string conversion
char stringbuffer[16];

// this is where all the nice touchpad functions come from:
#include "TM1001A.c"

// This program uses "Interrupt UART library with receive/transmit circular buffers"
// by Peter Fleury
#include "uart.c"

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

int main(void) {

	uint8_t field_val = 0;

//###############inits!####################

	uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU));

	touchpad_init(); // you need to call this to setup the I/O pin!

	_delay_ms(500);

	sei();

	uart_puts("\n\n\nTM1001A Touchpad Demo Application\n\r");

//######################################################################
// uncomment desired demo mode here!

#define DEMO_MODE KEYPAD_MODE
//#define DEMO_MODE ABSOLUTE_MODE
//#define DEMO_MODE RELATIVE_MODE
//######################################################################

#if DEMO_MODE == KEYPAD_MODE
	// begin of keypad mode demo block
	// current configuration is: 3 colums, 2 rows => 6 touch buttons
	// this can be changed by the PAD_ROWS/PAD_COLS defines in the TM1001A.c file

	//   -------------------------
	//   | back  |  up   | enter |
	//   -------------------------
	//   | left  |  down | right |
	//   -------------------------

	uart_puts("you selected the keypad demo modus:\n\r");
	touchpad_set_abs_mode(); // keypad functionality uses the "absolute mode"
	while (1) {

		_delay_ms(20);

		touchpad_read(); // read values from the touchpad

		field_val = decode_field(); // decode_field returns the number of the
		// touch button that was pressed last. or zero if nothing happened

		switch (field_val) {

		case 4:
			uart_puts("left\n\r");
			break;
		case 6:
			uart_puts("right\n\r");
			break;
		case 2:
			uart_puts("up\n\r");
			break;
		case 5:
			uart_puts("down\n\r");
			break;
		case 1:
			uart_puts("back\n\r");
			break;
		case 3:
			uart_puts("enter\n\r");
			break;
		default:
			break;
		}

	}
	// end of keypad mode demo block

#elif DEMO_MODE == ABSOLUTE_MODE
	// begin of keypad mode demo block
	uart_puts("you selected the absolute position demo modus:\n\r");
	touchpad_set_abs_mode();// tell the touchpad you want to use it in the "absolute mode"
	while (1) {

		_delay_ms(20);

		touchpad_read(); // read data from the touchpad
		uart_puts("x_pos: ");
		my_uitoa(x_abs(),stringbuffer,4);// x_abs returns current x position of your finger
		uart_puts(stringbuffer);
		uart_puts("\ty_pos: ");
		my_uitoa(y_abs(),stringbuffer,4);// y_abs returns current y position of your finger
		uart_puts(stringbuffer);
		uart_puts("\tz_pressure: ");// z_pressure returns current "pressure" (contact area) of your finger
		my_uitoa(z_pressure(),stringbuffer,4);
		uart_puts(stringbuffer);
		uart_puts("\r");

	}
	// end of absolute mode demo block

#elif DEMO_MODE == RELATIVE_MODE
	// begin of relative mode demo block
	uart_puts("you selected the relative position demo modus:\n\r");
	touchpad_set_rel_mode_100dpi();// use touchpad in relative mode
//	touchpad_set_rel_mode_200dpi(); // uncomment this line if you want double resolution
	uint8_t x, y = 0;
	int8_t dx, dy = 0;

	while (1) {

		_delay_ms(20);

		touchpad_read(); // read data from touchpad

		dx = delta_x();// returns the amount your finger has moved in x direction since last readout
		dy = delta_y();// returns the amount your finger has moved in y direction since last readout

		// increment/decrement some dummy variables with the
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
	// end of relative mode demo block

#endif

// end of main
	return 0;
}

