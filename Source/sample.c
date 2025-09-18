/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The GLCD application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-7
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Paolo Bernardi
** Modified date:           03/01/2020
** Version:                 v2.0
** Descriptions:            basic program for LCD and Touch Panel teaching
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "GLCD/GLCD.h" 
#include "TouchPanel/TouchPanel.h"
#include <stdio.h>
#include "LPC17xx.h"                    /* LPC17xx definitions                */
#include "button_EXINT/button.h"
#include <stdio.h> /*for sprintf*/

#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif

extern time;
extern score;

void start(){
	time =60;
	score =0;
	LCD_Clear(Black);
	enable_timer(0);   //[timer per generazione seed casuale]
	drawMaze();        //generazione labirinto
	pills();          //generazione pills
	char countDownGame[32];
	sprintf(countDownGame, "Game over in ");
	char seconds[32];
	sprintf(seconds, "s|");
	GUI_Text(125, 10, (uint8_t *)seconds, White, Black);
	char scoreText[32];
	sprintf(scoreText, "Score:");
	GUI_Text(145, 10, (uint8_t *)scoreText, White, Black);
	GUI_Text(5, 10, (uint8_t *)countDownGame, White, Black);

	LPC_SC -> PCONP |= (1 << 22);  // TURN ON TIMER 2
	LPC_SC -> PCONP |= (1 << 23);  // TURN ON TIMER 3	
	
	LCD_DrawFilledCircle(115, 235, 4 , Yellow);  // prima creazione di PACMAN
	init_timer(1, 0, 0, 3, 0x3EE710);							/*[timer per movimento pacman] timer , no prescaler, matchregister , modalità interrupt e reset, intervallo */
	enable_timer(1);

	init_timer(2, 0, 0, 3, 0x17D7840);							/*[timer per countdown] timer , no prescaler, matchregister , modalità interrupt e reset, intervallo) */
	enable_timer(2);

	init_timer(3, 0, 0, 3, 0x2FB11A);							/*[timer per pausa e score] timer , no prescaler, matchregister , modalità interrupt e reset, intervallo) */
	enable_timer(3);


	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);						
	}
//il display condivide il bus con i led, non possono essere usati insieme
int main(void)
{
	SystemInit();  												/* System Initialization (i.e., PLL)  */
	BUTTON_init();												/* BUTTON Initialization              */
	init_RIT(0x004C4B40);									/* RIT Initialization 50 msec       */
	enable_RIT();										/* enable RIT to count 50ms				 */
  LCD_Initialization();
	joystick_init();											/* Joystick Initialization            */
	//ADC_init();														/* ADC Initialization									*/

  TP_Init();                        //inizializzazione touch screen
	//TouchPanel_Calibrate();           //IN EMULAZIONE PUOI TOGLIERLO PER FACILITARE IL DEBUG

	start();
	
  while (1)	
  {
		__ASM("wfi");
  }
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
