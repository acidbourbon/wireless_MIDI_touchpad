/*
 * lcd.c
 *
 *  Created on: 30.07.2011
 *      Author: cyblord
 */

#include <avr/io.h>
#include <util/delay.h>
#include "lcd_config.h"

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#define INIT_DELAY() (_delay_ms(INIT_DELAY_MS));
#define STROBE_DELAY() (_delay_us(STROBE_DELAY_US));

//Display Commands


#define CMD_FUNCTION	32 	//Function Set
#define BIT_DL			16
#define BIT_N			8
#define BIT_F			4

#define CMD_ONOFF		8 //Display On/Off Command
#define BIT_D			4
#define BIT_C			2
#define BIT_B			1


//Untere 4 Bit von data auf den 4-Bit Datenbus ans Display legen
void setDataBus(unsigned char data) {
	if (data & 1) sbi(LCD_DATA,LCD_D4); else cbi(LCD_DATA,LCD_D4);
	if (data & 2) sbi(LCD_DATA,LCD_D5);	else cbi(LCD_DATA,LCD_D5);
	if (data & 4) sbi(LCD_DATA,LCD_D6);	else cbi(LCD_DATA,LCD_D6);
	if (data & 8) sbi(LCD_DATA,LCD_D7);	else cbi(LCD_DATA,LCD_D7);
}

void writeMode() {
	cbi(LCD_CTRL,LCD_RW);
}

void readMode() {
	sbi(LCD_CTRL,LCD_RW);
}

void commandMode() {
	cbi(LCD_CTRL,LCD_RS);
}

void dataMode() {
	sbi(LCD_CTRL,LCD_RS);
}

void strobe() {
	sbi(LCD_CTRL,LCD_E);
	STROBE_DELAY();
	cbi(LCD_CTRL,LCD_E);
}

void waitBusy() {
	readMode();
	commandMode();

	//D7 =Eingang
	cbi(LCD_DATA_D,LCD_D7);
	cbi(LCD_DATA,LCD_D7);
	int busy;
	do {
		sbi(LCD_CTRL,LCD_E);
		STROBE_DELAY();
		busy=bit_is_set(LCD_INPUT,LCD_D7);
		cbi(LCD_CTRL,LCD_E);

		strobe();
	}while (busy);

	//D7 = Ausgang
	sbi(LCD_DATA_D,LCD_D7);
}


//Untere 4 Bit von nibble ans LCD übertragen
void sendNibble(unsigned char nibble) {
	setDataBus(nibble);
	strobe();
}

//Ein Byte ans LCD übertragen als 2x4 Bit, höheres nibble zuerst.
void sendByte(unsigned char data) {
	sendNibble(data>>4);
	sendNibble(data);
}

void sendCommand(unsigned char cmd) {
	waitBusy();

	writeMode();
	commandMode();
	sendByte(cmd);
}

void sendData(unsigned char data) {
	waitBusy();

	writeMode();
	dataMode();
	sendByte(data);
}



void lcd_init() {

	//Portrichtungen setzen
	sbi(LCD_DATA_D,LCD_D4);
	sbi(LCD_DATA_D,LCD_D5);
	sbi(LCD_DATA_D,LCD_D6);
	sbi(LCD_DATA_D,LCD_D7);

	sbi(LCD_CTRL_D,LCD_E);
	sbi(LCD_CTRL_D,LCD_RS);
	sbi(LCD_CTRL_D,LCD_RW);

	INIT_DELAY();

	//LCD Initalisierungssequenz
	writeMode();
	commandMode();


	//Function Set: 0 0 1 DL N F — —

#ifdef C080204
	sendNibble(0x2); //4-Bit Mode (DL=0)
	_delay_ms(10);
#endif

	unsigned char cmd=CMD_FUNCTION;
	if (LCD_LINES!=1) cmd |= BIT_N; // Anzahl Zeilen
	if (LCD_5X10) cmd |= BIT_F; //Zeichenhöhe

	sendCommand(cmd); // 4-Bit Modus, Zeilen und Font setzen
	_delay_ms(10);
	//Display On/Off: 0 0 0 0 1 D C B
	sendCommand(0xC); //Display on, no cursor, no blink
	//Clear Display: 0 0 0 0 0 0 1
	sendCommand(0x1); //Clear Display and Cursor home
}

void lcd_print(char* string) {
	while(*string) {
		sendData(*string);
		string++;
	}
}

void lcd_goto(int line,int pos) {
	unsigned char adr;
	if (line==2) adr=LCD_SECONDLINE; else adr=LCD_FIRSTLINE;
	adr+=pos;
	sendCommand(128+adr);
}

void lcd_clear() {
	sendCommand(0x1);
}

void lcd_home() {
	sendCommand(0x2);
}

void lcd_set(int on,int cursor,int blink) {
	unsigned char cmd=CMD_ONOFF;
	if (on) cmd |= BIT_D;
	if (cursor) cmd |= BIT_C;
	if (blink) cmd |=BIT_B;
	sendCommand(cmd);
}
