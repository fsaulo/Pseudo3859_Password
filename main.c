/* 
 * File:   main.c
 * Author: Saulo
 *
 * Created on 9 de Novembro de 2020, 02:28
 */


#pragma config FOSC = EXTRC_CLKOUT// Oscillator Selection bits (RC oscillator: CLKOUT function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
#pragma config WDTE = ON          // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF        // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON         // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF           // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF          // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON         // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = ON          // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON         // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = ON           // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)


#pragma config BOR4V = BOR40V     // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF          // Flash Program Memory Self Write Enable bits (Write protection off)

#include <xc.h>
#include <pic16f887.h>

#define _XTAL_FREQ 20000000

#define DISPLAY1        PORTD.B0
#define DISPLAY2        PORTD.B1
#define DISPLAY3        PORTD.B2
#define DISPLAY4        PORTD.B3


int main(void)
{
    ANSEL       = 0;
    ANSELH      = 0;
    C1ON        = 0;
    C2ON        = 0;
    TRISB       = 0;
    PORTB       = 0;
    
    TRISD0      = 0;
    TRISD1      = 0;
    TRISD2      = 0;
    TRISD3      = 0;
    
    PORTB       = 0xFF;
    
    // unsigned int cnum_bcd_7seg = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x83, 0xF8,
    // 0x80, 0x98};
    
    for (;;) {
        PORTB = 0x00;
        __delay_ms(500);
        PORTB = 0xFF;
        __delay_ms(500);
    }
    return(0);
}

