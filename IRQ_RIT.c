/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include "RIT.h"
#include <stdbool.h>
#include "GLCD.h"
#include <stdio.h> /*for sprintf*/
/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
volatile int down_0 = 0;
volatile int down_1 = 0;
volatile int down_2 = 0;

volatile int score = 0;             	 //punteggio
uint16_t x_current = 115;	//posizione di pacman
uint16_t y_current = 235;
uint16_t test;     				//proiezione prossima cella 
int x_temp;     						//proiezione prossima posizione
uint16_t y_temp;
uint8_t current_direction = 0; // 0 = fermo, 1 = su, 2 = giù, 3 = sinistra, 4 = destra
int threshold = 1;         //moltiplicatore per generazione vite
uint16_t x_life = 25;     //posizione vite
volatile int end = 0;								//flag fine partita
volatile int time = 60;
int pause = 2;								//flag pausa

void RIT_IRQHandler (void)
{			

/*************************INT0***************************/
if(down_0 !=0){   //verifica se una delle down è != 0, implica che è stata scatenata un'interruzione dal relativo pulsante
	down_0++;
	if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){ //leggiamo il segnale, se zero allora il pulsante è premuto
		switch(down_0){
			case 2:     //case a 2 perchè nell'Interrupt handler del pulsante impostiamo down a 1 e qui la incrementiamo nuovamente
				//your code here
			if(pause!=1)
			{
				pause = 1;
			}
			else if(pause==1){
				pause = 0; 
			}
				break;
			default:
				break;
		}
	}
	else {	/* button released */ //se nel debug con l'emulatore entra nell'else è normale, non rileva subito la pressione del pulsante
		down_0=0;			
		NVIC_EnableIRQ(EINT0_IRQn);							 /* disable Button interrupts			*/
		LPC_PINCON->PINSEL4    |= (1 << 20);     /* External interrupt 0 pin selection, lo reimposto in modalità interruzione */
	}
} // end INT0

/*************************KEY1***************************/
if(down_1 !=0){
	down_1++;
	if((LPC_GPIO2->FIOPIN & (1<<11)) == 0){    //leggiamo il segnale
		switch(down_1){
			case 2:
				//your code here
				break;
			default:
				break;
		}
	}
	else {	/* button released */
		down_1=0;			
		NVIC_EnableIRQ(EINT1_IRQn);							 /* disable Button interrupts			*/
		LPC_PINCON->PINSEL4    |= (1 << 22);     /* External interrupt 0 pin selection */
	}
} // end KEY1

/*************************KEY2***************************/
if(down_2 !=0){
	down_2++;
	if((LPC_GPIO2->FIOPIN & (1<<12)) == 0){    //leggiamo il segnale
		switch(down_2){
			//your code here
			
				break;
			default:
				break;
		}
	}
	else {	/* button released */
		down_2=0;		
		NVIC_EnableIRQ(EINT2_IRQn);							 /* disable Button interrupts			*/
		LPC_PINCON->PINSEL4    |= (1 << 24);     /* External interrupt 0 pin selection */
	}
} // end KEY2
	

	

/* JOYSTICK */ 

	static int J_up=0;      //statica perchè mi interessa che il valore non venga riazzerato ogni volta che parte la funzione che gestisce le interruzioni del RIT
	static int J_down = 0;
	static int J_left = 0;
	static int J_right = 0;

	//gestisco up
	if((LPC_GPIO1->FIOPIN & (1<<29)) == 0){	  //leggo la porta1 e controllo se il pin29 è 0, se è vero allora il joystick è up
		/* Joytick UP pressed */
		J_up++;
		switch(J_up){
			case 1:
				if(current_direction!=1)
				{
					current_direction  =1;
				}
				break;
			default:
				//your code
				break;
		}
	}
	else{
			J_up=0;
	}
	
	//gestisco left
	if((LPC_GPIO1->FIOPIN & (1<<27)) == 0){	  //leggo la porta1 e controllo se il pin27 è 0, se è vero allora il joystick è left
		/* Joytick left pressed */
		J_left++;
		switch(J_left){
			case 1:
				if(current_direction!=3)
				{
					current_direction =3;
				}
				break;
			default:
				//your code
				break;
		}
	}
	else{
			J_left=0;
	}
	
	//gestisco right
	if((LPC_GPIO1->FIOPIN & (1<<28)) == 0){	  //leggo la porta1 e controllo se il pin28 è 0, se è vero allora il joystick è right
		/* Joytick right pressed */
		J_right++;
		switch(J_right){
			case 1:
				if(current_direction!=4)
				{
					current_direction =4;
				}
				break;
			default:
				//your code
				break;
		}
	}
	else{
			J_right=0;
	}
	
	//gestisco down
	if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){	  //leggo la porta1 e controllo se il pin26 è 0, se è vero allora il joystick è up
		/* Joytick DOWN pressed */
		J_down++;
		switch(J_down){
			case 1:
				if(current_direction!=2)
				{
					current_direction =2;
				}
				break;
			default:
				//your code
				break;
		}
	}
	else{
			J_down=0;
	}
	reset_RIT();          //reset alla fine perchè abbiamp più pulsanti indipendenti
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;
}

// Funzione per muovere Pacman
void move_pacman(void) {
	if(end==0)
	{
			switch (current_direction) {
					case 1:  // Su
							y_temp = y_current -10 ;
							x_temp = x_current - 6;
							test = LCD_GetPoint(x_temp, y_temp);
							if (test != 0xF800) {  																			// Controlla se non c'è un muro
									LCD_DrawFilledCircle(x_current, y_current, 4, 0x0000);  // Cancella la posizione precedente
									y_current = y_current-10;  															// Aggiorna la posizione Y
									LCD_DrawFilledCircle(x_current, y_current, 4, 0xFFE0);  // Disegna il nuovo cerchio
							}
							if (test == 0xFFFF) {  // Controlla se c'è una standard pill
								score+=10;
							}
							if (test == 0xF81F) {  // Controlla se c'è  una power pill
								score+=50;
							}
							break;
					case 2:  // Giù
							y_temp = y_current+10 ;
							x_temp = x_current- 6;
							test = LCD_GetPoint(x_temp, y_temp);
							if (test != 0xF800) {  																			// Controlla se non c'è un muro
									LCD_DrawFilledCircle(x_current, y_current, 4, 0x0000);  // Cancella la posizione precedente
									y_current = y_current+10;  															// Aggiorna la posizione Y
									LCD_DrawFilledCircle(x_current, y_current, 4, 0xFFE0);  // Disegna il nuovo cerchio
							}
							if (test == 0xFFFF) {  
								score+=10;
							}
							if (test == 0xF81F) {  
								score+=50;
							}
							break;
					case 3:  // Sinistra
							x_temp = x_current - 15;
							test = LCD_GetPoint(x_temp, y_current);
							if (test != 0xF800) {  																			// Controlla se non c'è un muro
									LCD_DrawFilledCircle(x_current, y_current, 4, 0x0000);  // Cancella la posizione precedente
									x_current = x_current-10;  															// Aggiorna la posizione X
									LCD_DrawFilledCircle(x_current, y_current, 4, 0xFFE0);  // Disegna il nuovo cerchio
							}
							if (test == 0xFFFF) { 
								score+=10;
							}
							if (test == 0xF81F) {  
								score+=50;
							}
							if(x_temp == 0xFFFFFFF6)
							{
								x_current = 235;
								LCD_DrawFilledCircle(x_current, y_current, 4, 0x0000);
							}
							break;
					case 4:  // Destra
							x_temp = x_current + 4;
							test = LCD_GetPoint(x_temp, y_current);
							if (test != 0xF800) {  																			// Controlla se non c'è un muro
									LCD_DrawFilledCircle(x_current, y_current, 4, 0x0000);  // Cancella la posizione precedente
									x_current = x_current+10;  															// Aggiorna la posizione X
									LCD_DrawFilledCircle(x_current, y_current, 4, 0xFFE0);  // Disegna il nuovo cerchio
							}
							if (test == 0xFFFF) {  
								score+=10;
							}
							if (test == 0xF81F) {  
								score+=50;
							}
							if(x_temp == 0x000000EF)
							{
								x_current = 5;
								LCD_DrawFilledCircle(x_current, y_current, 4, 0x0000);
							}
							break;
					default:
							// Pacman è fermo, non fare nulla
							break;
			}
    }
}

void countDown()
{
	if(time!=0 && end==0){
		char countDown[32];
		sprintf(countDown, "%02d", time);
		GUI_Text(105, 10, (uint8_t *)countDown, White, Black);
	
		time--; // Decrementa il tempo
	}
	else
	{
		end=1;
		disable_timer(2);
		disable_timer(1);
		gameOverDisplay();
	}
}

void updateScore()
{
	if(end==0)
	{
	char scoreText[32];
	sprintf(scoreText, "%d", score);
	GUI_Text(200, 10, (uint8_t *)scoreText, White, Black);
	
	if(score >= threshold*1000)
	{
		threshold++;
		LCD_DrawFilledCircle(x_life, 310, 5, 0xFFE0);
		x_life += 15;
	}
	if(score == 2640)
	{
		disable_timer(2);
		disable_timer(1);
		victoryDisplay();
		end=1;
	}
}
}

void pauseResume()
{
	if (end == 0){
	if (pause == 1)
	{
		disable_timer(1);
		disable_timer(2);
		GUI_Text(100,145, (uint8_t *)"Pause", White, Red);
	}
	else if (pause == 0)
	{
		//rettangolo al centro
		LCD_DrawConsecutiveSquares(90,130,6,Blue);
		LCD_DrawConsecutiveSquares(90,140,1,Blue);
		LCD_DrawConsecutiveSquares(90,150,1,Blue);
		LCD_DrawConsecutiveSquares(90,160,1,Blue);
		LCD_DrawConsecutiveSquares(140,140,1,Blue);
		LCD_DrawConsecutiveSquares(140,150,1,Blue);
		LCD_DrawConsecutiveSquares(140,160,1,Blue);
		LCD_DrawConsecutiveSquares(90,170,6,Blue);
		//parte centrale
		LCD_DrawConsecutiveSquares(100,140,4,Black);
		LCD_DrawConsecutiveSquares(100,150,4,Black);
		LCD_DrawConsecutiveSquares(100,160,4,Black);
				
		enable_timer(1);
		enable_timer(2);
		pause = 2;//in questo modo la chiamata di questa funzione farà qualcosa solo alla pressione del tasto  enon ogni volta che verrà chiamata
	}
}
}
/******************************************************************************
**                            End Of File
******************************************************************************/
