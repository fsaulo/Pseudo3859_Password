/*******************************************************************************
 * File:           main.c                                                      *     
 * Author:         Saulo G. Felix                                              *
 *                                                                             *     
 * Created on 9 de Novembro de 2020, 02:28                                     * 
 *******************************************************************************
 * Arquitetura:    Baseline PIC                                                *
 * Processador:    16F887                                                      *
 * Compilador:     MPLAB XC8 v2.31                                             *
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
 * PORTD.RD4:RD7    -> DISPLAY1:3                                              *
 * PORTD.RD0        -> POWER_LED                                               *
 * 
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


// Define portas RD4:RD7 do registrador REGD para o display de 7 segmentos
#define DISPLAY1        PORTDbits.RD4
#define DISPLAY2        PORTDbits.RD5
#define DISPLAY3        PORTDbits.RD6
#define DISPLAY4        PORTDbits.RD7
#define POWER_LED       PORTDbits.RD0

// Define SW0, SW1 como variaveis de entrada usando RC2 e RC3 da PORTC
#define SW0             PORTCbits.RC2
#define SW1             PORTCbits.RC3

// Define freq do oscilador em 20Mhz
#define _XTAL_FREQ      20000000

// Prototipos das funcoes utilizadas
void cdisplay();
void sel_bcd_output(uint16_t cwait, uint8_t preg, uint16_t pit);

int main(void)
{
    ANSEL       = 0;              // Define I/O digital da PORTA
    ANSELH      = 0;              // Define I/O digital da PORTB
    TRISB       = 0;              // Define PORTB Output
    TRISD       = 0;              // Define PORTD Output
    PORTB       = 0x00;           // Inicia PORTB com 0x00
    POWER_LED   = 1;              // Define LED em RD0 sempre aceso
    C1ON        = 0;              // Desliga comparador C1
    C2ON        = 0;              // Desliga comparador C2
    
    TRISDbits.TRISD2 = 1;         // Define RD2 da PORTD Input
    TRISDbits.TRISD3 = 1;         // Define RD3 da PORTD Input
    
    uint16_t pit   = 1000;
    uint16_t ccall = 0;
    uint8_t  preg  = 0;
    uint8_t  flag  = 1;
    
    
    for (;;) {
        PORTB = 0xFE;
        
        sel_bcd_output(ccall, preg, pit);
       
        if (SW0 == 1 && flag == 1) {
            __delay_us(20);
            if (SW0 == 1) {
                preg = (preg == 3) ? 0U : preg + 1U;
                flag = 0;
            }
        }
        
        if (SW0 == 0 && flag == 0) {
                flag = 1;
            }
        
        ccall = (ccall > pit) ? 0 : ccall + 1;
    }
}

// Para limpar os displays, injetamos 5V no anodo comum
void cdisplay()
{
    DISPLAY1 = 1;
    DISPLAY2 = 1;
    DISPLAY3 = 1;
    DISPLAY4 = 1;
}

/*
 * Realiza multiplexacao dos displays. Pisca o display que esta atualmente sele-
 * cionado.
 * @args: cwait : contador de periodo de descanso
 *        preg  : apontador para o atual display selecionado
 *        pit   : numero de iteracoes de osciosidade
 */
void sel_bcd_output(uint16_t cwait, uint8_t preg, uint16_t pit) 
{
    uint8_t pdisp;
    // uint8_t cnum_bcd_7seg = {0x01, 0x4F, 0x12, 0x06, 0x4C, 0x24, 0x20, 0x0F,
    // 0x00, 0x04};
    
    // Bloco responsavel pela multiplexacao dos displays de 7 segmentos.
    // As saidas sao configuradas para intercambiar entre regioes de corte
    // e saturacao de 4 TBJs tipo PNP com funcao de ativar e desativar o
    // anodo comum de cada display com periodo de 1 ms.
    for (pdisp = 0; pdisp < 4; ++pdisp) {
        
        // Para piscar o display selecionado, pula-se o conjunto de instrucoes
        // ate o proximo descanso de 1 ms, 'wait'. 'preg' eh o apontador do
        // display selecionado
        if (cwait < (pit / 2U) && preg == pdisp) {
            goto wait;            
        }
        
        switch ( pdisp ) {
            case 0:
                DISPLAY1 = 0;
                DISPLAY2 = 1;
                DISPLAY3 = 1; 
                DISPLAY4 = 1;
                break;
            case 1:
                DISPLAY1 = 1;
                DISPLAY2 = 0;
                DISPLAY3 = 1;
                DISPLAY4 = 1;
                break; 
            case 2:
                DISPLAY1 = 1;
                DISPLAY2 = 1;
                DISPLAY3 = 0;
                DISPLAY4 = 1;
                break; 
            case 3:
                DISPLAY1 = 1;
                DISPLAY2 = 1;
                DISPLAY3 = 1;
                DISPLAY4 = 0;
                break;                   
        }
        
        wait: 
            __delay_ms(1);        // Aguarda periodo de 1 ms
            cdisplay();           // Limpa displays
    }
}