/*
 * Sensor_testing.c
 *
 * Created: 11/25/2022 2:38:26 PM
 * Author : HP
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
int a=1;

unsigned char ADC_Conversion(unsigned char);
void stop(void);
unsigned char ADC_Value;
unsigned char flag = 0;
unsigned char ir_range=0;
unsigned int val=800;
unsigned int ShaftCountRight=0;
unsigned int ShaftCountLeft=0;
int right_turn=0;

void turn_on_all_proxy_sensors (void)
// turn on Sharp 2, 3, 4, red LED of the white line sensors, Sharp 1, 5 and IR proximity sensor
{
	PORTH = PORTH & 0xF3; //set PORTH 3 and PORTH 1 pins to 0
	PORTG = PORTG & 0xFB; //set PORTG 2 pin to 0
}
void turn_off_all_proxy_sensors (void)
// turn off Sharp 2, 3, 4, red LED of the white line sensors Sharp 1, 5 and IR proximity sensor
{
	PORTH = PORTH | 0x0C; //set PORTH 3 and PORTH 1 pins to 1
	PORTG = PORTG | 0x04; //set PORTG 2 pin to 1
}

void adc_init()
{
	ADCSRA = 0x00;
	ADCSRB = 0x00;		//MUX5 = 0
	ADMUX = 0x20;		//V ref=5V external --- ADLAR=1 --- MUX4:0 = 0000
	ACSR = 0x80;
	ADCSRA = 0x86;		//ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0
}

//Function For ADC Conversion
unsigned char ADC_Conversion(unsigned char Ch)
{
	unsigned char a;
	if(Ch>7)
	{
		ADCSRB = 0x08;
	}
	Ch = Ch & 0x07;
	ADMUX= 0x20| Ch;
	ADCSRA = ADCSRA | 0x40;		//Set start conversion bit
	while((ADCSRA&0x10)==0);	//Wait for conversion to complete
	a=ADCH;
	ADCSRA = ADCSRA|0x10; //clear ADIF (ADC Interrupt Flag) by writing 1 to it
	ADCSRB = 0x00;
	return a;
}

void adc_pin_config (void)
{
	DDRF = 0x00; //set PORTF direction as input
	PORTF = 0x00; //set PORTF pins floating
	DDRK = 0x00; //set PORTK direction as input
	PORTK = 0x00; //set PORTK pins floating
}

void motion_pin_config (void)
{
	DDRA = DDRA | 0x0F; //set direction of the PORTA 3 to PORTA 0 pins as output
	PORTA = PORTA & 0xF0; // set initial value of the PORTA 3 to PORTA 0 pins to logic 0
	DDRL = DDRL | 0x18; //Setting PL3 and PL4 pins as output for PWM generation
	PORTL = PORTL | 0x18; //PL3 and PL4 pins are for velocity control using PWM
}

void buzzer_pin_config (void)
{
	DDRC = DDRC | 0x08; //Setting PORTC 3 as output
	PORTC = PORTC & 0xF7; //Setting PORTC 3 logic low to turnoff buzzer
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

void left_encoder_pin_config (void)
{
	DDRE = DDRE & 0xEF; //Set the direction of the PORTE 4 pin as input
	PORTE = PORTE | 0x10; //Enable internal pull-up for PORTE 4 pin
}
void right_encoder_pin_config (void)
{
	DDRE = DDRE & 0xDF; //Set the direction of the PORTE 5 pin as input
	PORTE = PORTE | 0x20; //Enable internal pull-up for PORTE 5 pin
}

void left_position_encoder_interrupt_init (void) //Interrupt 4 enable
{
	cli(); //Clears the global interrupt
	EICRB = EICRB | 0x02; // INT4 is set to trigger with falling edge
	EIMSK = EIMSK | 0x10; // Enable Interrupt INT4 for left position encoder
	sei(); // Enables the global interrupt
}
void right_position_encoder_interrupt_init (void) //Interrupt 5 enable
{
	cli(); //Clears the global interrupt
	EICRB = EICRB | 0x08; // INT5 is set to trigger with falling edge
	EIMSK = EIMSK | 0x20; // Enable Interrupt INT5 for right position encoder
	sei(); // Enables the global interrupt
}

void timer1_init(void){
	TCCR1B|= (1<<WGM12) |(1<<CS12) |(1<<CS10);
	OCR1A=312;
	TIMSK1|=(1<<OCIE1A);
}


void port_init()
{
	cli();
	adc_pin_config();
	adc_init();
	timer1_init();
	motion_pin_config();
	buzzer_pin_config();
	left_encoder_pin_config();
	right_encoder_pin_config();
	left_position_encoder_interrupt_init();
	right_position_encoder_interrupt_init();
	sei();
}

unsigned int Sharp_GP2D12_estimation(unsigned char adc_reading)
{
	float distance;
	unsigned int distanceInt;
	distance = (int)(10.00*(2799.6*(1.00/(pow(adc_reading,1.1546)))));
	distanceInt = (int)distance;
	if(distanceInt>800)
	{
		distanceInt=800;
	}
	return distanceInt;
}

void motion_set (unsigned char Direction)
{
	unsigned char PortARestore = 0;
	Direction &= 0x0F; // removing upper nibble as it is not needed
	PortARestore = PORTA; // reading the PORTA's original status
	PortARestore &= 0xF0; // setting lower direction nibble to 0
	PortARestore |= Direction; // adding lower nibble for direction command and
	// restoring the PORTA status
	PORTA = PortARestore; // setting the command to the port
}



ISR(INT5_vect)
{
	if(right_turn){
	ShaftCountRight++;
	if(ShaftCountRight==22){
		right_turn=0;
		}	
	}
}
//SR for left position encoder
ISR(INT4_vect)
{
	if(right_turn){
		ShaftCountLeft++;
	}
}

void forward (void) //both wheels forward
{
	motion_set(0x06);
}
void back (void) //both wheels backward
{
	motion_set(0x09);
}
void left (void) //Left wheel backward, Right wheel forward
{
	motion_set(0x05);
}
void right (void) //Left wheel forward, Right wheel backward
{
	motion_set(0x0A);
}
void soft_left (void) //Left wheel stationary, Right wheel forward
{
	motion_set(0x04);
}
void soft_right (void) //Left wheel forward, Right wheel is stationary
{
	motion_set(0x02);
}
void soft_left_2 (void) //Left wheel backward, right wheel stationary
{
	motion_set(0x01);
}
void soft_right_2 (void) //Left wheel stationary, Right wheel backward
{
	motion_set(0x08);
}
void stop (void) //hard stop if PORTL 3 and PORTL 4 pins are at logic 1
{
	motion_set(0x00);
}

void right_degrees(void)
{
	right_turn=1;
	ShaftCountRight = 0;
	ShaftCountLeft = 0;
	right();
	while(right_turn){
		
	}
	
	
}

ISR(TIMER1_COMPA_vect){
	
}

int main(void)
{
    /* Replace with your application code */
	port_init();
    while (1) 
    {
		ir_range= ADC_Conversion(11);
		val= Sharp_GP2D12_estimation(ir_range);
		if(val<200){
			ShaftCountRight = 0;
			ShaftCountLeft = 0;
			buzzer_on();
			right_degrees();
			buzzer_off();
			stop();
		}
		switch(a){
			case 1:
				forward();
				break;
			
			default:
				stop();
				break;
		}
		
		
		
		
    }
}

