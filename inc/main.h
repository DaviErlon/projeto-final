#ifndef MAIN_H
#define MAIN_H

typedef struct{
    uint8_t R;
    uint8_t G;
    uint8_t B;
} led;

uint32_t valor_rgb(uint8_t, uint8_t, uint8_t);
void set_led(uint8_t, uint8_t, uint8_t, uint8_t);
void clear_leds(void);
void print_leds(void);
void gpio_configuracoes(void);
void pio_configuracoes(void);
void i2c_configuracoes(void);
void pwm_configuracoes(void);
void adc_configuracoes(void);
bool timer_callback(repeating_timer_t*);
bool timer_callback1(repeating_timer_t*);
void gpio_callback(uint, uint32_t);
void gpio_callback1(uint, uint32_t);
void cores(void);

#endif