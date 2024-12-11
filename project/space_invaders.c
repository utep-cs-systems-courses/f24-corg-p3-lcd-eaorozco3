#include <msp430.h>
#include <stdio.h> // Library is only used to allow the score to be converted to a char array.
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include "buzzer.h"
#include "transitions.h"
 
// Initialize green LED.
#define LED BIT6

// Initialize switch/switches.
#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8
#define SWITCHES 15

// Global variables for spaceship control
unsigned char currentCol = screenWidth / 2;
unsigned char oldCol = screenWidth / 2;
unsigned char currentRow = screenHeight - 30;
unsigned char init_laser = 0;

// Global variables for elements that require updating. e.g. score and speed.
int scoreValue = 0;
char scoreString[3] = "0";
unsigned char alienSpeed = 1;
unsigned char traversal_interval = 0;

/** 
Contains the positions of importance regarding the alien.
0. Alien's current column position.
1. Alien's current row position.
2. Alien's old position.
*/
char alienPositions[] = {0, 15, 0};

static char 
switch_update_interrupt_sense()
{
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);	/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);	/* if switch down, sense up */
  return p2val;
}

void 
switch_init()			/* setup switch */
{  
  P2REN |= SWITCHES;		/* enables resistors for switches */
  P2IE |= SWITCHES;		/* enable interrupts from switches */
  P2OUT |= SWITCHES;		/* pull-ups for switches */
  P2DIR &= ~SWITCHES;		/* set switches' bits for input */
  switch_update_interrupt_sense();
}

int switches = 0;

void
switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}

short redrawScreen = 1;

void wdt_c_handler()
{
  static int secCount = 0;

  secCount ++;
  if (secCount >= 25) { // Every 1/10 of a second.
    if(alienPositions[0] <= screenWidth - 3) { // If the alien is still in range, keep moving.
      alienPositions[0] += alienSpeed;
    } else {alienPositions[0] = 0;} // If out or range, return to start.
    

    if (switches & SW3) alienSpeed++; // Increase alien speed indefinitely.
    if (switches & SW2) init_laser = 1; // Allow for laser to be drawn.
    if (switches & SW1 && (currentCol - 12) > 5) currentCol -= 5; // Move left.
    if (switches & SW4 && (currentCol + 19) < screenWidth) currentCol += 5;  // Move right.

    secCount = 0;
    redrawScreen = 1;
    traversal_interval++; // Allows for a 1 second interval to occur after alien is shot down.
  }
}

void draw_firstalien(u_char columnMin, u_char rowMin, unsigned short color) {
  fillRectangle(columnMin + 5, rowMin, 10, 3, color);
  fillRectangle(columnMin, rowMin + 3, 20, 3, color);
}

void update_firstalien() {
  // Move alien if new and old positions are different. Update values afterward.
  if(alienPositions[0] != alienPositions[2] && traversal_interval >= 10) {
    draw_firstalien(alienPositions[2], alienPositions[1], COLOR_BLACK);
    draw_firstalien(alienPositions[0], alienPositions[1], COLOR_GREEN);
    alienPositions[2] = alienPositions[0];
  }

  if(traversal_interval < 10) {
    draw_firstalien(alienPositions[2], alienPositions[1], COLOR_BLACK);}
}

void update_laser() {
  if(init_laser) {
    // When a laser is shot, the column stays the same and the row changes.
    unsigned char laser_column = currentCol;
    unsigned char laser_row = currentRow;
    buzzer_set_period(2000);
    laser_row -= 3;
    while(laser_row >= 10) { // Draw laser.
      fillRectangle(laser_column + 4, laser_row - 22, 5, 25, COLOR_BLACK);
      laser_row -= 3;
      fillRectangle(laser_column + 4, laser_row - 22, 5, 25, COLOR_RED);
      if(laser_row <= alienPositions[1] && (((laser_column + 5) >= alienPositions[0]) && (laser_column + 5) <= alienPositions[0] + 10) && traversal_interval >= 10) { // If laser and alien meet.
	update_transitions(); // Update alien speed and score value.
	sprintf(scoreString, "%d", scoreValue); // Update score char array. 
	traversal_interval = 0; // Reset interval.
	fillRectangle(laser_column + 4, laser_row - 22, 5, 25, COLOR_BLACK);
	goto end_of_laser;
      } 
    }

  end_of_laser:
    init_laser = 0;
    fillRectangle(0, 0, screenWidth, 15, COLOR_BLACK);
    return;
  }
}

void draw_spaceship(u_char colMin, u_char rowMin, unsigned short color) {
  fillRectangle(colMin, rowMin, 10, 3, color);
  fillRectangle(colMin - 3, rowMin + 3, 15, 3, color);
  fillRectangle(colMin -6, rowMin + 6, 20, 3, color);
  fillRectangle(colMin - 9, rowMin + 9, 25, 3, color);
  fillRectangle(colMin - 12, rowMin + 12, 30, 3, color);
}

void update_spaceship() {
  static char firstTime = 1;
  // Draw spaceship before any button presses are made.
  if(firstTime) {draw_spaceship(currentCol, currentRow, COLOR_MAGENTA); firstTime = 0;}
  if(currentCol != oldCol) { // Move spaceship if positions are different.
     draw_spaceship(oldCol, currentRow, COLOR_BLACK);
     draw_spaceship(currentCol, currentRow, COLOR_MAGENTA);
     oldCol = currentCol;
     buzzer_set_period(1500); // Make a sound to inform the user the spaceship moved.
  }
}

void main()
{
  P1DIR |= LED;
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  buzzer_init();
  
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */

  clearScreen(COLOR_BLACK);
  while(1) {
    if(redrawScreen) {
      buzzer_set_period(0);
      redrawScreen = 0;
      update_spaceship();
      update_laser();
      update_firstalien();
      fillRectangle(0, screenHeight - 15, screenWidth, 1, COLOR_WHITE);
      drawString5x7(5, screenHeight - 10, "Score:", COLOR_WHITE, COLOR_BLACK);
      drawString5x7(45, screenHeight - 10, scoreString, COLOR_WHITE, COLOR_BLACK);
      drawRectOutline(0, 0, screenWidth, screenHeight, COLOR_BLACK);
    }

    P1OUT &= ~LED;
    or_sr(0x10); // Turn off the CPU.
    P1OUT |= LED;
  }
}

void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {	      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    switch_interrupt_handler();	/* single handler for all switches */
  }
}
