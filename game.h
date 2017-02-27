/*
 * game.h
 *
 * Written by Peter Sutton.
 *
 * Function prototypes for those functions available externally
 */

#include <stdint.h>

/*
 * The game board is 16 rows in size. Row 0 is considered to be at the top, 
 * row 15 is at the bottom. Each row is 8 columns wide.
 */
#define BOARD_ROWS 16
#define BOARD_WIDTH 8

#define MOVE_LEFT 0
#define MOVE_RIGHT 1
#define MATRIX_NUM_ROWS 8
typedef uint8_t PixelColour;
typedef PixelColour MatrixColumn[MATRIX_NUM_ROWS];
/*
 * Initialise the game.
 */
void init_game(void); 

/* 
 * Update the display for rows starting from the given row
 * (row_start) and doing so for num_rows rows. row_start should be between
 * 0 and BOARD_ROWS-1 inclusive. num_rows beyond this must still be on the 
 * board.
 */
void update_rows_on_display(uint8_t row_start, uint8_t num_rows);
void terminal_update_column(uint8_t x, MatrixColumn col);
void draw_block(uint8_t color);
/*
 * attempt_move
 * Attempts a move of the current block in the given direction 
 * (direction should be MOVE_LEFT or MOVE_RIGHT).
 * Returns 0 on failure (e.g. block was against edge or other blocks on 
 * the board prevented the move). Returns 1 on success. 
 * Should only be called if we have a current block.
 */
uint8_t attempt_move(int8_t direction);
void flag_terminal();
/*
 * Attempt to drop the current block by one row. Returns 0 on failure,
 * 1 on success.
 */
uint8_t attempt_drop_block_one_row(void);

/*
 * Attempt rotation (clockwise) of the current block on the board. 
 * Returns 0 on failure, 1 on success. 
 */
uint8_t attempt_rotation(void);

/*
 * Fix the current block to the board in its current position
 * and add another random block to the top. Returns 0 on failure
 * (new block could not be added - game over) or 1 on success.
 */
uint8_t fix_block_to_board_and_add_new_block(void);

void reset_speed_modifier(void);
void make_noise_low();
void make_noise_sad();
void make_noise_high();
void make_noise_();
void increment_speed_modifier(void);

float get_speed_modifier(void);

void reset_row_count(void);

void increment_row_count(void);

uint8_t get_row_count(void);
