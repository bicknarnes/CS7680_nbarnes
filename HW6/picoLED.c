#include "pico/stdlib.h"
#include <stdio.h>

int main()
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    const uint LED_Board = 2;
    gpio_init(LED_PIN);
    gpio_init(LED_Board);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_dir(LED_Board, GPIO_OUT);
    stdio_init_all();
    while (true)
    {
        gpio_put(LED_Board, 0);
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        gpio_put(LED_Board, 1);
        sleep_ms(500);
    }
}
