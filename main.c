#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "ws2812b.pio.h"
#include "inc/ssd1306.h"
#include "inc/main.h"

#define LED_PIN 7
#define LED_COUNT 25
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define BOT_A 5
#define BOT_B 6

PIO pio;
uint sm;
ssd1306_t ssd;
volatile led matriz_led[LED_COUNT] = {0};
volatile uint8_t flag;

int main()
{   
    // configurações para a PIO que controla a matriz de leds
    pio = pio0; 
    set_sys_clock_khz(128000, false);
    uint offset = pio_add_program(pio, &ws2812b_program);
    sm = pio_claim_unused_sm(pio, true);
    ws2812b_program_init(pio, sm, offset, LED_PIN);

    // configurações para o i2c que controla a tela OLED
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);

    repeating_timer_t meu_timer;
    add_repeating_timer_ms(1000, timer_callback, NULL, &meu_timer);
    ssd1306_draw_string(&ssd, "A INICIAR", 28, 20);
    ssd1306_draw_string(&ssd, "B PARAR", 28, 36);
    ssd1306_pixel(&ssd, 36, 26, true);
    ssd1306_pixel(&ssd, 36, 42, true);
    ssd1306_send_data(&ssd);

    gpio_configuracoes();

    while(true)
    {   
        if(flag == 0b10)
        {
            
        } 
        else if(flag == 0b01)
        {

        }

        flag = 0;
    }
}

void gpio_configuracoes(void){
    gpio_init(BOT_A);
    gpio_init(BOT_B);
    gpio_set_dir(BOT_A, GPIO_IN);
    gpio_set_dir(BOT_B, GPIO_IN);
    gpio_pull_up(BOT_A);
    gpio_pull_up(BOT_B);
    gpio_set_irq_enabled_with_callback(BOT_A, GPIO_IRQ_EDGE_FALL, true, gpio_callback);
    gpio_set_irq_enabled(BOT_B, GPIO_IRQ_EDGE_FALL, true);
}

// função que une os dados binários dos LEDs para emitir para a PIO
uint32_t valor_rgb(uint8_t B, uint8_t R, uint8_t G)
{
  return (G << 24) | (R << 16) | (B << 8);
}

// função que envia dados ao buffer de leds
void set_led(uint8_t indice, uint8_t r, uint8_t g, uint8_t b)
{
    if(indice < LED_COUNT){
    matriz_led[indice].R = r;
    matriz_led[indice].G = g;
    matriz_led[indice].B = b;
    }
}

// função que limpa o buffer de leds
void clear_leds(void)
{
    for(uint8_t i = 0; i < LED_COUNT; i++){
        matriz_led[i].R = 0;
        matriz_led[i].B = 0;
        matriz_led[i].G = 0;
    }
}

// função que manda os dados do buffer para os LEDs
void print_leds(void)
{
    uint32_t valor;
    for(uint8_t i = 0; i < LED_COUNT; i++){
        valor = valor_rgb(matriz_led[i].B, matriz_led[i].R,matriz_led[i].G);
        pio_sm_put_blocking(pio, sm, valor);
    }
}

bool timer_callback(repeating_timer_t *rt)
{
    static bool piscar = true;
    
    ssd1306_rect(&ssd, 3, 3, 121, 57, piscar, false);
    ssd1306_rect(&ssd, 4, 4, 119, 55, piscar, false);

    ssd1306_rect(&ssd, 1, 1, 126, 62, !piscar, false);
    ssd1306_rect(&ssd, 2, 2, 124, 60, !piscar, false);

    ssd1306_send_data(&ssd);

    piscar = !piscar;

    return true;
}

void gpio_callback(uint gpio, uint32_t events)
{   
    static uint32_t tempo_antes = 0;
    uint32_t tempo_agora = to_ms_since_boot(get_absolute_time());
    if (tempo_agora - tempo_antes > 200)
    {
        if(gpio == 5)
        {
            flag = 0b10;
        }
        else 
        {
            flag = 0b01;
        }

        tempo_antes = tempo_agora;
    }

    gpio_acknowledge_irq(gpio, events);
}