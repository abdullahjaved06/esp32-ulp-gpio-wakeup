/* ESP32 ULP FSM: GPIO Rising Edge Wake-Up Example */

#include <stdio.h>
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "ulp.h"
#include "ulp_main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

#define WAKE_GPIO GPIO_NUM_0

static void configure_and_start_ulp(void);

void app_main(void)
{
    vTaskDelay(pdMS_TO_TICKS(1000));  // Initial delay for stability

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason != ESP_SLEEP_WAKEUP_ULP) {
        printf("System startup or other wakeup, initializing ULP...\n");
        configure_and_start_ulp();
    } else {
        printf("ULP wakeup: Rising edge on GPIO detected.\n");
    }

    printf("Going into deep sleep.\n\n");
    ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());
    esp_deep_sleep_start();
}

static void configure_and_start_ulp(void)
{
    esp_err_t ret = ulp_load_binary(0, ulp_main_bin_start,
                                    (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
    ESP_ERROR_CHECK(ret);

    gpio_num_t monitored_gpio = WAKE_GPIO;
    int rtcio_index = rtc_io_number_get(monitored_gpio);
    assert(rtc_gpio_is_valid_gpio(monitored_gpio) && "Invalid RTC GPIO");

    ulp_io_number = rtcio_index;

    // Prepare selected GPIO as RTC input
    rtc_gpio_init(monitored_gpio);
    rtc_gpio_set_direction(monitored_gpio, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_dis(monitored_gpio);
    rtc_gpio_pulldown_dis(monitored_gpio);
    rtc_gpio_hold_en(monitored_gpio);

    // Disable and isolate all other RTC GPIOs
    for (gpio_num_t pin = 0; pin <= 39; ++pin) {
        if (rtc_gpio_is_valid_gpio(pin) && pin != monitored_gpio) {
            rtc_gpio_init(pin);
            rtc_gpio_set_direction(pin, RTC_GPIO_MODE_DISABLED);
            rtc_gpio_pullup_dis(pin);
            rtc_gpio_pulldown_dis(pin);
            rtc_gpio_hold_en(pin);
        }
    }

#if CONFIG_IDF_TARGET_ESP32
    rtc_gpio_isolate(GPIO_NUM_12);
    rtc_gpio_isolate(GPIO_NUM_15);
#endif

    esp_deep_sleep_disable_rom_logging();

    // Set polling interval for ULP program (~200ms)
    ulp_set_wakeup_period(0, 200000);

    ret = ulp_run(&ulp_entry - RTC_SLOW_MEM);
    ESP_ERROR_CHECK(ret);
}

