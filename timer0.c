/*
 * timer0.c
 *
 * Author: Peter Sutton
 *
 * We setup timer0 to generate an interrupt every 1ms
 * We update a global clock tick variable - whose value
 * can be retrieved using the get_clock_ticks() function.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer0.h"

/* Our internal clock tick count - incremented every 
 * millisecond. Will overflow every ~49 days. */
static volatile uint32_t clock_ticks;
volatile uint8_t seven_seg_cc = 0;
uint8_t seven_seg_data[10] = {63,6,91,79,102,109,125,7,127,111};
uint8_t temp_row_count = 0;

/* Set up timer 0 to generate an interrupt every 1ms. 
 * We will divide the clock by 64 and count up to 124.
 * We will therefore get an interrupt every 64 x 125
 * clock cycles, i.e. every 1 milliseconds with an 8MHz
 * clock. 
 * The counter will be reset to 0 when it reaches it's
 * output compare value.
 */
void init_timer0(void) {
	/* Reset clock tick count. L indicates a long (32 bit) 
	 * constant. 
	 */
	clock_ticks = 0L;
	seven_seg_cc = 0;
	/* Clear the timer */
	TCNT0 = 0;

	/* Set the output compare value to be 124 */
	OCR0A = 124;
	
	/* Set the timer to clear on compare match (CTC mode)
	 * and to divide the clock by 64. This starts the timer
	 * running.
	 */
	TCCR0A = (1<<WGM01);
	TCCR0B = (1<<CS01)|(1<<CS00);

	/* Enable an interrupt on output compare match. 
	 * Note that interrupts have to be enabled globally
	 * before the interrupts will fire.
	 */
	TIMSK0 |= (1<<OCIE0A);
	
	/* Make sure the interrupt flag is cleared by writing a 
	 * 1 to it.
	 */
	TIFR0 &= (1<<OCF0A);
}
 
uint32_t get_clock_ticks(void) {
	uint32_t return_value;

	/* Disable interrupts so we can be sure that the interrupt
	 * doesn't fire when we've copied just a couple of bytes
	 * of the value. Interrupts are re-enabled if they were
	 * enabled at the start.
	 */
	uint8_t interrupts_were_on = bit_is_set(SREG, SREG_I);
	cli();
	return_value = clock_ticks;
	if(interrupts_were_on) {
		sei();
	}
	return return_value;
}

/* Interrupt handler which fires when timer/counter 0 reaches 
 * the defined output compare value (every millisecond)
 */
ISR(TIMER0_COMPA_vect) {
	/* Increment our clock tick count */
	clock_ticks++;
	/* Display a digit */
	cli();
	seven_seg_cc = 1 ^ seven_seg_cc;
	temp_row_count = get_row_count();
	if(seven_seg_cc == 0) {
		/* Display rightmost digit - tenths of seconds */
		PORTC = seven_seg_data[(temp_row_count)%10]; //THIS PROBABLY BROKE IT
		PORTD &= 0b10111111;
	} else {
		/* Display leftmost digit - seconds + decimal point */
		PORTC = seven_seg_data[(temp_row_count/10)%10];
		PORTD |= 0b01000000;
	/* Output the digit selection (CC) bit */

	}
	sei();
}