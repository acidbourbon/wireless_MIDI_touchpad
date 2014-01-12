
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
#include "rfm70.c"

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

uint8_t x_abs_midi(){
	static uint8_t x_mem;
	if(z_pressure()){
		int32_t dummy = x_abs();
		dummy -= MIN_ABS_X;
		if(dummy < 0){
				dummy=0;
		}
		dummy *=255;
		dummy /=PAD_WIDTH;
		if(dummy > 255){
			dummy = 255;
		}
		x_mem = (uint8_t) dummy;
		return (uint8_t) dummy;
	}
	else return x_mem;

}

uint8_t y_abs_midi(){
	static uint8_t y_mem;
	if(z_pressure()){
		int32_t dummy = y_abs();
		dummy-=MIN_ABS_Y;
		if(dummy < 0){
				dummy=0;
		}
		dummy *=255;
		dummy /=PAD_HEIGHT;
		if(dummy > 255){
			dummy = 255;
		}
		y_mem = (uint8_t) dummy;
		return (uint8_t) dummy;
	}
	else return y_mem;
}

int main(void) {

	uint8_t field_val = 0;

//###############inits!####################

	uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU));

	touchpad_init(); // you need to call this to setup the I/O pin!
	Begin();
	setMode(0);// set mode t
	setChannel(8);

	_delay_ms(500);

	sei();

	uart_puts("\n\n\nTM1001A Touchpad debug output\n\r");

//######################################################################
// uncomment desired demo mode here!

//#define DEMO_MODE KEYPAD_MODE
#define DEMO_MODE ABSOLUTE_MODE
//#define DEMO_MODE RELATIVE_MODE
//######################################################################


#if DEMO_MODE == ABSOLUTE_MODE
	// begin of keypad mode demo block
	uart_puts("you selected the absolute position demo modus:\n\r");
	touchpad_set_abs_mode();// tell the touchpad you want to use it in the "absolute mode"

	uint8_t xmidi,ymidi,zmidi = 0;
	uint8_t databuffer[8];
	while (1) {

		_delay_ms(8);

		touchpad_read(); // read data from the touchpad
		xmidi=x_abs_midi();
		ymidi=y_abs_midi();
		zmidi=z_pressure()<<2;

		databuffer[0]='F';
		databuffer[1]=xmidi;
		databuffer[2]=ymidi;
		databuffer[3]=zmidi;
		databuffer[4]='0';
		sendPayload(databuffer,5, 0);



		uart_puts("x_pos: ");
		my_uitoa(xmidi,stringbuffer,4);// x_abs returns current x position of your finger
		uart_puts(stringbuffer);
		uart_puts("\ty_pos: ");
		my_uitoa(ymidi,stringbuffer,4);// y_abs returns current y position of your finger
		uart_puts(stringbuffer);
		uart_puts("\tz_pressure: ");// z_pressure returns current "pressure" (contact area) of your finger
		my_uitoa(zmidi,stringbuffer,4);
		uart_puts(stringbuffer);
		uart_puts("\r");

	}
	// end of absolute mode demo block


#endif

// end of main
	return 0;
}

