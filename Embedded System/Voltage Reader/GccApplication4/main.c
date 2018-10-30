#include <avr/io.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "avr.h"

#include <stdlib.h>

float voltage;
char voltageshow[4];

float min = 5.0, max = 0.0, avg, total = 0;
int n;
/*ISR (TIMER1_OVF_vect)    // Timer1 ISR
{
	//dtostrf(ADC/1024.0 * 5, 1, 4, voltageshow);
	pos_lcd(0,0);
	puts_lcd2(voltageshow);
	PORTB ^= 16;
	TCNT1 = 49911;  // 65,526 - 15,625, wait for 0.5 sec at 8 MHz
}*/

void displayblank(int r, int c, char* pos)
{
	pos_lcd(r,c);
	puts_lcd2(pos);
	pos_lcd(r, c+2);
	puts_lcd2("-----");
}

void displayallblanks()
{
	clr_lcd();
	displayblank(0, 0, "I ");
	displayblank(0, 8, "m ");
	displayblank(1, 0, "M ");
	displayblank(1, 8, "a ");
}

void display(float x, int r, int c, char* pos)
{
	dtostrf(x, 1, 3, voltageshow);
	//strcat(pos, voltageshow);
	pos_lcd(r,c);
	puts_lcd2(pos);
	pos_lcd(r, c+2);
	puts_lcd2(voltageshow);
}

void displayall()
{
	display(voltage, 0, 0, "I ");
	display(min, 0, 8, "m ");
	display(max, 1, 0, "M ");
	display(avg, 1, 8, "a ");
}
char c = 40;
ISR (ADC_vect)    // Timer1 ISR
{
	clr_lcd();
	SET_BIT(TIFR, TOV1); // clear timer overflow vector flag 1
	voltage = ADC/1023.0 * 5;
	n++;
	total += voltage;
	min = voltage > min ? min : voltage;
	max = voltage < max ? max : voltage;
	avg = total/n;
	displayall();
	PORTB ^= 16;
	TCNT1 = 49911;  // 65,526 - 15,625, wait for 0.5 sec at 8 MHz
}
int main()
{
	ini_lcd();
	clr_lcd();
	CLR_BIT(DDRA, 0);
	
	SET_BIT(DDRB, 4);   
	TCNT1 = 49911; 
	TCCR1B = (1<<CS12);
	
	//TIMSK = (1 << TOIE1);   // Enable timer1 overflow interrupt(TOIE1)
	SET_BIT(SFIOR, ADTS1);
	SET_BIT(SFIOR, ADTS2);
	sei();  // Enable global interrupts by setting global interrupt enable bit in SREG

	SET_BIT(ADCSRA, ADEN); //ADEN = 1
	SET_BIT(ADCSRA, ADATE);
	SET_BIT(ADCSRA, ADIE);
	SET_BIT(ADCSRA, ADPS2);
	SET_BIT(ADCSRA, ADPS1);
	SET_BIT(ADCSRA, ADPS0);

	SET_BIT(ADMUX, REFS0);
	
	SET_BIT(ADCSRA, ADSC); //ADSC = 1

	while(1)
	{
		checkforinput();
	}
}

int is_pressed(int r, int c)
{
	DDRC = 0;
	PORTC = 0;
	SET_BIT(PORTC, c+4);
	SET_BIT(DDRC, r);
	CLR_BIT(PORTC, r);
	if (GET_BIT(PINC, c+4))
	{
		return 0;
	}
	else
	{
		return 1;
	}

}

int get_key()
{
	int r,c;
	for (r = 0; r < 4; ++r)
	{
		for (c = 0; c < 4; ++c)
		{
			if (is_pressed(r,c))
			{
				return r*4 + c + 1;
			}
		}
	}
	return 0;
}

int b_get_key()
{
	int k = get_key();
	if (k == 0) return 0;
	while (k == get_key());
	return k;
}

void checkforinput()
{
	int k = b_get_key();
	switch (k){
		case 1:
			min = 6;
			max = 0;
			n = 0;
			avg = 0;
			total = 0;
			voltage = 0;
			CLR_BIT(SFIOR, ADTS1);
			CLR_BIT(SFIOR, ADTS2);	
			displayallblanks();
			break;
		case 2:
			SET_BIT(SFIOR, ADTS1);
			SET_BIT(SFIOR, ADTS2);
			break;
	}
}
