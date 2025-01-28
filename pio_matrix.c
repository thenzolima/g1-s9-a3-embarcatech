#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/timer.h" // função do teclado

// Arquivo PIO
#include "pio_matrix.pio.h"

// Definições
#define NUM_PIXELS 25
#define NUM_FRAMES 5
#define BUZZER_PIN 17
#define OUT_PIN 7
#define ROWS 4
#define COLS 4

// Botões de interrupção
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

double desenho2[NUM_FRAMES][NUM_PIXELS] = {
    // Frame 1: quadrado vazado
    {0, 0, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 0, 0},

    // Frame 2: Quadrado preenchido
    {0, 0, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 0, 0},


    // Frame 3: Seta para cima
    {0, 0, 1, 0, 0,
     0, 1, 1, 1, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0},

    // Frame 4: Letra "C"
    {0, 0, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 0, 0},

    // Frame 5: Letra "X"
    {1, 0, 0, 0, 1,
     0, 1, 0, 1, 0,
     0, 0, 1, 0, 0,
     0, 1, 0, 1, 0,
     1, 0, 0, 0, 1},
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
     1, 0, 0, 0, 1,
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

    // Frame 5: Apenas o centro aceso
    {0, 0, 0, 0, 0,
     0, 0, 0, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 0, 0, 0,
     0, 0, 0, 0, 0},
};

double desenho5[NUM_FRAMES][NUM_PIXELS] = {
    {0, 0, 0, 0, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 1, 0, 0,
     0, 0, 0, 0, 0},

    {0, 0, 0, 0, 0,
     0, 0, 0, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 0, 0, 0,
     0, 0, 0, 0, 0},

    {1, 0, 0, 0, 1,
     0, 1, 0, 1, 0,
     0, 0, 1, 0, 0,
     0, 0, 0, 0, 0,
     0, 0, 0, 0, 0},

    {1, 0, 0, 0, 1,
     1, 1, 0, 1, 1,
     1, 0, 1, 0, 1,
     0, 0, 0, 0, 0,
     0, 0, 0, 0, 0},

    {1, 0, 0, 0, 1,
     1, 1, 0, 1, 1,
     1, 0, 1, 0, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1},
};

double desenho6[NUM_FRAMES][NUM_PIXELS] = {
    // Frame 1: Letra "T"
    {1, 1, 1, 1, 1,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0},

    // Frame 2: Letra "E"
    {1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 0,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1},

    // Frame 3: Letra "C"
    {0, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 0, 0, 0, 0,
     1, 0, 0, 0, 0,
     0, 1, 1, 1, 1},

    // Frame 4: Letra "H"
    {1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1}
};

// Rotina de interrupção
static void gpio_irq_handler(uint gpio, uint32_t events)
{
    printf("Interrupção ocorreu no pino %d, no evento %d\n", gpio, events);
    printf("HABILITANDO O MODO GRAVAÇÃO\n");
    reset_usb_boot(0, 0); // habilita o modo de gravação do microcontrolador
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
void tocar_buzzer(int frequencia, int duracao_ms) {
    int periodo_us = 500000  / frequencia;   // Período total em microssegundos
    int meio_ciclo = periodo_us / 2;        // Meio ciclo
    int ciclos = (duracao_ms * 1000) / periodo_us; // Total de ciclos para a duração

    for (int i = 0; i < ciclos; i++) {
        gpio_put(BUZZER_PIN, true);         // Liga o buzzer
        busy_wait_us(meio_ciclo);          // Aguarda meio ciclo
        gpio_put(BUZZER_PIN, false);       // Desliga o buzzer
        busy_wait_us(meio_ciclo);          // Aguarda outro meio ciclo
    }
}

void executar_acao(char tecla, uint32_t cor, PIO pio, uint sm)
{
    switch (tecla)
    {

        //Teclas A,B,C,D e #:

    case 'A': // Apagar todos os LEDs
        for (int i = 0; i < NUM_PIXELS; i++) {
            pio_sm_put_blocking(pio, sm, criar_cor(0, 0, 0));
        }
        break;

    case 'B': // Todos os LEDs na cor azul, intensidade 100%
        for (int i = 0; i < NUM_PIXELS; i++) {
            pio_sm_put_blocking(pio, sm, criar_cor(1.0, 0.0, 0.0));
        }
        break;

    case 'C': // Todos os LEDs na cor vermelha, intensidade 80%
        for (int i = 0; i < NUM_PIXELS; i++) {
            pio_sm_put_blocking(pio, sm, criar_cor(0.0, 0.8, 0.0));
        }
        break;

    case 'D': // Todos os LEDs na cor verde, intensidade 50%
        for (int i = 0; i < NUM_PIXELS; i++) {
            pio_sm_put_blocking(pio, sm, criar_cor(0.0, 0.0, 0.5));
        }
        break;

    case '#': // Todos os LEDs na cor branca, intensidade 20%
        for (int i = 0; i < NUM_PIXELS; i++) {
            pio_sm_put_blocking(pio, sm, criar_cor(0.2, 0.2, 0.2));
        }
        break;

    //Teclas A,B,C,D e #

    case '2': // Animação para o desenho2 com som variável por frame
    gpio_init(BUZZER_PIN);                // Inicializa o pino do buzzer
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);   // Define o pino como saída

    int frequencias[NUM_FRAMES] = {1000, 1500, 1800, 1200, 2000}; // Frequências por frame

    for (int i = 0; i < NUM_FRAMES; i++) {
        
        // Exibe o frame atual
        animar_matriz(desenho2[i], cor, pio, sm);// Reproduz som para o frame atual
        tocar_buzzer(frequencias[i], 250); // 250ms de duração para cada som
        
    }

    break;


    case '3': // Animação para o desenho3
        for (int i = 0; i < NUM_FRAMES; i++)
        {
            animar_matriz(desenho3[i], cor, pio, sm);
            sleep_ms(500);
        }
        break;

    case '4': // Animação para o desenho4
        for (int i = 0; i < NUM_FRAMES; i++)
        {
            animar_matriz(desenho4[i], cor, pio, sm);
            sleep_ms(500);
        }
        break;

    case '5': // Animação para o desenho5
        for (int i = 0; i < NUM_FRAMES; i++)
        {
            animar_matriz(desenho5[i], cor, pio, sm);
            sleep_ms(500);
        }
        break;

    case '6': // Animação para o desenho6
        for (int i = 0; i < NUM_FRAMES; i++)
        {
            animar_matriz(desenho6[i], cor, pio, sm);
            sleep_ms(500);
        }
        break;

    case '*': // Reseta o microcontrolador
        reset_usb_boot(0, 0);
        return;

    default:
        return; // Nenhuma ação para outras teclas
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

    // Inicializar os botões de interrupção
    gpio_init(button_0);
    gpio_set_dir(button_0, GPIO_IN);
    gpio_pull_up(button_0);

    gpio_init(button_1);
    gpio_set_dir(button_1, GPIO_IN);
    gpio_pull_up(button_1);

    // Interrupção da GPIO habilitada
    gpio_set_irq_enabled_with_callback(button_0, GPIO_IRQ_EDGE_FALL, 1, &gpio_irq_handler);

    while (true)
    {
        char tecla = escanear_teclado();
        if (tecla)
            executar_acao(tecla, 0, pio, sm);
    }
}