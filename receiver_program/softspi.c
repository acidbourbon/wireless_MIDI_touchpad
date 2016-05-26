#define SOFTSPI_PORT PORTC
#define SOFTSPI_DDR DDRC
#define SOFTSPI_MOSI PC5
#define SOFTSPI_SCK PC4
#define SOFTSPI_CS PC3

void softspi_deselect() {
	SOFTSPI_PORT |= (1 << SOFTSPI_CS);
}

void softspi_select() {
	SOFTSPI_PORT &= ~(1 << SOFTSPI_CS);
}

void softspi_init() {
	SOFTSPI_PORT &= ~((1 << SOFTSPI_MOSI) | (1 << SOFTSPI_SCK));
	SOFTSPI_PORT |= (1 << SOFTSPI_CS);
	SOFTSPI_DDR |=
			((1 << SOFTSPI_MOSI) | (1 << SOFTSPI_SCK) | (1 << SOFTSPI_CS));
}

void softspi_write(unsigned char data_out) { //msb first
	unsigned char loop, mask;
	for (loop = 0, mask = 0x80; loop < 8; loop++, mask = mask >> 1) {
		//sclk = 0;
		SOFTSPI_PORT &= ~(1 << SOFTSPI_SCK);
		if (data_out & mask) {
			//mosi=1;
			SOFTSPI_PORT |= (1 << SOFTSPI_MOSI);
		} else {
			//mosi = 0;
			SOFTSPI_PORT &= ~(1 << SOFTSPI_MOSI);
		}
		//sclk = 1;
		SOFTSPI_PORT |= (1 << SOFTSPI_SCK);
	}
	//sclk = 0;
	SOFTSPI_PORT &= ~(1 << SOFTSPI_SCK);
}
