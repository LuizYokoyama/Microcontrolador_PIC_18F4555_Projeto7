# Microcontrolador_PIC_18F4555_Projeto7
Uso do LM 35 + ADC + Leds + Serial no microcontrolador PIC 18F4555

Atividade desenvolvida na disciplina Microprocessadores e Microcontroladores da minha graduação em Engenharia de Computação.

Neste programa, o microcontrolador lê a temperatura de um determinado componente, via transdutor de temperatura LM35 e conversor AD, e liga os Leds 1 a 1, à medida que a temperatura aumenta. Para temperaturas até 20°C, o Led em D0 é o único Led ligado. Os outros Leds, de D1 até D7, são ligados 1 a 1 a cada 10°C. A partir de 120°C todos Leds piscam, de forma intermitente, em intervalos de 100 ms. A temperatura deve ser enviada via canal serial.

Para compilar o código, recomenda-se o uso da IDE MPLab-X https://www.microchip.com/en-us/development-tools-tools-and-software/mplab-x-ide
