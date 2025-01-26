#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/timer.h"//função do teclado

// Arquivo PIO
#include "pio_matrix.pio.h"

// Definições
#define NUM_PIXELS 25
#define NUM_FRAMES 5
#define BUZZER_PIN 17
#define OUT_PIN 7
#define ROWS 4
#define COLS 4

//botão de interupção
const uint button_0 = 5;
const uint button_1 = 6;

// Declaração das matrizes do teclado matricial
const uint colunas[COLS] = {21, 20, 19, 18};
const uint linhas[ROWS] = {28, 27, 26, 22};
const char teclas[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

    

// Vetores de animação
double desenho_teste[NUM_FRAMES][NUM_PIXELS] = {
    {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1},
};


double desenho3[NUM_FRAMES][NUM_PIXELS] = {
    // Frame 1: Visualização de letra G
    {1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},

    // Frame 2: Visualização da letra P
      {1, 1, 1, 1, 1,
       1, 0, 0, 0, 1,
       1, 1, 1, 1, 1,
       0, 0, 0, 0, 1,
       1, 0, 0, 0, 0},

    // Frame 3: Visualização da letra I
    {0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0},

    // Frame 4: Visualização da letra O
    {1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},

    // Frame 5: Visualizção da letra U
    {1, 0, 0, 0, 1, 
    1, 0, 0, 0, 1, 
    1, 0, 0, 0, 1, 
    1, 0 , 0, 0, 1, 
    1, 1, 1, 1, 1},
};

double desenho4[NUM_FRAMES][NUM_PIXELS] = {
    // Frame 1: Linha do meio acesa
    {0, 0, 0, 0, 0,
     0, 0, 0, 0, 0,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 0,
     0, 0, 0, 0, 0},

    // Frame 2: Expansão para cima e para baixo
    {0, 0, 0, 0, 0,
     0, 1, 1, 1, 0,
     1, 1, 1, 1, 1,
     0, 1, 1, 1, 0,
     0, 0, 0, 0, 0},

    // Frame 3: Tudo aceso
    {1, 1, 1, 1, 1,
     1, 1, 1, 1, 1,
     1, 1, 1, 1, 1,
     1, 1, 1, 1, 1,
     1, 1, 1, 1, 1},

    // Frame 4: Contração para o meio
    {0, 0, 0, 0, 0,
     0, 1, 1, 1, 0,
     1, 1, 1, 1, 1,
     0, 1, 1, 1, 0,
     0, 0, 0, 0, 0},
};

//rotina da interrupção
static void gpio_irq_handler(uint gpio, uint32_t events){
    printf("Interrupção ocorreu no pino %d, no evento %d\n", gpio, events);
    printf("HABILITANDO O MODO GRAVAÇÃO");
	reset_usb_boot(0,0); //habilita o modo de gravação do microcontrolador
}

// Funções auxiliares
void init_pinos()
{
    for (int l = 0; l < ROWS; l++)
    {
        gpio_init(linhas[l]);
        gpio_set_dir(linhas[l], GPIO_OUT);
        gpio_put(linhas[l], true);
    }

    for (int c = 0; c < COLS; c++)
    {
        gpio_init(colunas[c]);
        gpio_set_dir(colunas[c], GPIO_IN);
        gpio_pull_up(colunas[c]);
    }
}

char escanear_teclado()
{
    for (int row = 0; row < ROWS; row++)
    {
        gpio_put(linhas[row], false);

        for (int col = 0; col < COLS; col++)
        {
            if (!gpio_get(colunas[col]))
            {
                sleep_ms(20);
                if (!gpio_get(colunas[col]))
                {
                    gpio_put(linhas[row], true);
                    return teclas[row][col];
                }
            }
        }
        gpio_put(linhas[row], true);
    }
    return 0;
}

uint32_t criar_cor(double b, double r, double g)
{
    return ((uint8_t)(g * 255) << 24) | ((uint8_t)(r * 255) << 16) | ((uint8_t)(b * 255) << 8);
}

void animar_matriz(double *desenho, uint32_t cor, PIO pio, uint sm)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        uint32_t valor = criar_cor(desenho[NUM_PIXELS - 1 - i], 0, 0);
        pio_sm_put_blocking(pio, sm, valor);
    }
}

void executar_acao(char tecla, uint32_t cor, PIO pio, uint sm)
{
    double r = 0, g = 0, b = 0;

    switch (tecla)
    {
    case '1': r = 0; g = 0; b = 1; break; // Azul
    case '2': r = 1; g = 0; b = 0; break; // Vermelho
    case '3': r = 0; g = 1; b = 0; break; // Verde
    case '4': r = 0.5; g = 0.5; b = 0.5; break; // Branco
    case '*': reset_usb_boot(0, 0); return;
    default: return;
    }

    for (int i = 0; i < NUM_FRAMES; i++)
    {
        animar_matriz(tecla == '3' ? desenho3[i] : desenho_teste[i], cor, pio, sm);
        sleep_ms(500);
    }
}

// Função principal
int main()
{
    stdio_init_all();

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);

    init_pinos();

    //inicializar o botão de interrupção - GPIO5
    gpio_init(button_0);
    gpio_set_dir(button_0, GPIO_IN);
    gpio_pull_up(button_0);

    //inicializar o botão de interrupção - GPIO5
    gpio_init(button_1);
    gpio_set_dir(button_1, GPIO_IN);
    gpio_pull_up(button_1);


     //interrupção da gpio habilitada
    gpio_set_irq_enabled_with_callback(button_0, GPIO_IRQ_EDGE_FALL, 1, & gpio_irq_handler);

    while (true)
    {
        char tecla = escanear_teclado();
        if (tecla)
            executar_acao(tecla, 0, pio, sm);
    }
}
