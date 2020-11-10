/*******************************************************************************
 * File:           main.c                                                      *     
 * Author:         Saulo G. Felix                                              *
 *                                                                             *     
 * Created on 9 de Novembro de 2020, 02:28                                     * 
 *******************************************************************************
 * Arquitetura:    Baseline PIC                                                *
 * Processador:    16F887                                                      *
 * Compilador:     MPLAB XC8 v1.36                                             *
 *******************************************************************************
 * Arquivos:       Pseudo3859_Password.pdl                                     *
 *                                                                             *
 * Pseudo codigo                                                               *
 *******************************************************************************
 * Descricao:      Pseudo3859_Password.X                                       *
 *                                                                             *
 * Este programa simula uma interface de usuario que aguarda o fornecimento de *
 * uma senha numerica simples. Exibe em display de 7 segmentos os numeros for- *
 * necidos. Mostra uma mensagem PIC16F887, caso a senha tenha sido digitada    *
 * corretamente. Exibe mensagem de erro se o usuario forneceu a senha incorre- *
 * ta.                                                                         *
 *******************************************************************************
 * Configuracao:                                                               *
 *                                                                             *
 * PORTD.B0:B3 -> DISPLAY1:3                                                   *
 *                                                                             *
 ******************************************************************************/

#pragma config FOSC = XT          // Oscillator Selection bits
#pragma config WDTE = OFF         // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF        // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON         // RE3/MCLR pin function select bit
#pragma config CP = OFF           // Code Protection bit
#pragma config CPD = OFF          // Data Code Protection bit
#pragma config BOREN = ON         // Brown Out Reset Selection bits
#pragma config IESO = ON          // Internal External Switchover bit
#pragma config FCMEN = ON         // Fail-Safe Clock Monitor Enabled bit
#pragma config LVP = ON           // Low Voltage Programming Enable bit
#pragma config BOR4V = BOR40V     // Brown-out Reset set to 4.0V
#pragma config WRT = OFF          // Flash Program Memory Self Write Enable bits

#include <xc.h>
#include <pic16f887.h>
#include <stdint.h>


// Define portas D4:D7 do registrador REGD para o display de 7 segmentos
#define DISPLAY1        PORTDbits.RD4
#define DISPLAY2        PORTDbits.RD5
#define DISPLAY3        PORTDbits.RD6
#define DISPLAY4        PORTDbits.RD7
#define POWER_LED       PORTDbits.RD0

#define _XTAL_FREQ      20000000  // Define freq do oscilador em 20Mhz 

int main(void)
{
    ANSEL       = 0;              // Define I/O digital da PORTA
    ANSELH      = 0;              // Define I/O digital da PORTB
    C1ON        = 0;              // Desliga comparador C1
    C2ON        = 0;              // Desliga comparador C2
    TRISB       = 0;              // Define PORTB Output
    TRISD       = 0;              // Define PORTD Output
    PORTB       = 0x00;           // Inicia PORTB com 0x00
    POWER_LED   = 1;              // Define LED em RD0 sempre aceso          
    
//    uint8_t cnum_bcd_7seg = {0x01};
    
    uint8_t i;
    
    for (;;) {
        PORTB = 0xFE;
        for (i = 0; i < 4; ++i) {
            uint8_t sel = i;
            switch( sel ) {
                case 0:
                    DISPLAY1 = 1;
                    DISPLAY2 = 0;
                    DISPLAY3 = 0;
                    DISPLAY4 = 0;
                    break;
                case 1:
                    DISPLAY1 = 0;
                    DISPLAY2 = 1;
                    DISPLAY3 = 0;
                    DISPLAY4 = 0;
                    break; 
                case 2:
                    DISPLAY1 = 0;
                    DISPLAY2 = 0;
                    DISPLAY3 = 1;
                    DISPLAY4 = 0;
                    break; 
                case 3:
                    DISPLAY1 = 0;
                    DISPLAY2 = 0;
                    DISPLAY3 = 0;
                    DISPLAY4 = 1;
                    break;                   
            }
            __delay_us(10);
        }
    }
    return(0);
}