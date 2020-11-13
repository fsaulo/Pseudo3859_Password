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
 ******************************************************************************/

// Bits de configuracao
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
#define OP_MODE         PORTDbits.RD1

// Define SW0, SW1 como variaveis de entrada usando RC2 e RC3 da PORTC
#define SW0             PORTCbits.RC0
#define SW1             PORTCbits.RC1
#define SW2             PORTCbits.RC2
#define SW3             PORTCbits.RC3
#define SW4             PORTDbits.RD0

// Define freq do oscilador em 20Mhz
#define _XTAL_FREQ      20000000

// Prototipos das funcoes utilizadas
void cdisplay();
void mux_display(uint8_t pdisp);
void chk_passwd(uint8_t* breg, uint8_t* preg);
void disp_error(uint8_t* breg);
void disp_success(uint8_t* breg);
void clearreg(uint8_t* breg);
void iohandler(uint8_t* bus, uint8_t* preg, uint8_t* nreg, uint8_t* flag);
void bcd_output(uint8_t* value, uint8_t cwait, uint8_t preg, uint8_t pit);


int main(void)
{
    ANSEL       = 0;              // Define I/O digital da PORTA
    ANSELH      = 0;              // Define I/O digital da PORTB
    TRISB       = 0;              // Define PORTB Output
    TRISD       = 0;              // Define PORTD Output
    PORTB       = 0;              // Inicia PORTB com 0x00
    C1ON        = 0;              // Desliga comparador C1
    C2ON        = 0;              // Desliga comparador C2
    
    TRISCbits.TRISC0 = 1;         // Define RC0 da PORTC Input
    TRISCbits.TRISC1 = 1;         // Define RC1 da PORTC Input
    TRISCbits.TRISC2 = 1;         // Define RC2 da PORTC Input
    TRISCbits.TRISC3 = 1;         // Define RC3 da PORTC Input
    TRISDbits.TRISD0 = 1;         // Define RD0 da PORTD Input
    
    uint8_t pit   = 200;          // Iteracoes de ociosidade
    uint8_t ccall = 0;            // Contador de iteracoes
    uint8_t preg  = 0;            // Apontador de registro
    uint8_t nreg  = 0;            // Apontador de registro
    uint8_t flag  = 1;            // Registra atuacao de botao 
       
    // Registrador onde sao armazenados os digitos fornecidos pelo usuario
    uint8_t  bus_register[4] = {0x0A, 0x0A, 0x0A, 0x0A};
    
    for (;;) {
        // Inicia displays
        bcd_output(bus_register, ccall, preg, pit);
        
        // Aguarda pelos comandos de entrada
        iohandler(bus_register, &preg, &nreg, &flag);
        
        if (preg == 4 || nreg == 4) {
            chk_passwd(bus_register, &preg);
            if (!OP_MODE) nreg = 0;
        }
        
        OP_MODE = SW4;
        
        // Alterna entre modos de operacao. 0x00 modo simples 0x01 modo avancado
        preg    = OP_MODE ? preg : 0U;
        
        // Incrementa contador ate que atinja 'pit' iteracoes
        ccall   = (ccall > pit) ? 0U : ccall + 1U;
    }
}

void chk_passwd(uint8_t* breg, uint8_t* preg) {
    uint8_t i;
    uint8_t passwd[] = {0x09, 0x05, 0x08, 0x03};
    
    for (i = 0; i < 4; ++i) {
        if (*(breg + i) != passwd[i]) {
            goto error;                   
        }
    }

    // Mostra mensagem de sucesso, pula instancia de tratamento de erro
    disp_success(breg); goto pass;

    // Usuario inseriu senha incorreta
    error: disp_error(breg);

    // Se estiver no modo avancado, reseta o apontador de registro
    pass : if (OP_MODE) *preg = 0;
}

// Preenche o registrador com o valor 0xFF, codigo que diz que a mensagem foi 
// inserida corretamente
void disp_success(uint8_t* breg)
{
    uint8_t i;
   
    for (i = 0; i < 4; ++i)
    {
        *breg = 0xFF;
        breg++;
    }
}

// Preenche o registrador com o valor 0xF7, codigo que diz que a mensagem foi 
// inserida de maneira incorreta
void disp_error(uint8_t* breg)
{
    uint8_t i;
   
    for (i = 0; i < 4; ++i)
    {
        *breg = 0xF7;
        breg++;
    }   
}

// Incrementa o valor atual do buffer, se o valor ultrapassar 9, o valor do
// registrador volta para 0
void key_fwd(uint8_t* bus, uint8_t pos)
{
    if (bus[pos] >= 9) {
        bus[pos] = 0x00;
    } else {
        bus[pos] += 1;
    }
}

// Preenche o registrador com o valor 0x0A, codigo que representa sem conteudo
void clearreg(uint8_t* breg)
{
    uint8_t i;

    for (i = 0; i < 4; ++i) {
        *breg = 0x0A;
        breg++;
    }
}

/*
 * Lida com todas as entradas e algumas saidas do sistema.
 * Possui mecanismo de deteccao de falso acionamento de botao ou sistema de 
 * deboucing.
 * @args: bus  -> buffer atual
 *        preg -> apontador para a posicao no buffer
 *        nreg -> apontador especial
 *        flag -> sinaliza botao pressionado e bloqueia outras entradas
 */
void iohandler(uint8_t* bus, uint8_t* preg, uint8_t* nreg, uint8_t* flag)
{
    
    // Incrementa o conteudo do buffer na posicao indicado por preg
    if (SW2 == 1U && *flag == 1) {
        __delay_us(50);
        if (SW2 == 1) {
            key_fwd(bus, *preg);
            *flag = 0;
        }
    }    
    
    // Limpa o conteudo do buffer e reseta as posicoes em preg e nreg
    // fazendo o sistema voltar ao estado inicial
    if (SW3 == 1U && *flag == 1) {
        __delay_us(50);
        if (SW3 == 1) {
            clearreg(bus);
            *preg = 0;
            *flag = 0;
            *nreg = 0;
        }
    }
    
    // Desbloqueia o registrador de entradas para receber novos comandos caso
    // o botao tenha sido pressionado e depois solto
    if (SW0 == 0 && SW1 == 0 && SW2 == 0  && SW3 == 0 && *flag == 0) {
        *flag = 1U;
    }
    
    uint8_t pp; // variavel auxiliar
    
    // Verifica se esta atualmente recebendo comandos do tipo avancado ou
    // comandos simplificados
    switch ( OP_MODE ) {
        
        // Modo simplificado: os numeros se deslocam da direita para a esquerda
        // a medida que forem sendo inseridos
        case 0:
            if (SW0 == 1U && *flag == 1) {      
                __delay_us(50);
                if (SW0 == 1) {
                    
                    for (pp = 3; pp >= 1; --pp) {
                        if (*nreg < 3)
                            bus[pp] = bus[pp - 1];
                    }

                    *nreg += 1;
                    *flag = 0;
                }
            }
            break;
            
        // Modo avancado: movimenta um cursor no qual eh possivel incrementar o
        // valor da posicao do cursor
        case 1:
            if (SW0 == 1U && *flag == 1) {
                __delay_us(50);
                if (SW0 == 1) {
                    *preg = (*preg > 3) ? 0U : *preg + 1U;
                    *flag = 0;     
                }
            }
            
            if (SW1 == 1U && *flag == 1) {
                __delay_us(50);
                if (SW1 == 1) {
                    *preg = (*preg == 0) ? 3U : *preg - 1U;
                    *flag = 0;     
                }
            }
            break;
    }
}

// Limpa os displays. Injeta-se 5V no anodo comum
void cdisplay()
{
    DISPLAY1 = 1;
    DISPLAY2 = 1;
    DISPLAY3 = 1;
    DISPLAY4 = 1;
}

/*
 * Realiza multiplexacao dos displays. Pisca o display que esta atualmente sele-
 * cionado. Exibe valor fornecido em 'value'. Apresenta 3 opcoes de funcionamen-
 * to para o display: 0xFF -> Mensagem de sucess; 0xF7 -> Mensagem de erro e
 * para qualquer outro 'value' exibe o decimal correspondente.
 * @args: cwait : contador de periodo de descanso
 *        preg  : apontador para o atual display selecionado
 *        pit   : numero de iteracoes de osciosidade
 */
void bcd_output(uint8_t* value, uint8_t cwait, uint8_t preg, uint8_t pit)
{
    uint8_t pdisp;
    uint16_t sbus;
    uint8_t func = *value;
    
    // Armazena os valores em hexadecimal que representam caracteres em bcd
    // num_bcd_7seg  -> numerico bcd de 0-9
    // num_pass_7seg -> alfa numerico bcd escrevem a palavra PIC16F887
    // num_erro_7seg -> alfa numerico bcd escrevem a palavra ErrO
    // aux_bus       -> apaga todos os leds do display
    uint8_t num_bcd_7seg[]  = { 0x01, 0x4F, 0x12, 0x06, 0x4C, 0x24, 0x20, 0x0F, 
                                0x00, 0x04, 0xFE, 0x7E };
    uint8_t num_pass_7seg[] = { 0x18, 0x4F, 0x31, 0x4F, 0x20, 0x38, 0x00, 0x00,
                                0x0F, 0x7F, 0x7F, 0x7F };
    uint8_t num_erro_7seg[] = { 0x01, 0x7A, 0x7A, 0x30 };
    uint8_t aux_bus[4]      = { 0x18, 0x7F, 0x7F, 0x7F };
    
    uint8_t t;              // Variavel auxiliar
    uint8_t pr = 0;         // Varialve auxiliar
    uint8_t li = 0;         // Variavel auxiliar
    
    // Bloco responsavel pela multiplexacao dos displays de 7 segmentos.
    // As saidas sao configuradas para intercambiar entre regioes de corte
    // e saturacao de 4 TBJs tipo PNP com funcao de ativar e desativar o
    // anodo comum de cada display com periodo de 500 us.
    switch(func) {
        
        // Envia para os displays a palavra PIC16F887, realiza a multiplexacao
        // entre os displays e desloca cada letra para direta
        case 0xFF:
            for (sbus = 0; sbus < 50*12; ++sbus) {
                if (SW3 == 1) break;
                
                // Conta 50 periodos de 800 us e desloca o conteudo da bus auxi-
                // liar para esquerda. A primeira posicao da bus recebe uma le-
                // tra da palavra PIC16F887 de acordo com o apontador de posicao
                if (pr > 50) {
                    li++;
                    if (li >= 12) li = 0;
                    pr = 0;
                    
                    for (t = 3; t >= 1; --t) {
                        aux_bus[t] = aux_bus[t - 1U];
                    }
                    aux_bus[0] = num_pass_7seg[li];
                }
                  
                // Responsavel por intercalar entre os displays
                for (pdisp = 0; pdisp < 4; ++pdisp) {
                    PORTB = aux_bus[pdisp];
                    mux_display(pdisp);
                    __delay_us(800);
                }
                pr++;
            }

            break;
            
        // Envia para os displays a palavra ErrO, realiza a multplexacao entre
        // os displays e pisca cada letra em intervalos de aproximadamente 500ms
        case 0xF7:
            
            for (li = 0; li < 2; ++li) {
                
                // Responsavel pela multiplexacao dos displays
                for (pdisp = 0; pdisp < 4; ++pdisp) {
                    mux_display(pdisp);
                    PORTB = num_erro_7seg[pdisp];
                    sbus++;
                    __delay_us(500); 

                }
                
                // Conta 500 periodos de aproximadamente 500 us e desliga todos
                // os leds por 500 ms
                if (sbus > 500) {
                    PORTB = 0x7F;
                    cdisplay();
                    __delay_ms(500);
                    sbus = 0;
                }
            }
            
            break;
        
        // Envia numeros de 0-9 para os displays de acordo com o valor que rece-
        // be do buffer. Esse valor determina a posicao a ser acessada do vetor
        // 'num_bcd_7seg', que contem os valores em bcd
        default:
            for (pdisp = 0; pdisp < 4; ++pdisp) {
        
                PORTB = num_bcd_7seg[*value];

                // Incrementa endereco para o proximo conteudo do registrador
                *value++;

                // Para piscar o display selecionado, pula-se o conjunto de ins-
                // trucoes ate o proximo descanso de 500 us, 'wait'. 'preg' eh o
                // apontador do display selecionado.
                if (cwait < (pit / 2U) && preg == pdisp) {
                    goto wait;            
                }

                mux_display(pdisp);

                wait:
                    __delay_us(500);        // Aguarda periodo de 500 us
                    cdisplay();             // Limpa displays
        }
            break;
    }
}

// Bloco responsavel por intercalar entre os displays, o display atualmente li-
// gado e determiado pelo apontador 'pdisp'
void mux_display(uint8_t pdisp) {
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
}