#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
int a=1;
int count=0;
int countr=0;
int countl=0;
int thresh=183;

void motion_pin_config (void)
{
	DDRA = DDRA | 0x0F; //set direction of the PORTA 3 to PORTA 0 pins as output
	PORTA = PORTA & 0xF0; // set initial value of the PORTA 3 to PORTA 0 pins to logic 0
	DDRL = DDRL | 0x18; //Setting PL3 and PL4 pins as output for PWM generation
	PORTL = PORTL | 0x18; //PL3 and PL4 pins are for velocity control using PWM
}

void timer1_init(void){
	TCCR1B|= (1<<WGM12) |(1<<CS12) |(1<<CS10);
	OCR1A=15625;	
	TIMSK1|=(1<<OCIE1A);
}

ISR(TIMER1_COMPA_vect){
	count++;
	//if(count==5){
		//count=0;
		//if(a==1){
			//a=2;
	//	}
		//else{
		//	a=1;
		//}
//	}
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

void init_devices()
{
	cli(); //Clears the global interrupt
	motion_pin_config();
	timer1_init();
	left_position_encoder_interrupt_init();
	right_position_encoder_interrupt_init();
	sei(); // Enables the global interrupt
}
//SR for right position encoder
ISR(INT5_vect)
{
	countr++;
	if(countr==thresh){
		countr=0;
			if(a==1){
				a=2;
			}
			else{
				a=1;
			}
		
	}
}
//SR for left position encoder
ISR(INT4_vect)
{
	countl++;
	//Your code
}

void motion_set (unsigned char Direction)
{
	unsigned char PortARestore = 0;
	Direction &= 0x0F; // removing upper nibbel as it is not needed
	PortARestore = PORTA; // reading the PORTA's original status
	PortARestore &= 0xF0; // setting lower direction nibbel to 0
	PortARestore |= Direction; // adding lower nibbel for direction command and
	// restoring the PORTA status
	PORTA = PortARestore; // setting the command to the port
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

int main(){
	init_devices();
	while (1)
	{
		switch(a){
			case 1:
				thresh=183;
				forward();
				break;
			case 2:
				thresh=22;
				right();
				break;
			default:
				stop();
				break;
		}
	}
}