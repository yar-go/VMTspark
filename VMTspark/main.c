#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

volatile uint32_t micros;
uint32_t rpm;
uint32_t max_rpm;

uint32_t old_open_time;
uint32_t open_time;
uint32_t diff_time;

uint32_t angle;

uint32_t old_start, new_start;
uint32_t old_end, new_end;


bool modulator_state;
bool old_modulator_state;



ISR(TIM0_COMPA_vect){
	micros += 32;
	TCNT0 = 0;
}

inline void timer_ini(){
	SREG |= (1<<7); // interupts on
	TCCR0B |= (1<<CS01); // 8x
	TCNT0 = 0;
	OCR0A = 38;
	TIMSK0 |= (1<<OCIE0A);
};



inline void calcAngle(){
	if (rpm<1000) angle = 2;
	else if(rpm<3000) angle = 27*rpm/2000 - 10;
	else if(rpm<7000) angle = 8*rpm/2000 + 18;
	
	angle = constrain(angle, 0, 38);
	//angle = map(rpm, 1000,);
}

inline void setup(){
	micros = 0;
	DDRB |= (1<<PB0);
	PORTB &= ~(PB1 | PB2 | PB0);
	
	old_start = 0;
	old_end = 0;
	
	old_modulator_state = PINB & (1<<PB2);
}

inline void loop(){	
	modulator_state = PINB & (1<<PB2);
	
	if (old_modulator_state && !modulator_state){//шторка відкрилась
		open_time = micros;
		
		diff_time = open_time - old_open_time;		// розрахунок оборотів двигуна
		rpm = 60000000UL/diff_time;
		
		calcAngle();
		
		new_start = open_time + (diff_time / 36)*(360 - angle)/10;  // розрахунок часу відкриття та закриття
		new_end = new_start + diff_time/4; // 90 градусів
		
		old_open_time = open_time;
	}
	old_modulator_state = modulator_state;
	
	
	if (old_start < micros && micros < old_end && rpm>80 && (!(PINB & (1<<PB1)) || rpm<3000)){
			PORTB &= ~(1<<PB0);
		}else{
			old_start = new_start;
			old_end = new_end;
			PORTB |= (1<<PB0);
	}

}

int main(void)
{
	timer_ini();
    setup();
    while (1) 
    {
		loop();
    }
}

