/** \file lcddemo.c
 *  \brief A simple demo that draws a string and square
 */

#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"

/** Initializes everything, clears the screen, draws "hello" and a square */
int
main()
{
  configureClocks();
  lcd_init();
  u_char width = screenWidth, height = screenHeight;

  clearScreen(COLOR_GREEN);

  int col = 0, row = 0;
  int centerCol = width / 2, centerRow = height / 2;

  //drawString5x7(20,20, "hello", COLOR_GREEN, COLOR_RED);
  drawString11x16(centerCol, centerRow, "Hello", COLOR_RED, COLOR_GREEN);

  //fillRectangle(30,30, 60, 60, COLOR_ORANGE);
  
}
