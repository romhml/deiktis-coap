#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "deiktis.h"

#define GPIO_GREEN_LED 21
#define GPIO_YELLOW_LED 19
#define GPIO_ORANGE_LED 18
#define GPIO_RED_LED 5

#define N_LED 4

const int LEDS[N_LED] = { GPIO_GREEN_LED, GPIO_YELLOW_LED, GPIO_ORANGE_LED, GPIO_RED_LED };
const char* LED_NAMES[N_LED] = { "GREEN", "YELLOW", "ORANGE", "RED" };

void init_leds(void) {
    for (int i=0; i < N_LED; i++) {
        gpio_pad_select_gpio(LEDS[i]);
        /* Set the GPIO as a push/pull output */
        gpio_set_direction(LEDS[i], GPIO_MODE_OUTPUT);
    }
}

void clear_leds(void) {
    for (int i=0; i < N_LED; i++) {
        gpio_set_level(LEDS[i], 0);
    }
}

void lights_up(int index) {
    clear_leds();

    if (index >= 0) {
        int led_index = (index < N_LED) ? index : N_LED - 1;

        printf("Turning on the %s LED\n", LED_NAMES[led_index]);
        gpio_set_level(LEDS[led_index], 1);
    } else {
        clear_leds();
        printf("Turning LED Off");
    }
}
