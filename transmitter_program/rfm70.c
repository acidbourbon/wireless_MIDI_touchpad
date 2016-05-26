/*
	Diese Library bassiert auf der Arduino RFM70 Library, ich habe sie lediglich so
	angepasst, dass man sie auf einem AVR und ohne zus�tzliche Libs von Arduino
	verwenden kann.

	Es sind nur die wichtigstens Funktionen implementiert!
	Bei Fragen oder Anregungen:
	Mein Blog: http:://projects.web4clans.com
	Meine Email: daniel.weber@web4clans.com

*/

#include "rfm70.h"
///////////////////////////////////////////////////////////////////////////////
// Register initialization values and command macros //
///////////////////////////////////////////////////////////////////////////////
 
//************ Address definition commands
const uint8_t PROGMEM RFM70_cmd_adrRX0[] = { (0x20|0x0A), 0x34,0x43,0x10,0x10,0x01};
const uint8_t PROGMEM RFM70_cmd_adrTX[]  = { (0x20|0x10), 0x34,0x43,0x10,0x10,0x01};
const uint8_t PROGMEM RFM70_cmd_adrRX1[] = { (0x20|0x0B), 0x35,0x43,0x10,0x10,0x02};
 
//************ Bank0 register initialization commands
 const uint8_t PROGMEM RFM70_bank0Init[][2] = {
  // address data
  { (0x20|0x00), 0x0F }, //Disable CRC ,CRC=1byte, POWER UP, TX
  { (0x20|0x01), 0x3F }, //Enable auto acknowledgement data pipe0-5
  { (0x20|0x02), 0x3F }, //Enable RX Addresses pipe0-5
  { (0x20|0x03), 0x03 }, //RX/TX address field width 5byte
  { (0x20|0x04), 0x08 }, //x = 250 ms = 4000ms, y = 15 tries
  { (0x20|0x05), 0x17 }, //channel = 0x17
  { (0x20|0x06), 0x3F }, //air data rate-2M,out power 5dbm,setup LNA gain high (0dBM)
  { (0x20|0x07), 0x07 }, //
  { (0x20|0x08), 0x00 }, //
  { (0x20|0x09), 0x00 }, //
  { (0x20|0x0C), 0xc3 }, //LSB Addr pipe 2
  { (0x20|0x0D), 0xc4 }, //LSB Addr pipe 3
  { (0x20|0x0E), 0xc5 }, //LSB Addr pipe 4
  { (0x20|0x0F), 0xc6 }, //LSB Addr pipe 5
  { (0x20|0x11), 0x20 }, //Payload len pipe0
  { (0x20|0x12), 0x20 }, //Payload len pipe0
  { (0x20|0x13), 0x20 }, //Payload len pipe0
  { (0x20|0x14), 0x20 }, //Payload len pipe0
  { (0x20|0x15), 0x20 }, //Payload len pipe0
  { (0x20|0x16), 0x20 }, //Payload len pipe0
  { (0x20|0x17), 0x20 }, //Payload len pipe0
  { (0x20|0x1C), 0x3F }, //Enable dynamic payload legth data pipe0-5
  { (0x20|0x1D), 0x07 } //Enables Dynamic Payload Length,Enables Payload with ACK
};
 
//************ Bank1 register initialization commands
 const uint8_t PROGMEM RFM70_bank1Init[][5] = {
  // address data
  { (0x20|0x00), 0x40, 0x4B, 0x01, 0xE2 },
  { (0x20|0x01), 0xC0, 0x4B, 0x00, 0x00 },
  { (0x20|0x02), 0xD0, 0xFC, 0x8C, 0x02 },
  { (0x20|0x03), 0x99, 0x00, 0x39, 0x41 },
  { (0x20|0x04), 0xb9, 0x9E, 0x86, 0x0B }, // b9? f9?
  { (0x20|0x05), 0x24, 0x06, 0x7F, 0xA6 },
  { (0x20|0x06), 0x00, 0x00, 0x00, 0x00 },
  { (0x20|0x07), 0x00, 0x00, 0x00, 0x00 },
  { (0x20|0x08), 0x00, 0x00, 0x00, 0x00 },
  { (0x20|0x09), 0x00, 0x00, 0x00, 0x00 },
  { (0x20|0x0a), 0x00, 0x00, 0x00, 0x00 },
  { (0x20|0x0b), 0x00, 0x00, 0x00, 0x00 },
  { (0x20|0x0C), 0x00, 0x12, 0x73, 0x00 },
  { (0x20|0x0D), 0x36, 0xb4, 0x80, 0x00 }
};
 
//************ Bank1 register 14 initialization commands
 const uint8_t PROGMEM RFM70_bank1R0EInit[] = {
  // address Data...
  (0x20|0x0E), 0x41,0x20,0x08,0x04,0x81,0x20,0xCF,0xF7,0xFE,0xFF,0xFF
};


//************ other commands: { <command>, <data>, ... }
const uint8_t PROGMEM RFM70_cmd_switch_cfg[] = { 0x50, 0x53 }; // switch Register Bank
const uint8_t PROGMEM RFM70_cmd_flush_rx[] = { 0xe2, 0x00 }; // flush RX FIFO
const uint8_t PROGMEM RFM70_cmd_flush_tx[] = { 0xe1, 0x00 }; // flush TX FIFO
const uint8_t PROGMEM RFM70_cmd_activate[] = { 0x50, 0x73 }; // Activation command
const uint8_t PROGMEM RFM70_cmd_tog1[]={ (0x20|0x04), 0xd9 | 0x06, 0x9e, 0x86, 0x0b }; //assosciated with set1[4]!
const uint8_t PROGMEM RFM70_cmd_tog2[]={ (0x20|0x04), 0xd9 & ~0x06, 0x9e, 0x86, 0x0b}; //assosciated with set1[4]!



void initSPI(uint8_t clk_div)
{
	// set the pin direction to output
	DDR_SPI |= (1<<SCK)|(1<<MOSI)|(1<<CSN);

	// chip select to high
	PORT_SPI |= (1<<CSN);
	// other to low
	PORT_SPI &=~((1<<MOSI)|(1<<SCK));
	
	// init SPI
	SPCR = (1<<SPE)|(1<<MSTR);

	// det clock divider
  	spiSetClockDivider(clk_div);


}

void initHardware(uint8_t irq)
{
	// set the CE ddr to output
	DDR_SPI |= (1<<CE);
	// and set it to low
	PORT_SPI &=~(1<<CE);

	if (irq != -1) 
    	DDRD &=~ (1<<IRQ);


}

void initRegisters(void)
{
  	// init bank 0 registers
  	selectBank(0);

	for (int i = 0; i < 20; i++)
    	writeRegVal(pgm_read_byte(&RFM70_bank0Init[i][0]), pgm_read_byte(&RFM70_bank0Init[i][1]));


  	// init address registers in bank 0
  	writeRegPgmBuf((uint8_t *)RFM70_cmd_adrRX0, sizeof(RFM70_cmd_adrRX0));
  	writeRegPgmBuf((uint8_t *)RFM70_cmd_adrRX1, sizeof(RFM70_cmd_adrRX1));
  	writeRegPgmBuf((uint8_t *)RFM70_cmd_adrTX, sizeof(RFM70_cmd_adrTX));

	// activate Feature register
  	if(!readRegVal(RFM70_REG_FEATURE))
    	writeRegPgmBuf((uint8_t *)RFM70_cmd_activate, sizeof(RFM70_cmd_activate));

	// now set Registers 1D and 1C
  	writeRegVal(pgm_read_byte(&RFM70_bank0Init[22][0]), pgm_read_byte(&RFM70_bank0Init[22][1]));
  	writeRegVal(pgm_read_byte(&RFM70_bank0Init[21][0]), pgm_read_byte(&RFM70_bank0Init[21][1]));

  	// init bank 1 registers
  	selectBank(1);

  	for (int i=0; i < 14; i++)
    	writeRegPgmBuf((uint8_t *)RFM70_bank1Init[i], sizeof(RFM70_bank1Init[i]));

	// set ramp curve
  	writeRegPgmBuf((uint8_t *)RFM70_bank1R0EInit, sizeof(RFM70_bank1R0EInit));


  	// do we have to toggle some bits here like in the example code?
  	writeRegPgmBuf((uint8_t *)RFM70_cmd_tog1, sizeof(RFM70_cmd_tog1));
  	writeRegPgmBuf((uint8_t *)RFM70_cmd_tog2, sizeof(RFM70_cmd_tog2));

	_delay_ms(RFM70_END_INIT_WAIT_MS);

  	//Check the ChipID
  	if (readRegVal(0x08) != 0x63) 
    	debug(RFM70_DEBUG_WRONG_CHIP_ID);
	else
	//uart_puts("load rfm70 register successful\n");

  	selectBank(0);
	setModeRX();
}

void Begin(void) 
{
  setBegin(-1, RFM77_DEFAULT_SPI_CLOCK_DIV);
}


void setBegin(uint8_t irq, uint8_t clk_div) 
{
  	initHardware(irq);
  	initSPI(clk_div);
  	_delay_ms(RFM70_BEGIN_INIT_WAIT_MS);
  	initRegisters();
 }



uint8_t transmitSPI(uint8_t val) 
{
  SPDR = val;
  while (!(SPSR & _BV(SPIF)))
    ;
  return SPDR;
}

void selectBank(uint8_t bank) 
{
  	uint8_t tmp = readRegVal(0x07) & 0x80;
  	if(bank) 
	{
    	if(!tmp)
      	writeRegPgmBuf((uint8_t *)RFM70_cmd_switch_cfg, sizeof(RFM70_cmd_switch_cfg));
  	} 
  	else 
	{
    	if(tmp)
      		writeRegPgmBuf((uint8_t *)RFM70_cmd_switch_cfg, sizeof(RFM70_cmd_switch_cfg));
  	}
}

void setMode(uint8_t mode) 
{
  if (mode == 1)
    setModeRX();
  else
    setModeTX();
}


void setModeRX(void)
{
  uint8_t val;

  writeRegPgmBuf((uint8_t *)RFM70_cmd_flush_rx, sizeof(RFM70_cmd_flush_rx)); // Flush RX FIFO
  val = readRegVal(RFM70_REG_STATUS); // Read Status
  writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_STATUS, val); // Reset IRQ bits
  PORT_SPI &=~ (1<<CE); // RFM chip disable
  // set PRIM_RX bit to 1
  val=readRegVal(RFM70_REG_CONFIG);
  val |= RFM70_PIN_PRIM_RX;
  writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_CONFIG, val);
  PORT_SPI |= (1<<CE); // RFM chip enable
}


void setModeTX(void)
{
  	uint8_t val;

  	writeRegPgmBuf((uint8_t *)RFM70_cmd_flush_tx, sizeof(RFM70_cmd_flush_tx)); // Flush TX FIFO
  	PORT_SPI &=~ (1<<CE); // disable rfm70
  	// set PRIM_RX bit to 0
  	val=readRegVal(RFM70_REG_CONFIG);
  	val &= ~RFM70_PIN_PRIM_RX;
  	writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_CONFIG, val);
  	PORT_SPI |= (1<<CE); // RFM chip enable
}

uint8_t getMode(void) 
{
  return readRegVal(RFM70_REG_CONFIG) & RFM70_PIN_PRIM_RX;
}


void setChannel(uint8_t cnum)
{
  writeRegVal( RFM70_CMD_WRITE_REG | RFM70_REG_RF_CH, cnum);
}

uint8_t getChannel(void) 
{
  return readRegVal(RFM70_REG_RF_CH);
}


uint8_t configRxPipe(uint8_t pipe_nr, uint8_t * adr, uint8_t plLen, uint8_t en_aa) 
{

  	uint8_t tmp;
  	uint8_t nr = pipe_nr -1;
  
  	if(plLen > 32 || nr > 5 || en_aa > 1)
    	return 0;

  	// write address
  	if(nr<2)      // full length for rx pipe 0 an 1
    	writeRegCmdBuf(RFM70_CMD_WRITE_REG | (RFM70_REG_RX_ADDR_P0 + nr), adr, sizeof(adr));
  	else // only LSB for pipes 2..5
    	writeRegVal(RFM70_CMD_WRITE_REG | (RFM70_REG_RX_ADDR_P0 + nr), adr[0]); //ODO:check this
  
  	// static
  	if (plLen) {
    	// set payload len
    	writeRegVal(RFM70_CMD_WRITE_REG | (RFM70_REG_RX_PW_P0 + nr), plLen);
        	// set EN_AA bit
        tmp = readRegVal(RFM70_REG_EN_AA);
        	if (en_aa)
            	tmp |= 1 << nr;
        	else
                tmp &= ~(1 << nr);
        writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_EN_AA, tmp);
        // clear DPL bit
        tmp = readRegVal(RFM70_REG_DYNPD);
        tmp &= ~(1 << nr);
        writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_DYNPD, tmp);        
        // set Enable pipe bit
        enableRxPipe(nr);
	}
  	// dynamic
  	else 
	{
    		// set payload len to default
    		writeRegVal(RFM70_CMD_WRITE_REG | (RFM70_REG_RX_PW_P0 + nr), 0x20);
        	// set EN_AA bit
        	tmp = readRegVal(RFM70_REG_EN_AA);
        	tmp |= 1 << nr;
        	writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_EN_AA, tmp);
        	// set DPL bit
        	tmp = readRegVal(RFM70_REG_DYNPD);
        	tmp |= 1 << nr;
        	writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_DYNPD, tmp);
        	// set Enable pipe bit
        	enableRxPipe(nr);
  	}
  return 1;
}

void enableRxPipe(uint8_t pipe_nr) 
{
  	uint8_t nr = pipe_nr - 1;
  	if (nr > 5) return;
  	uint8_t tmp;
  	// set Enable pipe bit
  	tmp = readRegVal(RFM70_REG_EN_RXADDR);
  	tmp |= 1 << nr;
  	writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_EN_RXADDR, tmp);
}

void disableRxPipe(uint8_t pipe_nr) 
{
  uint8_t nr = pipe_nr - 1;
  if (nr > 5) return;
  uint8_t tmp;
  // set Enable pipe bit
  tmp = readRegVal(RFM70_REG_EN_RXADDR);
  tmp &= ~(1 << nr);
  writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_EN_RXADDR, tmp);

}

void configTxPipe(uint8_t * adr, uint8_t pltype) 
{
  // write TX address
  writeRegCmdBuf(RFM70_CMD_WRITE_REG | RFM70_REG_TX_ADDR, adr, sizeof(adr));
  // write RX0 address
  writeRegCmdBuf(RFM70_CMD_WRITE_REG | RFM70_REG_RX_ADDR_P0, adr, sizeof(adr));
  // set static or dynamic payload
  uint8_t tmp;
  tmp = readRegVal(RFM70_REG_DYNPD);
  if(pltype == TX_DPL) // dynamic
        tmp |= 1;
  else  
    tmp &= ~(1 << 0);
  writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_DYNPD, tmp);
}

uint8_t sendPayload(uint8_t * payload, uint8_t len, uint8_t toAck)	// choose 0=nAck, 1=AckRequest
{
	
  	// check TX_FIFO
  	uint8_t status;
  	status = readRegVal(RFM70_REG_FIFO_STATUS); 
  	if (status & RFM70_FIFO_STATUS_TX_FULL) 
	{
    	debug(RFM70_DEBUG_FIFO_FULL);
    	return 0;
  	}

  	// send payload
	PORT_SPI &=~ (1<<CSN);
  	_delay_ms(RFM70_CS_DELAY);
  	if(toAck == -1)
    	transmitSPI(RFM70_CMD_W_ACK_PAYLOAD);
  	else if (toAck == 0)
    	transmitSPI(RFM70_CMD_W_TX_PAYLOAD_NOACK);
  	else
    	transmitSPI(RFM70_CMD_WR_TX_PLOAD);
  	while(len--) 
	{
    	transmitSPI(*(payload++));
		////uart_putc(*payload);
		//uart_puts(", ");
  	}
	PORT_SPI |= (1<<CSN);
  	_delay_ms(RFM70_CS_DELAY);
	//uart_puts("\n");
	//PORTA ^= (1<<PA4);
  	return 1;
}

uint8_t receivePayload(uint8_t *payload)
{
  	uint8_t len;
  	// check RX_FIFO
  	uint8_t status;
  	status = readRegVal(RFM70_REG_STATUS);
  	if (status & RFM70_IRQ_STATUS_RX_DR) { // RX_DR
		//PORTA ^= (1<<PA4);
    	//while(1) {
      	uint8_t fifo_sta;
      	len = readRegVal(RFM70_CMD_RX_PL_WID); // Payload width
      	readRegBuf(RFM70_CMD_RD_RX_PLOAD, payload, len);
      	fifo_sta = readRegVal(RFM70_REG_FIFO_STATUS);
      	//if (fifo_sta & RFM70_FIFO_STATUS_RX_EMPTY) break; // read until RX_FIFO empty
    	//}
		
        if (fifo_sta & RFM70_FIFO_STATUS_RX_EMPTY) {
        	status|= 0x40 & 0xCF; // clear status bit rx_dr
    		writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_STATUS, status); 
        }
    	return len;
  	}
  	else
	{
		
    	return 0;
	}
}


void flushTxFIFO() 
{
  writeRegPgmBuf((uint8_t *)RFM70_cmd_flush_tx, sizeof(RFM70_cmd_flush_tx)); // Flush TX FIFO
}

void flushRxFIFO() 
{
  writeRegPgmBuf((uint8_t *)RFM70_cmd_flush_rx, sizeof(RFM70_cmd_flush_rx)); // Flush RX FIFO
}





uint8_t readRegVal(uint8_t cmd) 
{
  	uint8_t res;
	PORT_SPI &=~ (1<<CSN);
  	_delay_ms(RFM70_CS_DELAY);

  	transmitSPI(cmd);

  	res=transmitSPI(0);
  	PORT_SPI |= (1<<CSN);
  	_delay_ms(RFM70_CS_DELAY);
  	return res;
}

uint8_t writeRegVal(uint8_t cmd, uint8_t val) 
{
  PORT_SPI &=~ (1<<CSN);
  _delay_ms(RFM70_CS_DELAY);
  transmitSPI(cmd);
  transmitSPI(val);
  PORT_SPI |= (1<<CSN);
  _delay_ms(RFM70_CS_DELAY);
  return 1;
}

void readRegBuf(uint8_t reg, uint8_t * buf, uint8_t len) 
{
  	uint8_t status, byte_ctr;
  	PORT_SPI &=~ (1<<CSN);
  	_delay_ms(RFM70_CS_DELAY);
  	status = transmitSPI(reg); // Select register to write, and read status UINT8
  	for(byte_ctr = 0; byte_ctr < len; byte_ctr++)
   		buf[byte_ctr] = transmitSPI(0); // Perform SPI_RW to read UINT8 from RFM70
  	PORT_SPI |= (1<<CSN);
  	_delay_ms(RFM70_CS_DELAY);
}



uint8_t writeRegPgmBuf(uint8_t * cmdbuf, uint8_t len) 
{
  PORT_SPI &=~ (1<<CSN);
  _delay_ms(RFM70_CS_DELAY);
  while(len--) {
    transmitSPI(pgm_read_byte(cmdbuf++));
  }
  PORT_SPI |= (1<<CSN);
  _delay_ms(RFM70_CS_DELAY);
  return 1;
}

uint8_t writeRegCmdBuf(uint8_t cmd, uint8_t * buf, uint8_t len) 
{
  	PORT_SPI &=~ (1<<CSN);
  	_delay_ms(RFM70_CS_DELAY);
  	transmitSPI(cmd);
  	while(len--) 
	{
    	transmitSPI(*(buf++));
  	}
  	PORT_SPI |= (1<<CSN);
  	_delay_ms(RFM70_CS_DELAY);
	return 1;
}

void configRfPower(uint8_t pwr) 
{
  if (pwr > 3) return;
  uint8_t tmp = readRegVal(RFM70_REG_RF_SETUP);
  tmp &= 0xF9;
  tmp |= pwr << 1;
  writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_RF_SETUP, tmp);
}




void debug(uint8_t token)
{
	switch(token)
	{
		case 0x01:
		//uart_puts("rfm error: the reseive chip id is invalid!\n");
		break;

		case 0x02:
		//uart_puts("rfm error: the fifo is full!\n");
		break;
	}
}

void spiSetClockDivider(uint8_t rate) 
{
  SPCR = (SPCR & ~SPI_CLOCK_MASK) | (rate & SPI_CLOCK_MASK);
  SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((rate >> 2) & SPI_2XCLOCK_MASK);
}


