//#define TAKT 7372800  //Quarzfrequenz
//#define TAKT 12e6
#define TAKT F_CPU

//Bits Makro
#define BITS(H,L)  (0b##H ##L)

#define MIN_ABS_X 150
#define MAX_ABS_X 1450
#define MIN_ABS_Y 210
#define MAX_ABS_Y 1130

#define PAD_WIDTH (MAX_ABS_X-MIN_ABS_X)
#define PAD_HEIGHT (MAX_ABS_Y-MIN_ABS_Y)

#define PAD_ROWS 2
#define PAD_COLS 3

#define PAD_COL_WIDTH (PAD_WIDTH/PAD_COLS)
#define PAD_ROW_HEIGHT (PAD_HEIGHT/PAD_ROWS)

//Vorteiler Timer 0 und 1 bei 2313, 8515, Mega8 (nicht Timer 2)
#define TIMER_VT_1_B BITS(0000,0001)  
#define TIMER_VT_8_B BITS(0000,0010)  
#define TIMER_VT_64_B BITS(0000,0011)
#define TIMER_VT_256_B BITS(0000,0100)
#define TIMER_VT_1024_B BITS(0000,0101)

#define TIMER_VT_1 1  
#define TIMER_VT_8 8  
#define TIMER_VT_64 64
#define TIMER_VT_256 256
#define TIMER_VT_1024 1024

// PORT B
#define ADB_BIT (1<<2)  // ADB
#define ADB_PIN PIND
#define ADB_POUT PORTD
#define ADB_PDIR DDRD

typedef unsigned char u08;
typedef unsigned short int u16;

#define COM_TALK0 BITS(0011,1100) //Adresse 3, Talk, Register0
#define COM_TALK1 BITS(0011,1101) //Adresse 3, Talk, Register1
#define COM_LISTEN1 BITS(0011,1001) //Adresse 3, Listen, Register1
#define COM_TALK2 BITS(0011,1110) //Adresse 3, Talk, Register2
#define COM_TALK3 BITS(0011,1111) //Adresse 3, Talk, Register3
#define COM_LISTEN3 BITS(0011,1011) //Adresse 3, Listen, Register3
#define MAX_ADB 9
u08 adb_werte[MAX_ADB];
u08 t0ovfcount;
volatile u08 adb_data_length;

ISR( TIMER0_OVF_vect) {
	t0ovfcount++;
}

// ADB Register auslesen / schreiben 
// Rückgabe Anzahl der Empfangenen Daten 
// Befehl in adb_werte[0]. Empfangene bzw. zu sendende Daten in adb_werte[1 bis 8]

uint16_t t0ext(void) {
	return (t0ovfcount * 256 + (u16) TCNT0);
}

void t0rst(void) {
	TCNT0 = 0; // Zähler auf null setzen
	t0ovfcount = 0;

}

u08 adb(void) {
	u08 bitpos;
	u08 z_byte;
	u08 anzahl_out;
	//Teilerwerte TC0
#define ADB_VORTEILER_A TIMER_VT_64
#define ADB_VORTEILER_A_BIT TIMER_VT_64_B
#define ADB_VORTEILER_B TIMER_VT_8
#define ADB_VORTEILER_B_BIT TIMER_VT_8_B
#define ADB_IMPULS_LANG TAKT/ADB_VORTEILER_B*65/1000000 //97
#define ADB_IMPULS_KURZ TAKT/ADB_VORTEILER_B*35/1000000	//52
#define ADB_IMPULS_ATT TAKT/ADB_VORTEILER_A*570/1000000 // 106
	//#define ADB_IMPULS_SYNC TAKT/ADB_VORTEILER_B*70/1000000
#define ADB_IMPULS_TLT TAKT/ADB_VORTEILER_B*250/1000000 // !!! 375 mach das mit anderem prescaler
//	#define ADB_IMPULS_TLT TAKT/ADB_VORTEILER_A*31/1000000 // modified
	//#define ADB_IMPULS_PAUSE TAKT/ADB_VORTEILER_B*200/1000000

	//Attention Signal
	TCCR0 = ADB_VORTEILER_A_BIT;
	t0rst(); //  TCNT0 = 0; // Zähler auf null setzen
	ADB_PDIR |= ADB_BIT; // Ausgang 0
	ADB_POUT &= ~ADB_BIT;
	while (t0ext() < ADB_IMPULS_ATT)
		; // Warteschleife

	TCCR0 = ADB_VORTEILER_B_BIT; // ab hier alles im schnellen modus

	anzahl_out = 1; // Mindestens Befehl übertragen
	z_byte = 0;
	do {
		// START BIT (1)
		t0rst(); //  TCNT0 = 0;
		ADB_PDIR |= ADB_BIT; // Ausgang 0
		ADB_POUT &= ~ADB_BIT;
		while (t0ext() < ADB_IMPULS_KURZ)
			;
		ADB_PDIR &= ~ADB_BIT; // Eingang über Pullup
		ADB_POUT |= ADB_BIT;
		while (t0ext() < ADB_IMPULS_KURZ + ADB_IMPULS_LANG)
			;

		for (; z_byte < anzahl_out; z_byte++) {
			for (bitpos = 128; bitpos; bitpos >>= 1) { //Daten ausgeben
				t0rst(); //  TCNT0 = 0;
				ADB_PDIR |= ADB_BIT; // Ausgang 0
				ADB_POUT &= ~ADB_BIT;
				if (adb_werte[z_byte] & bitpos)
					while (t0ext() < ADB_IMPULS_KURZ)
						;
				else
					while (t0ext() < ADB_IMPULS_LANG)
						;
				ADB_PDIR &= ~ADB_BIT; // Eingang über Pullup
				ADB_POUT |= ADB_BIT;
				while (t0ext() < ADB_IMPULS_KURZ + ADB_IMPULS_LANG)
					;
			}
		}
		// STOP BIT (0)
		t0rst(); //  TCNT0 = 0;
		ADB_PDIR |= ADB_BIT; // Ausgang 0
		ADB_POUT &= ~ADB_BIT;
		while (t0ext() < ADB_IMPULS_LANG)
			;
		ADB_PDIR &= ~ADB_BIT; // Eingang über Pullup
		ADB_POUT |= ADB_BIT;
		while (t0ext() < ADB_IMPULS_KURZ + ADB_IMPULS_LANG)
			;

		// Entscheidung, ob noch weitere Daten übertragen werden
		if (adb_werte[0] == COM_LISTEN3)
			anzahl_out = 2 + 1;
		else if (adb_werte[0] == COM_LISTEN1)
			anzahl_out = 8 + 1;
		else
			z_byte = 8;
	} while (z_byte < 2);

	// Warten auf Rückgabewerte sonst Abbruch
	// TCCR0 = ADB_VORTEILER_A_BIT; // micha modifikation
	t0rst(); //  TCNT0 = 0;

	do {
		if (t0ext() > ADB_IMPULS_TLT) {
			return (0);
		}
	} while (ADB_PIN & ADB_BIT);
	while (!(ADB_PIN & ADB_BIT))
		; // Start Bit

	z_byte = 1;
	while (z_byte < MAX_ADB) {
		adb_werte[z_byte] = 0;
		for (bitpos = 128; bitpos; bitpos >>= 1) { //Bits einlesen
			//TCCR0 = ADB_VORTEILER_B_BIT; //micha modifikation
			t0rst(); //  TCNT0 = 0;
			// Abbruch wenn keine weiteren Bytes übertragen werden.
			do {
				if (t0ext() > ADB_IMPULS_KURZ + ADB_IMPULS_LANG) {
					return (z_byte - 1);
				}
			} while (ADB_PIN & ADB_BIT);
			t0rst(); //  TCNT0 = 0;
			while (!(ADB_PIN & ADB_BIT))
				;
			if (t0ext() < (ADB_IMPULS_KURZ + ADB_IMPULS_LANG) / 2)
				adb_werte[z_byte] += bitpos;
		}
		z_byte++;
	}
	return (z_byte - 1);
}

void touchpad_init(void) {
	// Eingänge
	ADB_PDIR &= ~ADB_BIT;

	// Pull Up einschalten
	ADB_POUT |= ADB_BIT;

	//enable timer0 interrupt
	TIMSK |= (1 << TOIE0);
}

uint8_t touchpad_read(void) {
	adb_werte[0] = COM_TALK0;
	adb_data_length = adb();
	return adb_data_length;
}

void touchpad_set_abs_mode(void) {
	// Auf Absolutmodus umschalten
	adb_werte[0] = COM_LISTEN3;
	adb_werte[1] = BITS(0110,0011);
	adb_werte[2] = 4; //CDM Modus
	adb();

	adb_werte[0] = COM_TALK1;
	adb(); // Werte holen

	adb_werte[0] = COM_LISTEN1;
	adb_werte[7] = 0x00; //Absolutmodus
	adb();
}

void touchpad_set_rel_mode_100dpi(void) {
	// Auf Relativmodus umschalten
	adb_werte[0] = COM_LISTEN3;
	adb_werte[1] = BITS(0110,0011);
	adb_werte[2] = 1; //default modus: 100 dpi mouse
	adb();

}

void touchpad_set_rel_mode_200dpi(void) {
	// Auf Relativmodus umschalten
	adb_werte[0] = COM_LISTEN3;
	adb_werte[1] = BITS(0110,0011);
	adb_werte[2] = 2; //200 dpi mouse
	adb();

}

uint8_t z_pressure(void) {
	return ((adb_werte[5] & 0b01110000) >> 1) | (adb_werte[5] & 0b00000111);
}

uint16_t x_abs(void) {
	return ((adb_werte[4] & 0b111) << 10) | ((adb_werte[3] & 0b111) << 7)
			| (adb_werte[2] & 0b01111111);
}

uint16_t y_abs(void) {
	return ((adb_werte[4] & 0b01110000) << 6)
			| ((adb_werte[3] & 0b01110000) << 3) | (adb_werte[1] & 0b01111111);
}

int8_t delta_y(void) {
	if(adb_data_length ==0) {
		return 0;
	}
	if (adb_werte[1] & 0b01000000) {
		return -((128 - adb_werte[1]) & 0b00111111);
	} else {
		return adb_werte[1] & 0b00111111;
	}
}

int8_t delta_x(void) {
	if(adb_data_length ==0) {
		return 0;
	}
	if (adb_werte[2] & 0b01000000) {
		return -((128 - adb_werte[2]) & 0b00111111);
	} else {
		return adb_werte[2] & 0b00111111;
	}
}
/*
uint8_t touchpad_button_pressed(void){
	static uint8_t button_status=0;

	if(adb_data_length ==0) {
		return button_status;
	}
	if(adb_werte[1]&0b10000000){
		button_status=1;
		return 1;
	}else{
		button_status=0;
		return 0;
	}

}
*/

uint8_t decode_field(void) {
	static uint8_t last_pressure = 0;
	uint8_t pressure, i, j;
	uint8_t return_val = 0;
	static uint16_t x_mem, y_mem;
	uint16_t x, y;

	pressure = z_pressure();
	x = x_abs();
	y = y_abs();

	if (x) {
		x_mem = x;
	} else {
		x = x_mem;
	}

	if (y) {
		y_mem = y;
	} else {
		y = y_mem;
	}

	if ((pressure == 0) && (last_pressure > 0)) {

		i = (y - MIN_ABS_Y) / PAD_ROW_HEIGHT;
		j = (x - MIN_ABS_X) / PAD_COL_WIDTH;

		return_val = i * PAD_COLS + j + 1;

	}

	last_pressure = pressure;
	return return_val;
}
