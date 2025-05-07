#include <msp430.h>
#include <libTimer.h>
#include "buzzer.h"
#include "lcdutils.h"
#include "lcddraw.h"

#define LED BIT6/* note that bit zero req'd for display */

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15
int blue = 0, green = 0, red = 0, switches = 0;
unsigned int color = 0 | 0 | 0;

static char
switch_update_interrupt_sense()
{
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);/* if switch down, sense up */
  return p2val;
}

void
switch_init()/* setup switch */
{
  P2REN |= SWITCHES;/* enables resistors for switches */
  P2IE |= SWITCHES;/* enable interrupts from switches */
  P2OUT |= SWITCHES;/* pull-ups for switches */
  P2DIR &= ~SWITCHES;/* set switches' bits for input */
  switch_update_interrupt_sense();
}
void
switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}
void print_letter(char c,char off){
  c -= 0x20;
  char truC = 0,truR = 0; 
  for (char row = 0; row < 10; row+=2) {
    
    for (char col = 14; col > 0; col-=2) {
      
      unsigned short rowBits = font_5x7[c][truR];
      unsigned short colMask = 1<<(6-truC); /* mask to select bit associated with bit */
      if(rowBits & colMask){
	drawPixel(row+10+off, col+10, COLOR_BLACK);
	drawPixel(row+10+off,col+11,COLOR_BLACK);
	drawPixel(row+11+off,col+10,COLOR_BLACK);
	drawPixel(row+11+off,col+11,COLOR_BLACK);
      
      }
      truC++;
    }
    truC=0;
    truR++;
  }
  return;
}
void clear_letters(){
  fillRectangle(5,5,100,25,COLOR_WHITE);
}
void clear_pause(){
  fillRectangle(40,100,10,30,COLOR_WHITE);
  fillRectangle(80,100,10,30,COLOR_WHITE);
  clear_letters();
}
void make_pause(){
  fillRectangle(40,100,10,30,COLOR_BLACK);
  fillRectangle(80,100,10,30,COLOR_BLACK);
  print_letter('P',0);
  print_letter('A',15);
  print_letter('U',30);
  print_letter('S',45);
  print_letter('E',60);  
}

void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;      /* clear pending sw interrupts */
    switch_interrupt_handler();/* single handler for all switches */
  }
}

int main() {

  switch_init();
  configureClocks();
  buzzer_init();
  enableWDTInterrupts();
  lcd_init();
  or_sr(0x8);          // GIE enable interupts
  clearScreen(COLOR_WHITE);
  
}
int pauseDelay = 0;
int colorCount = 0;
int pause = 0;
int count= 0;
int secondCount = 0;
int colorCycle = 0;
short newnote = 0;
short *currentNote = notesdv;
void color_swap(int color){
  fillRectangle(35,70,20,20,color);
  fillRectangle(75,70,20,20,color);
  fillRectangle(45,40,10,30,color);
  fillRectangle(85,40,10,30,color);
  fillRectangle(45,40,40,10,color);
  
}  

void __interrupt_vec(WDT_VECTOR) WDT(){
  
  if (switches & SW4) count =0;  
  pauseDelay++;
  
  if(switches & SW1 && pauseDelay >=60){
    if(!pause) {
      pause = 1;
      pauseDelay=0;
      make_pause();
    }
    else {
      pause =0;
      pauseDelay =0;
      clear_pause();
    }
  }
  
  if(!pause){
  secondCount ++;
  colorCount ++;}
  else {
    update_note(0);
  }
  if (secondCount >= lendv[count]-5) {
    update_note(0);
  }
  if (colorCount >=120){
    color_swap(color);
    switch (colorCycle){
    case 1:
      red = 31;
      blue = 0;
      green = 0;
      break;
    case 2:
      red = 0;
      blue = 31;
      green = 0;
      break;
    case 3:
      red = 0;
      blue = 0;
      green = 31;
    case 4:
      colorCycle = 0;
      break;
    default:
      color_swap(0|0|0);
      break;
    }
    colorCycle++;
    colorCount = 0;
    color = (blue << 11) | (green << 5) | red;
  } 

  if(secondCount >=(lendv[count])){
    update_note(notesdv[count]);
    count++;
    secondCount = 0;
    if(count == 67){
      count = 0;
    }
  }
}
