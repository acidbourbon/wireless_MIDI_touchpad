/*
 * lcd_config.h
 *
 *  Created on: 31.07.2011
 *      Author: cyblord
 *
 */

#ifndef LCD_CONFIG_H_
#define LCD_CONFIG_H_

#define LCD_DATA	PORTD //Databus
#define LCD_DATA_D	DDRD  //Direction Databus
#define LCD_INPUT	PIND //Datenbus Eingang

//LCD 4-Bit Bus lines
#define LCD_D4		PD4
#define LCD_D5		PD5
#define LCD_D6		PD6
#define LCD_D7		PD7

#define LCD_CTRL	PORTC //Controlbus
#define LCD_CTRL_D	DDRC //Direction Controlbus

#define LCD_E		PC2 //Enable Portpin
#define LCD_RS		PC0 //RS Portpin
#define LCD_RW		PC1 //RW Portpin

#define LCD_LINES	2	//Anzahl Zeilen des LCDs, 1 oder 2
#define LCD_5X10	1	//Zeichenhöhe (1=5x10, 0 = 5x8 Pixel)

#define C080204		//Es wird ein C0802-04 (Pollin) LCD verwendet, falls nicht, bitte auskommentieren

//Hier können noch mehr Parameter verändert werden um möglichst viele Display Typen zu untersützen.
//Normalerweise müssen hier keine Änderungen vorgenommen werden.

#define LCD_FIRSTLINE	0x00 //DDRAM Start-Adressen der einzelnen Zeilen
#define LCD_SECONDLINE	0x40
#define STROBE_DELAY_US 2 	// Zeit in µS für die Enable bei einem Strobe High bleibt.
#define INIT_DELAY_MS	500 // Zeit in ms zwischen lcd_init() aufruf und init Sequenz



#endif /* LCD_CONFIG_H_ */
