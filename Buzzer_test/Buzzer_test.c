/*
 * Buzzer_test.c
 *
 * Created: 23-11-2022 18:18:19
 * Author : SHIVANI
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1000000UL
#include <util/delay.h>


//Function to initialize Buzzer
void buzzer_pin_config (void)
{
	DDRC = DDRC | 0x08; //Setting PORTC 3 as output
	PORTC = PORTC & 0xF7; //Setting PORTC 3 logic low to turnoff buzzer
}
void port_init (void)
{
	buzzer_pin_config();
}
void buzzer_on (void)
{
	unsigned char port_restore = 0;
	port_restore = PINC;
	port_restore = port_restore | 0x08;
	PORTC = port_restore;
}
void buzzer_off (void)
{
	unsigned char port_restore = 0;
	port_restore = PINC;
	port_restore = port_restore & 0xF7;
	PORTC = port_restore;
}
void init_devices (void)
{
	cli(); //Clears the global interrupts
	port_init();
	sei(); //Enables the global interrupts
}
//Main Function
int main(void)
{
	init_devices();
	while(1)
	{
		buzzer_on();
		_delay_ms(1000); //delay
		buzzer_off();
		_delay_ms(1000); //delay
	}
}