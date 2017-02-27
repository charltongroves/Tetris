/*
 * project.c
 *
 * Main file for the Tetris Project.
 *
 * Author: Peter Sutton. Modified by <YOUR NAME HERE>
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>		// For random()
#define F_CPU 8000000L
#include <util/delay.h>
#include <avr/eeprom.h>

#include "ledmatrix.h"
#include "scrolling_char_display.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "score.h"
#include "timer0.h"
#include "game.h"

#define F_CPU 8000000L
#define first 0
#define second 5
#define third 10
#define fourth 15
#define fifth 20
#define numonboard 25
#define signature 26

#include <util/delay.h>
#include <avr/eeprom.h>

// Function prototypes - these are defined below (after main()) in the order
// given here
extern int isalpha(int __c) __ATTR_CONST__;
void initialise_hardware(void);
void splash_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);
void handle_new_lap(void);

// ASCII code for Escape character
#define ESCAPE_CHAR 27
#define SPACE 32


char first_leader[3], second_leader[3], third_leader[3], fourth_leader[3], fifth_leader[3];
char first_leaderboard[3], second_leaderboard[3], third_leaderboard[3], fourth_leaderboard[3], fifth_leaderboard[3];
uint16_t first_score_sort, second_score_sort, third_score_sort, fourth_score_sort, fifth_score_sort, new_score_sort;
unsigned char first_letter, second_letter, third_letter;
uint16_t first_score = 0, second_score = 0, third_score = 0, fourth_score = 0, fifth_score = 0, new_score;
uint8_t num_on_board;
uint8_t xory;
uint16_t x_value=500, y_value=500;
uint32_t joystick_delay = 1;
/////////////////////////////// main //////////////////////////////////
int main(void) {
	
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	reset_row_count();
	// Show the splash screen message. Returns when display
	// is complete
	splash_screen();
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}
//void update_ssd() {
	///* Display a digit */
	//seven_seg_cc = 1 ^ seven_seg_cc;
	//
	//if(seven_seg_cc == 0) {
		///* Display rightmost digit - tenths of seconds */
		//PORTC = seven_seg_data[get_row_count()%10];
		//PORTD = PORTD - 64;
		//} else {
		///* Display leftmost digit - seconds + decimal point */
		//PORTC = seven_seg_data[(get_row_count()/10)%10];
		//PORTD = PORTD + 64;
	//}
	///* Output the digit selection (CC) bit */
//}
uint8_t compare_score() {
	uint16_t lowest_score = 0;
	uint8_t low_score_index = 0;
	if (first_score >= second_score) {
		lowest_score = second_score;
		low_score_index = 2;
		} if (second_score >= first_score) {
		lowest_score = first_score;
		low_score_index = 1;
		} if (lowest_score >= third_score) {
		lowest_score = third_score;
		low_score_index = 3;
		} if (lowest_score >= fourth_score) {
		lowest_score = fourth_score;
		low_score_index = 4;
		} if (lowest_score >= fifth_score) {
		lowest_score = fifth_score;
		low_score_index = 5;
	}
	if (get_score() > lowest_score) {
		return low_score_index;
		} else {
		return 0;
	}
}
int comp (const void * elem1, const void * elem2) {
	return ( *(int*)elem2 - *(int*)elem1 );
}
void sort_score(uint8_t length) {
	if (num_on_board > 0) {
		uint16_t all_scores[] = {first_score, second_score, third_score, fourth_score, fifth_score};
		/*move_cursor(1,20);
		printf_P(PSTR("%u"), all_scores[0]);
		move_cursor(1,21);
		printf_P(PSTR("%u"), all_scores[1]);
		move_cursor(1,22);
		printf_P(PSTR("%u"), length);*/
		qsort(all_scores, length, sizeof(uint16_t), comp);
		for (uint8_t i = 0; i < length; i++) {
			/*move_cursor(1,1+i);
			printf_P(PSTR("%u"), all_scores[i]);*/
			if (all_scores[i] == first_score) {
				move_cursor(3,7+i);
				printf_P(PSTR("%c%c%c : %u"), first_leader[0], first_leader[1], first_leader[2], first_score);
			} else if (all_scores[i] == second_score) {
				move_cursor(3,7+i);
				printf_P(PSTR("%c%c%c : %u"), second_leader[0], second_leader[1], second_leader[2], second_score);
			} else if (all_scores[i] == third_score) {
				move_cursor(3,7+i);
				printf_P(PSTR("%c%c%c : %u"), third_leader[0], third_leader[1], third_leader[2], third_score);
			} else if (all_scores[i] == fourth_score) {
				move_cursor(3,7+i);
				printf_P(PSTR("%c%c%c : %u"), fourth_leader[0], fourth_leader[1], fourth_leader[2], fourth_score);
			} else if (all_scores[i] == fifth_score) {
				move_cursor(3,7+i);
				printf_P(PSTR("%c%c%c : %u"), fifth_leader[0], fifth_leader[1], fifth_leader[2], fifth_score);
			}
		}
	}
}
void input_name(void) {
	first_letter = 255;
	second_letter = 255;
	third_letter = 255;
	while (1) {
		char serial_input = -1;
		if(serial_input_available()) {
			serial_input = fgetc(stdin);
			if (isalpha(serial_input)) {
				if (first_letter == 255) {
					first_letter = serial_input;
					printf_P(PSTR("%c"),first_letter);
					} else if (second_letter == 255) {
					second_letter = serial_input;
					printf_P(PSTR("%c"),second_letter);
					} else if (third_letter == 255) {
					third_letter = serial_input;
					printf_P(PSTR("%c"),third_letter);
					break;
				}
			}
		}
		//update_ssd();
		_delay_ms(10);
	}
}

void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	if (eeprom_read_byte((uint8_t*)signature) != 131) {
		eeprom_write_byte((uint8_t*)signature, (uint8_t) 131);
		eeprom_write_byte((uint8_t*)numonboard, (uint8_t) 0 );
	}
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200,0);
	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS2);
	// Set up our main timer to give us an interrupt every millisecond
	init_timer0();
	DDRC = 0xFF;
	DDRD = 0xC1;
	// Turn on global interrupts
	sei();
	

}
void joystick_main(void) {
	if (xory == 0) {
		//move_cursor(1,19);
		//printf_P(PSTR("THISWORK"));
		ADMUX = (1<<REFS0)|(0<<MUX0);
	} else {
		//move_cursor(1,19);
		//printf_P(PSTR("xx       "));
		ADMUX = (1<<REFS0)|(1<<MUX0);
	}
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<ADSC)) {
		;
	}
	if(xory == 0) {
		x_value = ADC;
		xory = 1;
	} else {
		//move_cursor(1,19);
		//printf_P(PSTR("THISWORK"));
		y_value = ADC;
		xory = 0;
	}

	
}
uint8_t check_left(void) {
	if ((x_value > 850) && (joystick_delay < get_clock_ticks())) {
		joystick_delay = get_clock_ticks() +128;
		return 1;
	} else {
		return 0;
	}
}
uint8_t check_right(void) {
	if ((x_value < 150) && (joystick_delay < get_clock_ticks())) {
		joystick_delay = get_clock_ticks() +128;
		return 1;
	} else {
		return 0;
	}
}
uint8_t check_up(void) {
	if ((y_value > 850) && (joystick_delay < get_clock_ticks())) {
		joystick_delay = get_clock_ticks() +256;
		return 1;
	} else {
		return 0;
	}
}
uint8_t check_down(void) {
	
	if ((y_value < 150) && (joystick_delay < get_clock_ticks())) {
		joystick_delay = get_clock_ticks() + 128;
		return 1;
	} else {
		return 0;
	}
}
void read_highscores(void) {
	/*memset(&first_leader[0], 0, sizeof(first_leader));
	memset(&second_leader[0], 0, sizeof(second_leader));
	memset(&third_leader[0], 0, sizeof(third_leader));
	memset(&fourth_leader[0], 0, sizeof(fourth_leader));
	memset(&fifth_leader[0], 0, sizeof(fifth_leader));*/
	num_on_board = eeprom_read_byte((uint8_t*)numonboard);
	if (num_on_board > 0) {
		move_cursor(0,6);
		printf_P(PSTR(" HIGHSCORES \n"));
		first_leader[0] = eeprom_read_byte((uint8_t*)0); 
		first_leader[1] = eeprom_read_byte((uint8_t*)1);
		first_leader[2] = eeprom_read_byte((uint8_t*)2);
		first_score = eeprom_read_word((uint16_t*)3);
	}
	if (num_on_board > 1) {
		second_leader[0] = eeprom_read_byte((uint8_t*)5);
		second_leader[1] = eeprom_read_byte((uint8_t*)6);
		second_leader[2] = eeprom_read_byte((uint8_t*)7);
		second_score = eeprom_read_word((uint16_t*)8);
	}
	if (num_on_board > 2) {
		third_leader[0] = eeprom_read_byte((uint8_t*)10);
		third_leader[1] = eeprom_read_byte((uint8_t*)11);
		third_leader[2] = eeprom_read_byte((uint8_t*)12);
		third_score = eeprom_read_word((uint16_t*)13);
	}
	if (num_on_board > 3) {
		fourth_leader[0] = eeprom_read_byte((uint8_t*)15);
		fourth_leader[1] = eeprom_read_byte((uint8_t*)16);
		fourth_leader[2] = eeprom_read_byte((uint8_t*)17);
		fourth_score = eeprom_read_word((uint16_t*)18);
	}
	if (num_on_board > 4) {
		fifth_leader[0] = eeprom_read_byte((uint8_t*)20);
		fifth_leader[1] = eeprom_read_byte((uint8_t*)21);
		fifth_leader[2] = eeprom_read_byte((uint8_t*)22);
		fifth_score = eeprom_read_word((uint16_t*)23);
	}
}

void write_highscores(void) {
	if (num_on_board > 0) {
		move_cursor(3,7);
		printf_P(PSTR("%c%c%c : %u"), first_leader[0], first_leader[1], first_leader[2], first_score);
	}
	if (num_on_board > 1) {
		move_cursor(3,8);
		printf_P(PSTR("%c%c%c : %u"), second_leader[0], second_leader[1], second_leader[2], second_score);
	}
	if (num_on_board > 2) {
		move_cursor(3,9);
		printf_P(PSTR("%c%c%c : %u"), third_leader[0], third_leader[1], third_leader[2], third_score);
	}
	if (num_on_board > 3) {
		move_cursor(3,10);
		printf_P(PSTR("%c%c%c : %u"), fourth_leader[0],fourth_leader[1],fourth_leader[2], fourth_score);
	}
	if (num_on_board > 4) {
		move_cursor(3,11);
		printf_P(PSTR("%c%c%c : %u"), fifth_leader[0], fifth_leader[1], fifth_leader[2], fifth_score);
	}
}
void splash_screen(void) {
	reset_row_count();
	// Reset display attributes and clear terminal screen then output a message
	set_display_attribute(TERM_RESET);
	clear_terminal();
	hide_cursor();	// We don't need to see the cursor when we're just doing output
	move_cursor(3,3);
	printf_P(PSTR("Tetris"));
	move_cursor(3,5);
	set_display_attribute(FG_GREEN);	// Make the text green
	printf_P(PSTR("CSSE2010/7201 Tetris Project by Charlton Groves"));	
	set_display_attribute(FG_WHITE);	// Return to default colour (White)
	// Output the scrolling message to the LED matrix
	// and wait for a push button to be pushed.
	ledmatrix_clear();
	//eeprom_write_byte((uint8_t*)numonboard, (uint8_t) 0);
	
	// Red message the first time through
	PixelColour colour = COLOUR_RED;
	while(1) {
		set_scrolling_display_text("43960914", colour);
		// Scroll the message until it has scrolled off the 
		// display or a button is pushed. We pause for 130ms between each scroll.
		while(scroll_display()) {
			_delay_ms(130);
			if(button_pushed() != -1) {
				// A button has been pushed
				return;
			}
		}
		// Message has scrolled off the display. Change colour
		// to a random colour and scroll again.
		switch(random()%4) {
			case 0: colour = COLOUR_LIGHT_ORANGE; break;
			case 1: colour = COLOUR_RED; break;
			case 2: colour = COLOUR_YELLOW; break;
			case 3: colour = COLOUR_GREEN; break;
		}
	}
}

void new_game(void) {
	// Initialise the game and display
	clear_terminal();
	init_game();
	reset_row_count();
	// Clear the serial terminal
	draw_horizontal_line(7,39,48);
	draw_horizontal_line(24,39,48);
	draw_vertical_line(39,7,24);
	draw_vertical_line(48,7,24);
	move_cursor(40,4);
	printf_P(PSTR("Score: %6d"), get_score());
	move_cursor(40,5);
	printf_P(PSTR("Row Count: %3d"), get_row_count());
	// Initialise the score
	init_score();
	
	// Delete any pending button pushes or serial input
	empty_button_queue();
	clear_serial_input_buffer();
	reset_speed_modifier();
	num_on_board = eeprom_read_byte((uint8_t*)numonboard);
	read_highscores();
	//write_highscores();
	sort_score(num_on_board);
}

void play_game(void) {
	uint32_t last_drop_time;
	int8_t button, paused;
	char serial_input, escape_sequence_char;
	uint8_t characters_into_escape_sequence = 0;
	paused = 0;

	
	// Record the last time a block was dropped as the current time -
	// this ensures we don't drop a block immediately.
	last_drop_time = get_clock_ticks();

	// We play the game forever. If the game is over, we will break out of
	// this loop. The loop checks for events (button pushes, serial input etc.)
	// and on a regular basis will drop the falling block down by one row.
	while(1) {
		// Check for input - which could be a button push or serial input.
		// Serial input may be part of an escape sequence, e.g. ESC [ D
		// is a left cursor key press. We will be processing each character
		// independently and can't do anything until we get the third character.
		// At most one of the following three variables will be set to a value 
		// other than -1 if input is available.
		// (We don't initalise button to -1 since button_pushed() will return -1
		// if no button pushes are waiting to be returned.)
		// Button pushes take priority over serial input. If there are both then
		// we'll retrieve the serial input the next time through this loop
		serial_input = -1;
		escape_sequence_char = -1;
		button = button_pushed();
		if(button == -1) {
			// No push button was pushed, see if there is any serial input
			if(serial_input_available()) {
				// Serial data was available - read the data from standard input
				serial_input = fgetc(stdin);
				// Check if the character is part of an escape sequence
				if(characters_into_escape_sequence == 0 && serial_input == ESCAPE_CHAR) {
					// We've hit the first character in an escape sequence (escape)
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 1 && serial_input == '[') {
					// We've hit the second character in an escape sequence
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 2) {
					// Third (and last) character in the escape sequence
					escape_sequence_char = serial_input;
					serial_input = -1;  // Don't further process this character - we
										// deal with it as part of the escape sequence
					characters_into_escape_sequence = 0;
				} else {
					// Character was not part of an escape sequence (or we received
					// an invalid second character in the sequence). We'll process 
					// the data in the serial_input variable.
					characters_into_escape_sequence = 0;
				}
			}
		}
		
		// Process the input.
		if (paused == 0) { 
			if(button==3 || escape_sequence_char=='D' || check_left()) {
				// Attempt to move left
				(void)attempt_move(MOVE_LEFT);
			} else if(button==0 || escape_sequence_char=='C'|| check_right()) {
				// Attempt to move right
				(void)attempt_move(MOVE_RIGHT);
			} else if (button==2 || escape_sequence_char == 'A' || check_up()) {
				// Attempt to rotate
				(void)attempt_rotation();
			} else if (button==1 || serial_input == SPACE) {
				// Attempt to drop block
				flag_terminal();
				while(1) {
					if(!attempt_drop_block_one_row()) {
						break;
					}
				}
				flag_terminal();
				update_rows_on_display(0, BOARD_ROWS);
				if(!fix_block_to_board_and_add_new_block()) {
					break;
				}
				last_drop_time = get_clock_ticks();
			} else if (escape_sequence_char == 'B' || check_down()) {
				if(!attempt_drop_block_one_row()) {
					if(!fix_block_to_board_and_add_new_block()) {
						break;
					}
				}
			}
		}
		if(serial_input == 'p' || serial_input == 'P') {
			paused = 1 ^ paused;
			
			// Unimplemented feature - pause/unpause the game until 'p' or 'P' is
			// pressed again. All other input (buttons, serial etc.) must be ignored.
		} 
		// else - invalid input or we're part way through an escape sequence -
		// do nothing
		
		// Check for timer related events here
		if(paused == 0) {
			if(get_clock_ticks() >= last_drop_time + 60000/get_speed_modifier()) {
				// 600ms (0.6 second) has passed since the last time we dropped
				// a block, so drop it now.
				if(!attempt_drop_block_one_row()) {
					// Drop failed - fix block to board and add new block
					if(!fix_block_to_board_and_add_new_block()) {
						break;	// GAME OVER
					}
				}
				last_drop_time = get_clock_ticks();
			}
		}
		//update_ssd();
		joystick_main();
	}
	// If we get here the game is over. 
}
void upload_highscore(uint16_t start) {
	uint16_t finalscore = get_score();
	unsigned char *p = (unsigned char*)&finalscore;
	eeprom_write_byte((uint8_t*) start, (uint8_t) first_letter);
	eeprom_write_byte((uint8_t*) start+1, (uint8_t) second_letter);
	eeprom_write_byte((uint8_t*) start+2, (uint8_t) third_letter);
	eeprom_write_byte((uint8_t*) start+3, (uint8_t) p[0]);
	eeprom_write_byte((uint8_t*) start+4, (uint8_t) p[1]);
}
void handle_game_over() {
	move_cursor(2,2);
	// Print a message to the terminal. 
	printf_P(PSTR("GAME OVER"));
	make_noise_low();
	make_noise_low();
	make_noise_sad();
	if (num_on_board < 5) {
		move_cursor(2,3);
		set_display_attribute(FG_YELLOW);
		printf_P(PSTR("NEW HIGHSCORE\n"));
		set_display_attribute(FG_WHITE);
		printf_P(PSTR("Enter your name:"));
		uint8_t overwrite = num_on_board+1;
		input_name();
		if (overwrite == 0) {
			
		} else if (overwrite == 1) {
			upload_highscore(first);
		} else if (overwrite == 2) {
			upload_highscore(second);
		} else if (overwrite == 3) {
			upload_highscore(third);
		} else if (overwrite == 4) {
			upload_highscore(fourth);
		} else if (overwrite == 5) {
			upload_highscore(fifth);
		}
		eeprom_write_byte((uint8_t*)numonboard, (uint8_t) num_on_board+1);
	} else {
		uint8_t overwrite = compare_score();
		/*move_cursor(1,23);
		printf_P(PSTR("%u"), compare_score());*/
		if (overwrite != 0) {
			move_cursor(2,3);
			set_display_attribute(FG_YELLOW);
			printf_P(PSTR("NEW HIGHSCORE\n"));
			set_display_attribute(FG_WHITE);
			printf_P(PSTR("Enter your name:"));
			input_name();
		}
		if (overwrite == 1) {
			upload_highscore(first);
		} else if (overwrite == 2) {
			upload_highscore(second);
		} else if (overwrite == 3) {
			upload_highscore(third);
		} else if (overwrite == 4) {
			upload_highscore(fourth);
		} else if (overwrite == 5) {
			upload_highscore(fifth);
		}
	}
	read_highscores();
	sort_score(num_on_board);
	/*move_cursor(2,3);
	printf_P(PSTR("%d %d %c"), first_letter, second_letter, third_letter);*/

	while(button_pushed() == -1) {
		_delay_ms(1);
		//update_ssd()
		; // wait until a button has been pushed
	}
	
}

