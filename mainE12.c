/*
 * LM 35 + ADC + Leds + Serial:
 *  Lê a temperatura de um determinado componente, via transdutor de temperatura LM35
 *  e conversor AD, e liga os Leds 1 a 1, à medida que a temperatura aumenta.
 * Para temperaturas até 20°C, o Led em D0 é o único Led ligado.
 * Os outros Leds, de D1 até D7, são ligados 1 a 1 a cada 10°C. 
 * A partir de 120°C todos Leds piscam, de forma intermitente, em intervalos de 100 ms.
 * A temperatura é enviada via canal serial.
 */

/* 
 * File:   mainE12.c
 * Author: Luiz Felix
 *
 * Created on 14 de Dezembro de 2020, 23:40
 */

#define _XTAL_FREQ 4000000 // Oscilador de 4 MHz

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#pragma config FOSC = HS // Oscilador externo
#pragma config WDT = OFF // Watchdog Timer desligado
#pragma config MCLRE = OFF // Master Clear desabilitado
long contador; // Variável usada na conversão AD
float temperatura; // Temperatura lida
int piscaLeds= 0; //flag que indica se deve piscar os LEDs
int LEDs = 0xAA; //valor dos bits dos LEDs D7, D5, D3 e D1 para piscar 

void setupADC(void) {
    TRISA = 0b00000001; // Habilita pino A0 como entrada

    ADCON2bits.ADCS = 0b110; // Clock do AD: Fosc/64
    ADCON2bits.ACQT = 0b010; // Tempo de aquisição: 4 Tad
    ADCON2bits.ADFM = 0b1; // Formato: à direita
    ADCON1bits.VCFG = 0b01; // Tensões de referência: Vss no Pino AN3
    ADCON0 = 0; // Seleciona o canal AN0
    ADCON0bits.ADON = 1; // Liga o ADC
}

void inicializa_RS232() {
    RCSTA = 0X90; // Habilita porta serial, recepção de 8 bits em modo continuo, assíncrono.
    int valor;
    TXSTA = 0X24; // modo assíncrono, transmissão 8 bits.
    valor = (int) (((_XTAL_FREQ / 9600) - 16) / 16); // valor para gerar o baud rate
    SPBRG = valor; // esse registrador, carregado com o ?valor? calculado, define o baud rate
    //(pois corre se o risco de ter uma interrupção escrita e leitura ao mesmo tempo)
}

void escreve(char valor) {
    TXIF = 0; // limpa flag que sinaliza envio completo.
    TXREG = valor; // Envia caractere desejado à porta serial
    while (TXIF == 0); // espera caractere ser enviado
}

void imprime(char frase[]) {
    int indice = 0; // índice da cadeia de caracteres
    int tamanho = 0;
    tamanho = strlen(frase); // tamanho total da cadeia a ser impressa
    while (indice < tamanho) { // verifica se todos foram impressos
        escreve(frase[indice]); // Chama rotina que escreve o caractere
        indice++; // incrementa índice
    }
}

void setupTmr0() {
    GIE = 1; // Habilita interrupção global
    TMR0IE = 1; // Habilita interrupção do Timer0
    
    T08BIT = 0; // Modo 16 bits
    T0CS = 0; // Fonte do clock interna
    PSA = 0; // habilita Prescaler
    T0CONbits.T0PS = 0; // Multiplicador Prescaler: 2 x 50 ms = total 100ms
    TMR0H = 0x3C; // Começa a contar de 15535
    TMR0L = 0xAF; // até 65535 (conta 50 mil vezes 1 microsegundo) vezes 2 (prescaler) = total 100ms
    TMR0ON = 1; // Liga o timer0

}

void interrupt interrupcao(void) { // Função de atendimento de interrupção
    if (TMR0IF) {
        if (piscaLeds){ // se o flag indicar que deve piscar LEDs
            LEDs= ~LEDs; //inverte os LEDs acesos
            LATD = LEDs; //manda acender os LEDs na porta
        }
        TMR0H = 0x3C; // Começa a contar de 15535
        TMR0L = 0xAF; // até 65535 (conta 50 mil vezes)
        TMR0IF = 0; // Flag do timer 0 em 0
    }
}

void main(void) {
    TRISD = 0; // Porta D com todos pinos de saída (acionamento dos Leds)
    setupTmr0();
    inicializa_RS232();
    setupADC();
    float tempAnterior = 0; // para evitar ficar repetindo o mesmo valor na serial
    char text[100]; //string de texto para enviar para a serial
    while (1) { // Inicia loop infinito
        if (temperatura >= 120)
            piscaLeds = 1;
        else piscaLeds = 0;
        ADCON0bits.GO = 1; // Inicia a conversão A/D
        while (!ADCON0bits.GO) { // Aguarda fim da conversão
        }
        contador = (ADRESH * 0x100) + ADRESL; // Transfere valor para variável
        temperatura = ((1.5 * 100 * contador) / 1023.0); // Calcula temperatura
            
        if (temperatura != tempAnterior){
            sprintf(text, "Temperatura %.1f graus Celsius \r", temperatura); //escreve a mensagem com a temperatura
            imprime(text); //imprime mensagem na serial
            tempAnterior = temperatura; //guarda a temperatura anterior para comparar se a temperatura mudou

            if (temperatura <= 20) {
                    LATD = 1; // só o pino D0 ligado
                }
                else if (temperatura >= 30 && temperatura < 40)  {
                    LATD = 3; // só os LEDs dos pinos D0 e D1 ligados
                }else if (temperatura >= 40 && temperatura < 50) {
                    LATD = 7; // só os LEDs dos pinos D0, D1 e D2 ligados
                }else if (temperatura >= 50 && temperatura < 60) {
                    LATD = 15; // só os LEDs dos pinos D0, D1, D2 e D3 ligados
                }else if (temperatura >= 60 && temperatura < 70) {
                    LATD = 0x1F; // só os LEDs dos pinos D0, D1, D2, D3 e D4 ligados
                }else if (temperatura >= 70 && temperatura < 80) {
                    LATD = 0x3F; // só os LEDs dos pinos D0, D1, D2, D3, D4 e D5 ligados
                }else if (temperatura >= 80 && temperatura < 90) {
                    LATD = 0x7F; // só os LEDs dos pinos D0, D1, D2, D3, D4, D5 e D6 ligados
                }else if (temperatura >= 90 && temperatura < 120) {
                    LATD = 0xFF; // todos os LEDs ligados
                }
        }
    }
    return;
}
