#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
#define JOY_Y 26
#define BUZZER_A 10
#define BUZZER_B 21
#define LED_G 11

// variaveis globais 
PIO pio;
uint sm;
ssd1306_t ssd;
volatile led matriz_led[LED_COUNT] = {0};
uint32_t tempo_antes = 0;
volatile uint8_t flag = 0x08;
repeating_timer_t meu_timer;
uint16_t leitura;
int16_t temperatura;

int main()
{   
    // configurações iniciais
    pio_configuracoes();
    i2c_configuracoes();
    gpio_configuracoes();
    pwm_configuracoes();
    adc_configuracoes();

    // configurações do display
    add_repeating_timer_ms(1000, timer_callback, NULL, &meu_timer);
    ssd1306_draw_string(&ssd, "MAMADEIRA", 28, 10);
    ssd1306_draw_string(&ssd, "SMART", 44, 20);
    ssd1306_draw_string(&ssd, "A START", 36, 47);
    ssd1306_pixel(&ssd, 44, 53, true);
    ssd1306_send_data(&ssd);

    char buffer[3];

    while(true)
    {   
        if (flag & 0x01)
        {   
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, "    C", 28, 28);
            ssd1306_hline(&ssd, 55, 56, 28, true);
            ssd1306_vline(&ssd, 54, 29, 30, true);
            ssd1306_hline(&ssd, 55, 56, 31, true);
            ssd1306_vline(&ssd, 57, 29, 30, true);
            ssd1306_draw_string(&ssd, "B STOP", 36, 47);
            ssd1306_pixel(&ssd, 44, 53, true);
            flag &= ~(0x01);
        }
        if (flag & 0x02)
        {    
            leitura = adc_read();
            temperatura = ((float)leitura / 4095.0f) * 165.0f - 40.0f;
    
            if(temperatura < -9)
            {
                sprintf(buffer, "%d", temperatura);
                ssd1306_draw_string(&ssd, buffer, 28, 28);
                ssd1306_send_data(&ssd);
            } 
            else if (temperatura < 0)
            {
                sprintf(buffer, " %d", temperatura);
                ssd1306_draw_string(&ssd, buffer, 28, 28);
                ssd1306_send_data(&ssd);
            }
            else if (temperatura < 10)
            {   
                sprintf(buffer, "  %d", temperatura);
                ssd1306_draw_string(&ssd, buffer, 28, 28);
                ssd1306_send_data(&ssd);
            }
            else if (temperatura < 100)
            {
                sprintf(buffer, " %d", temperatura);
                ssd1306_draw_string(&ssd, buffer, 28, 28);
                ssd1306_send_data(&ssd);
                if ((flag & 0x08) && (temperatura == 36 || temperatura == 37))
                {
                    flag &= ~(0x08);
                    add_repeating_timer_ms(1000, timer_callback1, NULL, &meu_timer);
                }
            }
            else if (temperatura < 126)
            {
                sprintf(buffer, "%d", temperatura);
                ssd1306_draw_string(&ssd, buffer, 28, 28);
                ssd1306_send_data(&ssd);
            }
        }

        if (flag & 0x04)
        {
            ssd1306_fill(&ssd, false);
            add_repeating_timer_ms(1000, timer_callback, NULL, &meu_timer);
            ssd1306_draw_string(&ssd, "MAMADEIRA", 28, 10);
            ssd1306_draw_string(&ssd, "SMART", 44, 20);
            ssd1306_draw_string(&ssd, "A START", 36, 47);
            ssd1306_pixel(&ssd, 44, 53, true);
            ssd1306_send_data(&ssd);

            flag = 0x08;
        }
    }
}

/* 
*  
* Abaixo se encontram as funções auxiliares
*
*/ 

void adc_configuracoes(void)
{
    adc_init();
    adc_gpio_init(JOY_Y); 
    adc_select_input(0); 
}

void pwm_configuracoes(void)
{
    gpio_set_function(BUZZER_A, GPIO_FUNC_PWM);
    gpio_set_function(BUZZER_B, GPIO_FUNC_PWM);
    pwm_set_clkdiv(pwm_gpio_to_slice_num(BUZZER_A), 128.0);
    pwm_set_clkdiv(pwm_gpio_to_slice_num(BUZZER_B), 128.0);
}

void i2c_configuracoes(void)
{
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
}

void pio_configuracoes(void)
{
    pio = pio0; 
    set_sys_clock_khz(128000, false);
    uint offset = pio_add_program(pio, &ws2812b_program);
    sm = pio_claim_unused_sm(pio, true);
    ws2812b_program_init(pio, sm, offset, LED_PIN);
}

void gpio_configuracoes(void)
{
    gpio_init(BOT_A);
    gpio_init(BOT_B);
    gpio_init(LED_G);
    gpio_set_dir(BOT_A, GPIO_IN);
    gpio_set_dir(BOT_B, GPIO_IN);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_pull_up(BOT_A);
    gpio_pull_up(BOT_B);
    gpio_set_irq_enabled_with_callback(BOT_A, GPIO_IRQ_EDGE_FALL, true, gpio_callback);
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

bool timer_callback1(repeating_timer_t *rt)
{   
    static bool flag1 = true;

    gpio_set_function(BUZZER_A, GPIO_FUNC_PWM);
    gpio_put(BUZZER_A, true);
    gpio_set_function(BUZZER_B, GPIO_FUNC_PWM);
    gpio_put(BUZZER_B, true);

    if(temperatura != 36 && temperatura != 37)
    {   
        gpio_set_function(BUZZER_A, GPIO_FUNC_SIO);
        gpio_put(BUZZER_A, false);
        gpio_set_function(BUZZER_B, GPIO_FUNC_SIO);
        gpio_put(BUZZER_B, false);
        ssd1306_draw_string(&ssd, "         ", 28, 12);
        gpio_put(LED_G, false);
        flag |= 0x08;
        return false;
    }
    ssd1306_draw_string(&ssd, "TMP IDEAL", 28, 12);
    if (flag1)
    {
        pwm_set_wrap(pwm_gpio_to_slice_num(BUZZER_A), 6000);
        pwm_set_gpio_level(pwm_gpio_to_slice_num(BUZZER_A), 3000);
        pwm_set_enabled(pwm_gpio_to_slice_num(BUZZER_A), true);
        pwm_set_wrap(pwm_gpio_to_slice_num(BUZZER_B), 6000);
        pwm_set_gpio_level(pwm_gpio_to_slice_num(BUZZER_B), 3000);
        pwm_set_enabled(pwm_gpio_to_slice_num(BUZZER_B), true);
        gpio_put(LED_G, true);
    }
    if (!flag1)
    {   
        gpio_put(LED_G, false);
        pwm_set_enabled(pwm_gpio_to_slice_num(BUZZER_A), false);
        pwm_set_enabled(pwm_gpio_to_slice_num(BUZZER_B), false);
    }
    flag1 = !flag1;
    return true;
}

void gpio_callback(uint gpio, uint32_t events)
{   
    uint32_t tempo_agora = to_ms_since_boot(get_absolute_time());
    if (tempo_agora - tempo_antes > 200)
    {   
        cancel_repeating_timer(&meu_timer);
        flag |= 0x03;
        gpio_set_irq_enabled(BOT_A, GPIO_IRQ_EDGE_FALL, false);
        gpio_set_irq_enabled_with_callback(BOT_B, GPIO_IRQ_EDGE_FALL, true, gpio_callback1);

        tempo_antes = tempo_agora;
    }

    gpio_acknowledge_irq(gpio, events);
}

void gpio_callback1(uint gpio, uint32_t events)
{
    uint32_t tempo_agora = to_ms_since_boot(get_absolute_time());
    if (tempo_agora - tempo_antes > 200)
    {   
        // caso haja uma interrupção quando tiver em alarme
        if (!(flag & 0x08))
        {
            gpio_set_function(BUZZER_A, GPIO_FUNC_SIO);
            gpio_put(BUZZER_A, false);
            gpio_set_function(BUZZER_B, GPIO_FUNC_SIO);
            gpio_put(BUZZER_B, false);
            ssd1306_draw_string(&ssd, "         ", 28, 12);
            gpio_put(LED_G, false);
            flag |= 0x08;
        }

        flag = 0x04;
        gpio_set_irq_enabled(BOT_B, GPIO_IRQ_EDGE_FALL, false);
        gpio_set_irq_enabled_with_callback(BOT_A, GPIO_IRQ_EDGE_FALL, true, gpio_callback);

        tempo_antes = tempo_agora;
    }

    gpio_acknowledge_irq(gpio, events);
}