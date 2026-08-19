/**
 * @file main.c
 * @author Matth
 * @date 2026-08-17
 * @brief Main function
 */

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "i2c.h"
#include "i2c_LCD.h"

/* ========================================================================== */
/*                           HARDWARE CONFIGURATION                           */
/* ========================================================================== */

#define _XTAL_FREQ 4000000UL

#pragma config FOSC = INTOSC
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config MCLRE = ON
#pragma config CP = OFF
#pragma config CPD = OFF
#pragma config BOREN = OFF
#pragma config CLKOUTEN = OFF
#pragma config IESO = OFF
#pragma config FCMEN = OFF
#pragma config WRT = OFF
#pragma config PLLEN = OFF
#pragma config STVREN = ON
#pragma config BORV = LO
#pragma config LVP = ON

#define SPBRG_VALUE 25   // 9600 baud at 4 MHz with BRGH = 1
#define I2C_SLAVE 0x27


/* ========================================================================== */
/*                             SETUP & VARIABLES                              */
/* ========================================================================== */
// UART Comms Protocol between PIC16F1829 and Matrix Display Controller
#define UART_CMD_STACK  'S'     // Send button drop command
#define UART_CMD_RESET  'R'     // Send game reset command
#define UART_RESP_PASS  'P'     // Matrix response: Block placed successfully
#define UART_RESP_LOSE  'L'     // Matrix response: Game over (Loss)
#define UART_RESP_WIN  'W'     // Matrix response: Game completed (Win)

#define BUTTON_INPUT PORTAbits.RA4  // Button input pin (RA4) for user interaction
#define BUTTON_PRESSED 0          // Button pressed state (active low)

#define MAX_GAME_HEIGHT 32      // Total levels to win Stacker


typedef enum {

    GAME_STATE_PREGAME = 0,
    GAME_STATE_IN_PROGRESS = 1

}Game_State;

typedef enum{

    GAME_RESULT_LOST = 0,
    GAME_RESULT_WON = 1

}Game_Result;


uint8_t curLevel = 1;          // Current game level (1 to MAX_GAME_HEIGHT)
Game_State gameStatus = GAME_STATE_PREGAME;  // Current game state (pre-game because the game hasn't started yet)

/* ========================================================================== */
/*                         PERIPHERAL INITIALIZATION                          */
/* ========================================================================== */

void UART_Init(void)
{
    OSCCON = 0x6A;   // 4 MHz internal oscillator

    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;

    TRISCbits.TRISC4 = 1;   // RX input
    TRISCbits.TRISC5 = 0;   // TX output

    APFCON0bits.RXDTSEL = 1;
    APFCON0bits.TXCKSEL = 1;

    SPBRG = SPBRG_VALUE;

    TXSTAbits.SYNC = 0;   // async mode
    TXSTAbits.BRGH = 1;   // high speed
    RCSTAbits.SPEN = 1;   // serial port enable
    TXSTAbits.TXEN = 1;   // enable transmitter
    RCSTAbits.CREN = 1;   // enable receiver
   
}

void LCD_Init(void) {
    
    i2c_Init(); // Start I2C as Master 100KH
    I2C_LCD_Init(I2C_SLAVE); //pass I2C_SLAVE to the init function to create an instance
}

/* ========================================================================== */
/*                            UART DRIVER FUNCTIONS                           */
/* ========================================================================== */

void UART_Write(char data)
{
    while (!PIR1bits.TXIF) {
        ;
    }
    TXREG = data;
}

char UART_Data_Ready(void)
{
    return PIR1bits.RCIF;
}

char UART_Read(void)
{
    if (RCSTAbits.OERR) {
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
    }

    while (!PIR1bits.RCIF) {
        ;
    }

    return RCREG;
}


/* ========================================================================== */
/*                             LCD UI DISPLAY CONTROLLER                      */
/* ========================================================================== */


//LCD functions
void preGame(char * Sout) {
    
    I2C_LCD_Command(I2C_SLAVE, 0x01);
    I2C_LCD_Pos(I2C_SLAVE, 0x44);
    
    sprintf(Sout, "Press Button");
    I2C_LCD_SWrite(I2C_SLAVE, Sout, 12);
    
    I2C_LCD_Pos(I2C_SLAVE, 0x1A);
    
    sprintf(Sout, "To Begin");
    I2C_LCD_SWrite(I2C_SLAVE, Sout, 8);
    
    gameStatus = GAME_STATE_PREGAME;  // Set gameStatus to preGame state
    
}

void gameStart(char * Sout, uint8_t curLevel) {
  
    I2C_LCD_Command(I2C_SLAVE, 0x01);
    I2C_LCD_Pos(I2C_SLAVE, 0x44);
    
    sprintf(Sout, "Game Start");
    I2C_LCD_SWrite(I2C_SLAVE, Sout, 10);
    
    I2C_LCD_Pos(I2C_SLAVE, 0x17);
    sprintf(Sout, "Level: %d/%d", curLevel, MAX_GAME_HEIGHT);
    
    if (curLevel <= 9) {
        I2C_LCD_SWrite(I2C_SLAVE, Sout, 11);
    }
    else {
        I2C_LCD_SWrite(I2C_SLAVE, Sout, 12);
    }
    

    gameStatus = GAME_STATE_IN_PROGRESS;  // Set gameStatus to inProgress state

}

void gameProgress(char * Sout, uint8_t curLevel){
    
    I2C_LCD_Command(I2C_SLAVE, 0x01);
    I2C_LCD_Pos(I2C_SLAVE, 0x44);
    
    sprintf(Sout, "Level: %d/%d", curLevel, MAX_GAME_HEIGHT);
    
    if (curLevel <= 9) {
        I2C_LCD_SWrite(I2C_SLAVE, Sout, 11);
    }
    else {
        I2C_LCD_SWrite(I2C_SLAVE, Sout, 12);
    }
}


void gameEnd(char * Sout, uint8_t level, Game_Result result) {
    
    I2C_LCD_Command(I2C_SLAVE, 0x01);
    I2C_LCD_Pos(I2C_SLAVE, 0x45);
    
    if (result == GAME_RESULT_LOST) {
        I2C_LCD_Pos(I2C_SLAVE, 0x46);
        sprintf(Sout, "Game End");
        I2C_LCD_SWrite(I2C_SLAVE, Sout, 8);
    
        I2C_LCD_Pos(I2C_SLAVE, 0x19);
        sprintf(Sout, "Level: %d", level);
        
        if (level <= 9) {
            I2C_LCD_SWrite(I2C_SLAVE, Sout, 8);
        }
        else {
            I2C_LCD_SWrite(I2C_SLAVE, Sout, 9);
        }
        
    }
    else if (result == GAME_RESULT_WON) {
     
        I2C_LCD_Pos(I2C_SLAVE, 0x46);   // Line 1, column 6
        sprintf(Sout, "Game End");
        I2C_LCD_SWrite(I2C_SLAVE, Sout, 8);

        I2C_LCD_Pos(I2C_SLAVE, 0x19);   // Line 2, column 5
        sprintf(Sout, "Completed!");
        I2C_LCD_SWrite(I2C_SLAVE, Sout, 10);
    }
    
    __delay_ms(4500);
    
    preGame(Sout);
}


/* ========================================================================== */
/*                               MAIN APPLICATION                             */
/* ========================================================================== */

void main(void)
{
    char receivedChar = 0;
    
    unsigned char Sout[16];
 

    UART_Init();
    LCD_Init();
    
    preGame(Sout);
    
    TRISAbits.TRISA4 = 1;     // RA4 set as input pin (button)
    ANSELAbits.ANSA4 = 0;      // RA4 set as digital 
    OPTION_REGbits.nWPUEN = 0; //globally allows for pin weak pull ups to be used
    WPUAbits.WPUA4 = 1;       //pull-up enabled for RA4

            
    TRISCbits.TRISC6 = 0;   // Red LED output
    LATCbits.LATC6 = 1;
    

    while (1)
    {
        if (BUTTON_INPUT == BUTTON_PRESSED)
        {
            __delay_ms(20);

            if (BUTTON_INPUT == BUTTON_PRESSED)
            {
                if(receivedChar != UART_RESP_LOSE && receivedChar != UART_RESP_WIN) {
                    
                    if (gameStatus == GAME_STATE_PREGAME){   //gameStatus for gameStart
                        gameStart(Sout, curLevel);      //first time press
                    } else {            //gameStatus updates at gameStart to 1, will not be called anymore
                        gameProgress(Sout, curLevel);   //continuing game after that
                    }
                    
                    UART_Write(UART_CMD_STACK);   // send to Arduino
                    
                    __delay_ms(50);
                    
                    if(UART_Data_Ready()){    
                    
                    receivedChar = UART_Read();
                    
                    }
                    
                    if(receivedChar == UART_RESP_PASS) {   //If block placed successfully
                        curLevel +=1;
                    }
                }
               

                if (receivedChar == UART_RESP_LOSE) //If game lost
                {
                    
                    gameEnd(Sout, curLevel - 1 , GAME_RESULT_LOST);
                    __delay_ms(2000);
                    UART_Write(UART_CMD_RESET); // for sending reset comand
                    curLevel = 1;
                    receivedChar = 0; 
                    
                    while(BUTTON_INPUT == BUTTON_PRESSED)
                    {
                        ;
                    }
                    
                    __delay_ms(200);
                    continue;
                        

                }
                else if (receivedChar == UART_RESP_WIN) { //If Game won
                    gameEnd(Sout, curLevel - 1, GAME_RESULT_WON);
                    __delay_ms(2000);
                    UART_Write(UART_CMD_RESET); // for sendign the reset command
                    curLevel = 1;
                    receivedChar = 0; 
                    
                    while(BUTTON_INPUT == BUTTON_PRESSED)
                    {
                        ;
                    }
                    __delay_ms(200);
                    continue;
                }

                while (BUTTON_INPUT == BUTTON_PRESSED)
                {
                    ;
                }

                __delay_ms(20);
            }
        }
    }
}
