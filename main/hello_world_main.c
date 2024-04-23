#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <string.h>
#include "ds1307.h"

#define MSBFIRST 1
#define DATA_PIN_74165 32
#define CLOCK_PIN 27
#define LATCH_PIN_74165 33
#define CLOCKINHIBITPIN_74165 25

#define BLINK_GPIO 12

#define DATA_PIN_4094 19
#define LATCH_PIN_4094_1 21
#define LATCH_PIN_4094_2 14

#define DEBOUNCE_DELAY 10
#define HOLD_THRESHOLD 20

#define NUM_BUTTONS 12

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

uint8_t reverse_bits(uint8_t inval, int bits)
{
    if (bits > 0)
    {
        bits--;
        return (reverse_bits(inval >> 1, bits) | ((inval & 1) << bits));
    }
    return 0;
}

esp_err_t ds1307_adjust(i2c_dev_t *dev, int year, int month, int day, int hour, int minute, int second)
{
    if (!dev)
    {
        return ESP_ERR_INVALID_ARG;
    }

    struct tm time = {
        .tm_year = year - 1900,
        .tm_mon = month,
        .tm_mday = day,
        .tm_hour = hour,
        .tm_min = minute,
        .tm_sec = second};

    ESP_ERROR_CHECK(ds1307_set_time(dev, &time));

    return ESP_OK;
}

void shift_out(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        if (bitOrder == MSBFIRST)
            gpio_set_level(dataPin, !!(val & (1 << (7 - i))));
        else
            gpio_set_level(dataPin, !!(val & (1 << i)));

        gpio_set_level(clockPin, 1);
        gpio_set_level(clockPin, 0);
    }
}

uint8_t read74165()
{
    gpio_set_level(CLOCKINHIBITPIN_74165, 1);
    gpio_set_level(LATCH_PIN_74165, 0);
    gpio_set_level(LATCH_PIN_74165, 1);
    gpio_set_level(CLOCKINHIBITPIN_74165, 0);

    uint8_t data = 0;
    for (int i = 0; i < 8; i++)
    {
        gpio_set_level(CLOCK_PIN, 1);
        gpio_set_level(CLOCK_PIN, 0);

        bitWrite(data, 7 - i, gpio_get_level(DATA_PIN_74165));
    }
    gpio_set_level(CLOCKINHIBITPIN_74165, 1);
    data = reverse_bits(data, 8) >> 3;
    return data;
}
void show_led(uint8_t display[8])
{
    const uint8_t digit[10] = {
        0xB7, // 0
        0x11, // 1
        0x3E, // 2
        0x3B, // 3
        0x99, // 4
        0xAB, // 5
        0xAF, // 6
        0x31, // 7
        0xBF, // 8
        0xBB, // 9
        0x00, // test 1
        0xFF  // test 2
    };

    for (int i = 0; i < 8; ++i)
    {
        gpio_set_level(LATCH_PIN_4094_1, 0);
        gpio_set_level(LATCH_PIN_4094_2, 0);
        shift_out(DATA_PIN_4094, CLOCK_PIN, MSBFIRST, digit[display[i]]);
        gpio_set_level(LATCH_PIN_4094_1, 1);
        gpio_set_level(LATCH_PIN_4094_2, 1);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
}
void task1(void *pvParameters)
{
    i2c_dev_t dev;
    memset(&dev, 0, sizeof(i2c_dev_t));
    gpio_set_direction(DATA_PIN_74165, GPIO_MODE_INPUT);
    gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LATCH_PIN_74165, GPIO_MODE_OUTPUT);
    gpio_set_direction(CLOCKINHIBITPIN_74165, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(DATA_PIN_74165, GPIO_PULLUP_ONLY);

    gpio_set_direction(DATA_PIN_4094, GPIO_MODE_OUTPUT);
    gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LATCH_PIN_4094_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LATCH_PIN_4094_2, GPIO_MODE_OUTPUT);

    ESP_ERROR_CHECK(ds1307_init_desc(&dev, 0, 23, 22));
    ESP_ERROR_CHECK(ds1307_start(&dev, 0));
    bool running;
    ESP_ERROR_CHECK(ds1307_is_running(&dev, &running));

    struct tm time;
    if (!running)
    {
        ds1307_get_time(&dev, &time);
        ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
                                      time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec));
    }

    uint8_t display[8] = {0};
    static int last_button;
    uint8_t current_data = 0;
    uint8_t last_data = 0;
    TickType_t last_press = 0;
    TickType_t press_duration = 0;
    bool is_pressed = false;
    for (int i = 0; i <= 9; i++)
    {
        for (int j = 0; j <= 7; j++)
        {
            display[j] = i;
        }
        show_led(display);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    while (1)
    {
        // Đọc thời gian từ RTC
        ds1307_get_time(&dev, &time);
        int hour = time.tm_hour;
        int minute = time.tm_min;
        int second = time.tm_sec;

        // Đọc trạng thái nút
        if (current_data != 0)
            last_button = current_data;
        current_data = read74165();
        if (current_data && !last_data)
        {
            last_press = xTaskGetTickCount();
            is_pressed = true;
        }
        else if (!current_data && last_data && is_pressed)
        {
            press_duration = xTaskGetTickCount() - last_press;
            is_pressed = false;

            if (press_duration < HOLD_THRESHOLD)
            {
                printf("Button %d pressed\n", last_button);
            }
            else
            {
                last_button += 12;
                printf("Button %d held\n", last_button);
            }
        }
        last_data = current_data;
        // printf("Button %d\n", last_button);
        // Hiển thị thời gian và trạng thái nút lên LED
        uint8_t display[8] = {0};
        display[0] = (last_button / 10) % 10;
        display[1] = last_button % 10;
        display[2] = (hour / 10) % 10;
        display[3] = hour % 10;
        display[4] = (minute / 10) % 10;
        display[5] = minute % 10;
        display[6] = (second / 10) % 10;
        display[7] = second % 10;

        show_led(display);

        // for (int i = 0; i < 8; ++i)
        // {
        //     gpio_set_level(LATCH_PIN_4094_1, 0);
        //     gpio_set_level(LATCH_PIN_4094_2, 0);
        //     shift_out(DATA_PIN_4094, CLOCK_PIN, MSBFIRST, digit[display[i]]);
        //     gpio_set_level(LATCH_PIN_4094_1, 1);
        //     gpio_set_level(LATCH_PIN_4094_2, 1);
        // }

        // vTaskDelay(pdMS_TO_TICKS(10));
    }
}
void led_task(void *pvParameters)
{
    esp_rom_gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
   
    while (1)
    {
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(50));
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
void app_main()
{
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreate(task1, "task1", 2048, NULL, 5, NULL);
    xTaskCreate(led_task, "led_task", 2048, NULL, 10, NULL); // Không cần truyền tham số vào led_task
}
