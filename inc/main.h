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
bool meu_callback(repeating_timer_t*);
void gpio_configuracoes(void);

#endif