/*
 * RTC_Atmega1284.c
 *
 * Created: 10/13/2016 6:51:02 PM
 * Author : Chandan
 */ 
#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "UART_1.h"
#include <math.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>

#define SCL 100000L
#define TRUE 1
#define FALSE 0

unsigned char hex_to_decimal(unsigned char hex);

void i2c_init()
{
	TWBR = (((F_CPU/SCL)-16)/8);
	TWSR = 0;
	TWCR |= (1<<TWEN);
}

uint8_t i2c_start()
{
	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTA);
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xF8) == 0x08)
	{
		return TRUE;
	}
	else
	{
		UART_1_puts("\r\n Sta ");
		UART_1_putc(TWSR);
		return FALSE;
	}
}

uint8_t i2c_rep_start()
{
	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTA);
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xF8) == 0x10){
		return TRUE;
	}
	else
	{
		UART_1_puts("\r\n R-Sta ");
		UART_1_putc(TWSR);
		return FALSE;
	}
}

void i2c_stop()
{
	TWCR=(1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
	while(TWCR & (1<<TWSTO));
}

uint8_t i2c_slave_add_write(uint8_t address)
{
	TWDR = address;
	TWCR=(1<<TWEN)|(1<<TWINT);
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xF8) == 0x18)
	{
		return TRUE;
	}
	else
	{
		UART_1_puts("\r\n St0 ");
		UART_1_putc(TWSR);
		return FALSE;
	}
}

uint8_t i2c_data_byte_write(uint8_t data)
{
	TWDR = data;
	TWCR=(1<<TWEN)|(1<<TWINT);
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xF8) == 0x28)
	{
		return TRUE;
	}
	else
	{
		UART_1_puts("\r\n St1 ");
		UART_1_putc(TWSR);
		return FALSE;
	}	
}

uint8_t i2c_write_add_read(uint8_t address)
{
	TWDR = address;
	TWCR=(1<<TWEN)|(1<<TWINT);
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xF8) == 0x40)
	{
		return TRUE;
	}
	else
	{
		UART_1_puts("\r\n St2 ");
		UART_1_putc(TWSR);
		return FALSE;
	}
}

uint8_t i2c_write(uint8_t data)
{
	TWDR=data;
	TWCR=(1<<TWEN)|(1<<TWINT);
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xF8) == 0x18 || (TWSR & 0xF8) == 0x28 || (TWSR & 0xF8) == 0x40)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
	
}

uint8_t i2c_read_data_ACK()
{
	
	TWCR|=(1<<TWEA);
	TWCR|=(1<<TWINT)|(1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xF8) == 0x50)
	{
		return TWDR;
	}
	else
	{
		UART_1_puts("\r\n St3 ");
		UART_1_putc(TWDR);
		return FALSE;
	}	
}

uint8_t i2c_read_data_NACK()
{
	
	TWCR&=~(1<<TWEA);
	TWCR|=(1<<TWINT)|(1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xF8) == 0x58)
	{
		return TWDR;
	}
	else
	{
		UART_1_puts("\r\n St4 ");
		UART_1_putc(TWDR);
		return FALSE;
	}
}

void rtc_write(unsigned char reg, unsigned char data)
{
	if (i2c_start())
	{
		
		if (i2c_slave_add_write(0xD0))
		{
			if (i2c_data_byte_write(reg))
			{
				if (i2c_data_byte_write(data))
				{
		
					i2c_stop();				
				} 
				else
				{
					UART_1_puts("\r\n Data byte write Failed");
				}

			} 
			else
			{
				UART_1_puts("\r\n Reg Add Filed");
			}
					
		} 
		else
		{
			UART_1_puts("\r\n SLA+W failed");
		}
	}
	else
	{
		UART_1_puts("\r\n Start Failed");
	}
	
	_delay_ms(10);
}

unsigned char rtc_read(char dev_addr,char dev_loc)
{
	char ch1, ch2, ch3, ch4, ch5, ch6, ch7; 
	if (i2c_start())
	{
		if (i2c_slave_add_write(0xD0))
		{
			if (i2c_data_byte_write(0x00))
			{
				if (i2c_rep_start())
				{
					
					if (i2c_write_add_read(0xD1))
					{
						ch1 = i2c_read_data_ACK();
						UART_1_putc(hex_to_decimal((ch1 & 0xF0)>>4));
						UART_1_putc(hex_to_decimal(ch1 & 0x0F));
						UART_1_putc(':');
						
						ch2 = i2c_read_data_ACK();
						UART_1_putc(hex_to_decimal((ch2 & 0xF0)>>4));
						UART_1_putc(hex_to_decimal(ch2 & 0x0F));
						UART_1_putc(':');
						
						ch3 = i2c_read_data_ACK();
						UART_1_putc(hex_to_decimal((ch3 & 0x10)>>4));
						UART_1_putc(hex_to_decimal(ch3 & 0x0F));
						UART_1_putc('-');
						
						ch4 = i2c_read_data_ACK();
						UART_1_putc(hex_to_decimal(ch4 & 0x0F));
						UART_1_putc('-');
						
						ch5 = i2c_read_data_ACK();
						UART_1_putc(hex_to_decimal((ch5 & 0xF0)>>4));
						UART_1_putc(hex_to_decimal(ch5 & 0x0F));
						UART_1_putc('/');
						
						ch6 = i2c_read_data_ACK();
						UART_1_putc(hex_to_decimal((ch6 & 0xF0)>>4));
						UART_1_putc(hex_to_decimal(ch6 & 0x0F));
						UART_1_putc('/');
						
						ch7 = i2c_read_data_NACK();
						i2c_stop();
						UART_1_putc(hex_to_decimal((ch7 & 0xF0)>>4));
						UART_1_putc(hex_to_decimal(ch7 & 0x0F));
						
						UART_1_puts("\r\n");
					}
					else
					{
						UART_1_puts("\r\n SLA+R failed");
					}
				}
			} 
		}
	} 
	return 0;
}

void display_time_date()
{	
	char ch; 
	
	UART_1_puts("\r\n Time : ");
	
	ch = rtc_read(0xD0, 0x02);
	
}

int main(void)
{
	_delay_ms(5000);
	
	DDRD = 0x0A;
	
	i2c_init();
	UART_1_init();
	UART_1_puts("\r\n Starting...");
	
	_delay_ms(5000);
	rtc_write(0x00,0x00);
	_delay_ms(1000);
	rtc_write(0x01,0x06);
	_delay_ms(1000);
	rtc_write(0x02,0x03);
	_delay_ms(1000);
	rtc_write(0x04,0x25);
	_delay_ms(1000);
	rtc_write(0x05,0x10);
	_delay_ms(1000);
	rtc_write(0x06,0x16);
	
	_delay_ms(5000);
    while (1) 
    {
		display_time_date();
		
		_delay_ms(2000);
    }
}

unsigned char hex_to_decimal(unsigned char hex)
{
	unsigned char value;
	switch(hex)
	{
		case 0:
				value = '0';
		break;
		
		case 1:
			value = '1';
		break;
		
		case 2:
			value = '2';
		break;
		
		case 3:
			value = '3';
		break;
		
		case 4:
			value = '4';
		break;
		
		case 5:
			value = '5';
		break;
		
		case 6:
			value = '6';
		break;
		
		case 7:
			value = '7';
		break;
		
		case 8:
			value = '8';
		break;
		
		case 9:
			value = '9';
		break;
		
		default:
			value = ' ';
		break;
	}
	return value;
}
