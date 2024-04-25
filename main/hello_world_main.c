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
#define HOLD_THRESHOLD 30

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
    // 0x00, // test 1
    // 0xFF  // test 2
};
void show_led(uint8_t data)
{
        gpio_set_level(LATCH_PIN_4094_1, 0);
        gpio_set_level(LATCH_PIN_4094_2, 0);
        shift_out(DATA_PIN_4094, CLOCK_PIN, MSBFIRST, data);
        gpio_set_level(LATCH_PIN_4094_1, 1);
        gpio_set_level(LATCH_PIN_4094_2, 1);
}
void button_task(void *arg);
typedef struct {
    int last_button;
    int status;
} ButtonData;
QueueHandle_t button_queue; // Khai báo hàng đợi để chia sẻ dữ liệu

void task1(void *arg)
{
    i2c_dev_t dev;
    memset(&dev, 0, sizeof(i2c_dev_t));
    ButtonData buttonData;
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
        // ESP_ERROR_CHECK(ds1307_adjust(&dev, 2024, 4, 24, 10, 55, 30));
    }

    // uint8_t display[8] = {0};
    uint8_t data[8] = {0};
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j <= 7; j++)
        {
            data[j] = digit[i];
            show_led(data[j]);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    bool flag = false;
    static int digit_index = 0;
    int last_button =0;
    while (1)
    {
        // int *last_button_ptr = (int *)pvParameters;
        // // Sử dụng giá trị của last_button ở đây
        // int last_button = *last_button_ptr;
        // Nhận dữ liệu từ hàng đợi
        if (xQueueReceive(button_queue, &buttonData, portMAX_DELAY) == pdPASS)
        {
            // In ra giá trị buttonData từ button_task()
            //printf("Button data: last_button = %d, status = %d\n", buttonData.last_button, buttonData.status);
            last_button = buttonData.last_button;
        }
        ds1307_get_time(&dev, &time);
        int hour = time.tm_hour;
        int minute = time.tm_min;
        int second = time.tm_sec;
        int hc = (hour / 10) % 10;
        int hv = hour % 10;
        int mc = (minute / 10) % 10;
        int mv = minute % 10;
        int sc = (second / 10) % 10;
        int sv = second % 10;

        // Hiển thị thời gian và trạng thái nút lên LED
        uint8_t data[8] = {
            digit[(last_button / 10) % 10],
            digit[last_button % 10],
            digit[(hour / 10) % 10],
            digit[hour % 10] | 0x40,
            digit[(minute / 10) % 10],
            digit[minute % 10] | 0x40,
            digit[(second / 10) % 10],
            digit[second % 10]};
        ///////////////
        static int current_index = 2;
        if (last_button == 11)
        {
            flag = true;
        }
        else if (flag)
        {
            if (last_button == 1)
            {
                digit_index++;
                if (current_index == 2)
                {
                    if (digit_index > 2)
                        digit_index = 0;
                    if (digit_index == 2 && hv > 3)
                        hv = 0;
                }
                if (current_index == 3)
                {
                    if (digit_index > 3 && hc == 2)
                    {
                        hc = 1;
                    }
                    if (digit_index < 0 && hc == 2)
                    {
                        hc = 1;
                    }
                    if (digit_index > 9)
                        digit_index = 0;
                }
                if (current_index != 3 && current_index % 2 != 0)
                {
                    if (digit_index > 9)
                        digit_index = 0;
                }
                if (current_index != 2 && current_index % 2 == 0)
                {
                    if (digit_index > 5)
                        digit_index = 0;
                }
                data[current_index] = digit[digit_index];
                switch (current_index)
                {
                case 2:
                    hour = digit_index * 10 + hv;
                    if (hour > 23)
                        hour = 00;
                    break;
                case 3:
                    hour = hc * 10 + digit_index;
                    break;
                case 4:
                    minute = digit_index * 10 + mv;
                    break;
                case 5:
                    minute = mc * 10 + digit_index;
                    break;
                case 6:
                    second = digit_index * 10 + sv;
                    break;
                case 7:
                    second = sc * 10 + digit_index;
                    break;
                }
                ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
                                              time.tm_mday, hour, minute, second));
                vTaskDelay(5);
            }
            // Bấm phím 2 để thay đổi giá trị tai vị trí con trỏ hiện tại của mảng data[] là 1 trong 10 giá trị của mảng digit[]
            if (last_button == 2)
            {
                digit_index--;
                if (current_index == 2)
                {
                    if (digit_index < 0)
                        digit_index = 2;
                    if (digit_index == 2 && hv > 3)
                        hv = 0;
                }
                if (current_index == 3)
                {
                    if (digit_index > 3 && hc == 2)
                    {
                        hc = 1;
                    }
                    if (digit_index < 0 && hc == 2)
                    {
                        hc = 1;
                    }
                    if (digit_index > 9)
                        digit_index = 0;
                }
                if (current_index % 2 != 0)
                {
                    if (digit_index < 0)
                        digit_index = 9;
                }
                if (current_index != 2 && current_index % 2 == 0)
                {
                    if (digit_index < 0)
                        digit_index = 5;
                }
                data[current_index] = digit[digit_index];
                switch (current_index)
                {
                case 2:
                    hour = digit_index * 10 + hv;
                    break;
                case 3:
                    hour = hc * 10 + digit_index;
                    break;
                case 4:
                    minute = digit_index * 10 + mv;
                    break;
                case 5:
                    minute = mc * 10 + digit_index;
                    break;
                case 6:
                    second = digit_index * 10 + sv;
                    break;
                case 7:
                    second = sc * 10 + digit_index;
                    break;
                }
                ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
                                              time.tm_mday, hour, minute, second));
                vTaskDelay(5);
            }
            // Bấm phím 3 để thay đổi vị trí con trỏ hiện tại của mảng data[]
            if (last_button == 3)
            {
                digit_index = 0;
                current_index++;
                if (current_index > 7)
                    current_index = 2;
                printf("%d\n", current_index);
                vTaskDelay(5);
            }
            // Bấm phím 4 để thay đổi vị trí con trỏ hiện tại của mảng data[]
            if (last_button == 4)
            {
                digit_index = 0;
                current_index--;
                if (current_index < 2)
                    current_index = 7;
                printf("%d\n", current_index);
                vTaskDelay(5);
            }
            if (last_button == 5)
            {
                ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
                                              time.tm_mday, hour, minute, second));
                vTaskDelay(5);
            }
            if (last_button == 6)
            {
                ESP_ERROR_CHECK(ds1307_adjust(&dev, 2014, 4, 12, 11, 11, 11));
                vTaskDelay(5);
            }
            if (last_button == 12)
            {
                flag = false;
            }
        }
        for (int i = 0; i < 8; i++)
        {
            show_led(data[i]);
        }
        vTaskDelay(10);
    }
}

QueueHandle_t button_queue; // Khai báo hàng đợi để chia sẻ dữ liệu

uint8_t read74165(ButtonData *buttonData)
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

    if (data == 0)
    {
        buttonData->status = 0;
        buttonData->last_button = 0;
    }

    return data;
}

void button_task(void *arg)
{
    ButtonData buttonData;

    gpio_set_direction(DATA_PIN_74165, GPIO_MODE_INPUT);
    gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LATCH_PIN_74165, GPIO_MODE_OUTPUT);
    gpio_set_direction(CLOCKINHIBITPIN_74165, GPIO_MODE_OUTPUT);

    gpio_set_pull_mode(DATA_PIN_74165, GPIO_PULLUP_ONLY);
    uint8_t current_data = 0;
    uint8_t last_data = 0;
    uint8_t last_button;
    TickType_t last_press = 0;
    TickType_t press_duration = 0;
    bool is_pressed = false;

    while (1)
    {
        if (current_data != 0)
            last_button = current_data;
        current_data = read74165(&buttonData);
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
                buttonData.last_button = last_button;
                buttonData.status = 1;
            }
            else
            {
                buttonData.last_button = last_button;
                buttonData.status = 2;
            }
        }
        last_data = current_data;

        // Gửi dữ liệu vào hàng đợi
        xQueueSend(button_queue, &buttonData, portMAX_DELAY);

        // vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY));
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
    // int button_result = 0; // Variable to store the result of button press
    ESP_ERROR_CHECK(i2cdev_init());
    button_queue = xQueueCreate(1, sizeof(ButtonData));
    xTaskCreate(task1, "task1", 2048, NULL, 10, NULL);
    xTaskCreate(led_task, "led_task", 2048, NULL, 10, NULL);      // Không cần truyền tham số vào led_task
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL); // Không cần truyền tham số vào button_task
}