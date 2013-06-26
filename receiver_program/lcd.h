/*
 * lcd.h
 *
 *  Created on: 31.07.2011
 *      Author: cyblord
 *
 * Diese lib soll HD4478 Kompatible Displays im 4-Bit Modus ansteuern.
 * Sie wurde von mir entwickelt um das Pollin C0802-04 LCD anzusteuern.
 * Dieses hat eine Besonderheit bei der initalisierung.
 * Deshalb kann dieses LCD per #define in der lcd_config.h ausgewählt werden.
 * Ohne diese Optionen sollten aber auch alle anderen Displays funktionieren.
 * In der lcd_config.h können alle Einstellungen vorgenommen werden. Alle Leitungen
 * können separat angegeben werden. Lediglich müssen alle Datenleitungen auf demselben Port, und alle
 * Control-Leitungen auf demselben Port sein.
 * Wo möglich wird das Busy-Flag ausgewertet, statt feste Delays zu verwenden.
 * Die "Enable Pulse Width (high level)" (vgl. Datenblatt hitachi HD44780)) kann per STROBE_DELAY_US verändert werden.
 * Standardwert ist 2µS. Laut Datenblatt sind hier 450nS (bei 1µS cycle time) angegeben.
 */

#ifndef LCD_H_
#define LCD_H_

void lcd_init();
void lcd_print(char* string);
void lcd_clear();
void lcd_home(); //Cursor home
void lcd_goto(int line,int pos); //1. Zeile = 1, 2. Zeile=2
void lcd_set(int on,int cursor,int blink);


#endif /* LCD_H_ */
