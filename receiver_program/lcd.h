/*
 * lcd.h
 *
 *  Created on: 31.07.2011
 *      Author: cyblord
 *
 * Diese lib soll HD4478 Kompatible Displays im 4-Bit Modus ansteuern.
 * Sie wurde von mir entwickelt um das Pollin C0802-04 LCD anzusteuern.
 * Dieses hat eine Besonderheit bei der initalisierung.
 * Deshalb kann dieses LCD per #define in der lcd_config.h ausgew�hlt werden.
 * Ohne diese Optionen sollten aber auch alle anderen Displays funktionieren.
 * In der lcd_config.h k�nnen alle Einstellungen vorgenommen werden. Alle Leitungen
 * k�nnen separat angegeben werden. Lediglich m�ssen alle Datenleitungen auf demselben Port, und alle
 * Control-Leitungen auf demselben Port sein.
 * Wo m�glich wird das Busy-Flag ausgewertet, statt feste Delays zu verwenden.
 * Die "Enable Pulse Width (high level)" (vgl. Datenblatt hitachi HD44780)) kann per STROBE_DELAY_US ver�ndert werden.
 * Standardwert ist 2�S. Laut Datenblatt sind hier 450nS (bei 1�S cycle time) angegeben.
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
