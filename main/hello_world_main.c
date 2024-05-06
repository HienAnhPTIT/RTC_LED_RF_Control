// #include <stdio.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/queue.h>
// #include <string.h>
// #include "ds1307.h"

// #define MSBFIRST 1
// #define DATA_PIN_74165 32  // GPIO dùng để đọc dữ liệu từ IC 74165
// #define CLOCK_PIN 27       // GPIO dùng để đồng bộ clock với IC 74165
// #define LATCH_PIN_74165 33 // GPIO dùng để lấy dữ liệu từ IC 74165
// #define CLOCKINHIBITPIN_74165 25

// #define DATA_PIN_4094 19
// #define LATCH_PIN_4094_1 21
// #define LATCH_PIN_4094_2 14

// #define DEBOUNCE_DELAY 10 // Thời gian chống nhiễu (milliseconds)
// #define HOLD_THRESHOLD 20 // Ngưỡng thời gian giữ nút (milliseconds)

// #define NUM_BUTTONS 12

// #define bitRead(value, bit) (((value) >> (bit)) & 0x01)
// #define bitSet(value, bit) ((value) |= (1UL << (bit)))
// #define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
// #define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
// #define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))
// uint8_t display[8] = {0};

// uint8_t reverse_bits(uint8_t inval, int bits)
// {
//     if (bits > 0)
//     {
//         bits--;
//         return (reverse_bits(inval >> 1, bits) | ((inval & 1) << bits));
//     }
//     return 0;
// }
// esp_err_t ds1307_adjust(i2c_dev_t *dev, int year, int month, int day, int hour, int minute, int second)
// {
//     // Kiểm tra tham số hợp lệ
//     if (!dev)
//     {
//         // ESP_OK(TAG, "Invalid device pointer");
//         return ESP_ERR_INVALID_ARG;
//     }

//     // Tạo struct tm từ các thông số truyền vào
//     struct tm time = {
//         .tm_year = year - 1900, // year là số năm tính từ 1900
//         .tm_mon = month,        // month là các tháng từ 1 đến 12
//         .tm_mday = day,
//         .tm_hour = hour,
//         .tm_min = minute,
//         .tm_sec = second};

//     // Cài đặt thời gian trên DS1307
//     ESP_ERROR_CHECK(ds1307_set_time(dev, &time));

//     return ESP_OK;
// }
// void shift_out(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
// {
//     uint8_t i;

//     for (i = 0; i < 8; i++)
//     {
//         if (bitOrder == MSBFIRST)
//             gpio_set_level(dataPin, !!(val & (1 << (7 - i))));
//         else
//             gpio_set_level(dataPin, !!(val & (1 << i)));

//         gpio_set_level(clockPin, 1);
//         gpio_set_level(clockPin, 0);
//     }
// }

// uint8_t read74165()
// {
//     gpio_set_level(CLOCKINHIBITPIN_74165, 1);
//     gpio_set_level(LATCH_PIN_74165, 0);
//     gpio_set_level(LATCH_PIN_74165, 1);
//     gpio_set_level(CLOCKINHIBITPIN_74165, 0);

//     // Read data from 74165
//     uint8_t data = 0;
//     for (int i = 0; i < 8; i++)
//     {
//         gpio_set_level(CLOCK_PIN, 1); // Rising edge to shift data
//         gpio_set_level(CLOCK_PIN, 0); // Falling edge

//         bitWrite(data, 7 - i, gpio_get_level(DATA_PIN_74165));
//     }
//     gpio_set_level(CLOCKINHIBITPIN_74165, 1);
//     data = reverse_bits(data, 8) >> 3;
//     return data;
// }
// static int digit_index = 0;
// int last_button; // Chuyển last_button sang kiểu int
// int year;
// int month;
// int day;
// int hour;
// int minute;
// int second;
// int hc;
// int hv;
// int mc;
// int mv;
// int sc;
// int sv;

// void button_task(void *arg)
// {
//     gpio_set_direction(DATA_PIN_74165, GPIO_MODE_INPUT);
//     gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LATCH_PIN_74165, GPIO_MODE_OUTPUT);
//     gpio_set_direction(CLOCKINHIBITPIN_74165, GPIO_MODE_OUTPUT);

//     gpio_set_pull_mode(DATA_PIN_74165, GPIO_PULLUP_ONLY);
//     uint8_t current_data = 0;
//     uint8_t last_data = 0;
//     // int last_button = 0; // Chuyển last_button sang kiểu int
//     TickType_t last_press = 0;
//     TickType_t press_duration = 0;
//     bool is_pressed = false;
//     bool flag = false;
//     while (1)
//     {
//         // Con trỏ chỉ mục cho phần tử hiện tại trong mảng data
//         static int current_index = 2;
//         if (current_data != 0)
//             last_button = current_data;
//         current_data = read74165();
//         if (current_data && !last_data)
//         {
//             last_press = xTaskGetTickCount();
//             is_pressed = true;
//         }
//         else if (!current_data && last_data && is_pressed)
//         {
//             press_duration = xTaskGetTickCount() - last_press;
//             is_pressed = false;

//             if (press_duration < HOLD_THRESHOLD)
//             {
//                 printf("Button %d pressed\n", last_button);
//             }
//             else
//             {
//                 last_button += 12; // Thêm 12 vào giá trị press
//                 printf("Button %d held\n", last_button);
//             }
//         }
//         last_data = current_data;
//         i2c_dev_t dev;
//         memset(&dev, 0, sizeof(i2c_dev_t));
//         ESP_ERROR_CHECK(ds1307_init_desc(&dev, 0, 23, 22));
//         // if(start)
//         ESP_ERROR_CHECK(ds1307_start(&dev, 0));
//         vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY));
//         if (last_button == 11)
//         {
//             // Serial.println(data[current_index]++);
//             // if (currentMillis - previousMillis >= interval)
//             // {
//             //   previousMillis = currentMillis; // Lưu thời gian hiện tại
//             //   data[current_index] = !data[current_index]
//             flag = true;
//             // Serial.println(flag);
//             // printf("%d", flag);
//         }
//         else if (flag)
//         {
//             int last_button = read74165();
//             // Bấm phím 1 để thay đổi giá trị tai vị trí con trỏ hiện tại của mảng data[] là 1 trong 10 giá trị của mảng digit[]
//             if (last_button == 1)
//             {
//                 digit_index++;
//                 if (current_index == 2)
//                 {
//                     if (digit_index > 2)
//                         digit_index = 0;
//                     // if(data[current_index] == 0x3E)
//                     // flaghour = false;
//                     if (digit_index == 2 && hv > 3)
//                         hv = 0;
//                 }
//                 if (current_index == 3)
//                 {
//                     if (digit_index > 3 && hc == 2)
//                     {
//                         hc = 1;
//                     }
//                     if (digit_index < 0 && hc == 2)
//                     {
//                         hc = 1;
//                     }
//                     if (digit_index > 9)
//                         digit_index = 0;
//                     // if(flaghour)
//                     //   hc=1;
//                 }
//                 if (current_index != 3 && current_index % 2 != 0)
//                 {
//                     if (digit_index > 9)
//                         digit_index = 0;
//                 }
//                 if (current_index != 2 && current_index % 2 == 0)
//                 {
//                     if (digit_index > 5)
//                         digit_index = 0;
//                 }
//                 display[current_index] = digit_index;
//                 // Serial.println(digit_index);
//                 switch (current_index)
//                 {
//                 case 2:
//                     hour = digit_index * 10 + hv;
//                     if (hour > 23)
//                         hour = 00;
//                     break;
//                 case 3:
//                     hour = hc * 10 + digit_index;
//                     // if(hour>=24) hour= 00;
//                     break;
//                 case 4:
//                     minute = digit_index * 10 + mv;
//                     // if(minute>=60) minute= 00;
//                     break;
//                 case 5:
//                     minute = mc * 10 + digit_index;
//                     // if(minute>=60) minute= 00;
//                     break;
//                 case 6:
//                     second = digit_index * 10 + sv;
//                     // if(second>=60) second= 00;
//                     break;
//                 case 7:
//                     second = sc * 10 + digit_index;
//                     // if(second>=60) second= 00;
//                     break;
//                 }
//                 // DateTime adjusted_time(now.year(), now.month(), now.day(), hour, minute, second);
//                 // rtc.adjust(adjusted_time);
//                 // data[current_index] = digit[8];
//                 // delay(50);
//                 ESP_ERROR_CHECK(ds1307_adjust(&dev, year + 1900, month, day, hour, minute, second));
//                 vTaskDelay(5);
//             }
//             // Bấm phím 2 để thay đổi giá trị tai vị trí con trỏ hiện tại của mảng data[] là 1 trong 10 giá trị của mảng digit[]
//             if (last_button == 2)
//             {
//                 // data[current_index] = digit[9];
//                 digit_index--;
//                 // if (digit_index < 0)
//                 //   digit_index = 9;
//                 // data[current_index] = digit[digit_index];
//                 if (current_index == 2)
//                 {
//                     if (digit_index < 0)
//                         digit_index = 2;
//                     if (digit_index == 2 && hv > 3)
//                         hv = 0;
//                 }
//                 if (current_index == 3)
//                 {
//                     if (digit_index > 3 && hc == 2)
//                     {
//                         hc = 1;
//                     }
//                     if (digit_index < 0 && hc == 2)
//                     {
//                         hc = 1;
//                     }
//                     if (digit_index > 9)
//                         digit_index = 0;
//                 }
//                 if (current_index % 2 != 0)
//                 {
//                     if (digit_index < 0)
//                         digit_index = 9;
//                 }
//                 if (current_index != 2 && current_index % 2 == 0)
//                 {
//                     if (digit_index < 0)
//                         digit_index = 5;
//                 }
//                 display[current_index] = digit_index;
//                 // Serial.println(digit_index);
//                 switch (current_index)
//                 {
//                 case 2:
//                     hour = digit_index * 10 + hv;
//                     // if(hour>=24) hour= 00;
//                     break;
//                 case 3:
//                     hour = hc * 10 + digit_index;
//                     // if(hour>=24) hour= 00;
//                     break;
//                 case 4:
//                     minute = digit_index * 10 + mv;
//                     // if(minute>=60) minute= 00;
//                     break;
//                 case 5:
//                     minute = mc * 10 + digit_index;
//                     // if(minute>=60) minute= 00;
//                     break;
//                 case 6:
//                     second = digit_index * 10 + sv;
//                     // if(second>=60) second= 00;
//                     break;
//                 case 7:
//                     second = sc * 10 + digit_index;
//                     // if(second>=60) second= 00;
//                     break;
//                 }
//                 // DateTime adjusted_time(now.year(), now.month(), now.day(), hour, minute, second);
//                 // rtc.adjust(adjusted_time);
//                 // data[current_index] = digit[8];
//                 ESP_ERROR_CHECK(ds1307_adjust(&dev, year + 1900, month, day, hour, minute, second));
//             }
//             // Bấm phím 3 để thay đổi vị trí con trỏ hiện tại của mảng data[]
//             if (last_button == 3)
//             {
//                 digit_index = 0;
//                 current_index++;
//                 if (current_index > 7)
//                     current_index = 2;
//                 // Serial.println(current_index);
//                 // delay(50);
//                 vTaskDelay(5);
//             }
//             // Bấm phím 3 để thay đổi vị trí con trỏ hiện tại của mảng data[]
//             if (last_button == 4)
//             {
//                 digit_index = 0;
//                 current_index--;
//                 if (current_index < 2)
//                     current_index = 7;
//                 // Serial.println(current_index);
//                 // delay(50);
//                 vTaskDelay(5);
//             }
//             // if (last_button == 5)
//             // {
//             //     rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Nếu không, sử dụng thời gian hiện tại
//             //     delay(50);
//             // }
//             if (last_button == 6)
//             {
//                 ESP_ERROR_CHECK(ds1307_adjust(&dev, 2014, 4, 23, 11, 11, 11));
//                 // delay(50);
//                 vTaskDelay(5);
//             }
//             if (last_button == 12)
//             {
//                 // Serial.println(data[current_index]++);
//                 // if (currentMillis - previousMillis >= interval)
//                 // {
//                 //   previousMillis = currentMillis; // Lưu thời gian hiện tại
//                 //   data[current_index] = !data[current_index]
//                 flag = false;
//                 // Serial.println(flag);
//             }
//         }
//     }
// }

// void show_led(uint8_t display[8])
// {
//     const uint8_t digit[10] = {
//         0xB7, // 0
//         0x11, // 1
//         0x3E, // 2
//         0x3B, // 3
//         0x99, // 4
//         0xAB, // 5
//         0xAF, // 6
//         0x31, // 7
//         0xBF, // 8
//         0xBB, // 9
//         0x00, // test 1
//         0xFF  // test 2
//     };

//     for (int i = 0; i < 8; ++i)
//     {
//         gpio_set_level(LATCH_PIN_4094_1, 0);
//         gpio_set_level(LATCH_PIN_4094_2, 0);
//         shift_out(DATA_PIN_4094, CLOCK_PIN, MSBFIRST, digit[display[i]]);
//         gpio_set_level(LATCH_PIN_4094_1, 1);
//         gpio_set_level(LATCH_PIN_4094_2, 1);
//     }
// }

// void led_task(void *pvParameters)
// {
//     // int lastbutton; // Khai báo lastbutton là một biến local
//     // int hour, minute, second; // Declare variables for hour, minute, and second
//     gpio_set_direction(DATA_PIN_4094, GPIO_MODE_OUTPUT);
//     gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LATCH_PIN_4094_1, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LATCH_PIN_4094_2, GPIO_MODE_OUTPUT);

//     for (int i = 0; i <= 9; i++)
//     {
//         for (int j = 0; j <= 7; j++)
//         {
//             display[j] = i;
//         }
//         show_led(display);
//         vTaskDelay(pdMS_TO_TICKS(50));
//     }
//     while (1)
//     {
//         display[0] = (last_button / 10) % 10;
//         display[1] = last_button % 10;
//         display[2] = (hour / 10) % 10;
//         display[3] = hour % 10;
//         display[4] = (minute / 10) % 10;
//         display[5] = minute % 10;
//         display[6] = (second / 10) % 10;
//         display[7] = second % 10;
//         show_led(display);
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }
// void rtc_task(void *pvParameters)
// {
//     i2c_dev_t dev;
//     memset(&dev, 0, sizeof(i2c_dev_t));
//     ESP_ERROR_CHECK(ds1307_init_desc(&dev, 0, 23, 22));
//     // if(start)
//     ESP_ERROR_CHECK(ds1307_start(&dev, 0));
//     bool running;
//     ESP_ERROR_CHECK(ds1307_is_running(&dev, &running)); // Kiểm tra xem DS1307 có đang chạy không

//     struct tm time;
//     if (!running)
//     {
//         ds1307_get_time(&dev, &time);
//         printf("heheheh");
//         // ESP_ERROR_CHECK(ds1307_adjust(&dev, 2024, 4, 17, 15, 41, 30)); // Cài đặt thời gian lên DS1307
//         ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
//                                       time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec)); // Cài đặt thời gian lên DS1307
//     }
//     while (1)
//     {
//         // Lấy thời gian từ RTC
//         ds1307_get_time(&dev, &time);
//         year = time.tm_year;
//         month = time.tm_mon;
//         day = time.tm_mday;
//         hour = time.tm_hour;
//         minute = time.tm_min;
//         second = time.tm_sec;
//         hc = (hour / 10) % 10;
//         hv = hour % 10;
//         mc = (minute / 10) % 10;
//         mv = minute % 10;
//         sc = (second / 10) % 10;
//         sv = second % 10;
//     }
// }

// void app_main()
// {
//     ESP_ERROR_CHECK(i2cdev_init());

//     xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);
//     xTaskCreate(led_task, "led_task", 2048, NULL, 10, NULL); // Không cần truyền tham số vào led_task
//     xTaskCreate(rtc_task, "rtc_task", 2048, NULL, 10, NULL); // Tạo task cho RTC
// }

//////////////////////
/////////////////////
// #include <stdio.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/queue.h>
// #include <string.h>
// #include "ds1307.h"

// #define MSBFIRST 1
// #define DATA_PIN_74165 32
// #define CLOCK_PIN 27
// #define LATCH_PIN_74165 33
// #define CLOCKINHIBITPIN_74165 25

// #define BLINK_GPIO 12

// #define DATA_PIN_4094 19
// #define LATCH_PIN_4094_1 21
// #define LATCH_PIN_4094_2 14

// #define DEBOUNCE_DELAY 10
// #define HOLD_THRESHOLD 30

// #define NUM_BUTTONS 12

// #define bitRead(value, bit) (((value) >> (bit)) & 0x01)
// #define bitSet(value, bit) ((value) |= (1UL << (bit)))
// #define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
// #define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
// #define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

// uint8_t reverse_bits(uint8_t inval, int bits)
// {
//     if (bits > 0)
//     {
//         bits--;
//         return (reverse_bits(inval >> 1, bits) | ((inval & 1) << bits));
//     }
//     return 0;
// }

// esp_err_t ds1307_adjust(i2c_dev_t *dev, int year, int month, int day, int hour, int minute, int second)
// {
//     if (!dev)
//     {
//         return ESP_ERR_INVALID_ARG;
//     }

//     struct tm time = {
//         .tm_year = year - 1900,
//         .tm_mon = month,
//         .tm_mday = day,
//         .tm_hour = hour,
//         .tm_min = minute,
//         .tm_sec = second};

//     ESP_ERROR_CHECK(ds1307_set_time(dev, &time));

//     return ESP_OK;
// }

// void shift_out(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
// {
//     uint8_t i;

//     for (i = 0; i < 8; i++)
//     {
//         if (bitOrder == MSBFIRST)
//             gpio_set_level(dataPin, !!(val & (1 << (7 - i))));
//         else
//             gpio_set_level(dataPin, !!(val & (1 << i)));

//         gpio_set_level(clockPin, 1);
//         gpio_set_level(clockPin, 0);
//     }
// }

// // uint8_t read74165()
// // {
// //     gpio_set_level(CLOCKINHIBITPIN_74165, 1);
// //     gpio_set_level(LATCH_PIN_74165, 0);
// //     gpio_set_level(LATCH_PIN_74165, 1);
// //     gpio_set_level(CLOCKINHIBITPIN_74165, 0);

// //     uint8_t data = 0;
// //     for (int i = 0; i < 8; i++)
// //     {
// //         gpio_set_level(CLOCK_PIN, 1);
// //         gpio_set_level(CLOCK_PIN, 0);

// //         bitWrite(data, 7 - i, gpio_get_level(DATA_PIN_74165));
// //     }
// //     gpio_set_level(CLOCKINHIBITPIN_74165, 1);
// //     data = reverse_bits(data, 8) >> 3;
// //     return data;
// // }
// const uint8_t digit[10] = {
//     0xB7, // 0
//     0x11, // 1
//     0x3E, // 2
//     0x3B, // 3
//     0x99, // 4
//     0xAB, // 5
//     0xAF, // 6
//     0x31, // 7
//     0xBF, // 8
//     0xBB, // 9
//     // 0x00, // test 1
//     // 0xFF  // test 2
// };
// void show_led(uint8_t data)
// {

//     // for (int i = 0; i < 8; ++i)
//     // {
//         gpio_set_level(LATCH_PIN_4094_1, 0);
//         gpio_set_level(LATCH_PIN_4094_2, 0);
//         shift_out(DATA_PIN_4094, CLOCK_PIN, MSBFIRST, data);
//         gpio_set_level(LATCH_PIN_4094_1, 1);
//         gpio_set_level(LATCH_PIN_4094_2, 1);
//     // }
//     // vTaskDelay(pdMS_TO_TICKS(10));
// }
// // static int last_button;
// int button_task(void *pvParameters);
// void task1(void *pvParameters)
// {
//     i2c_dev_t dev;
//     memset(&dev, 0, sizeof(i2c_dev_t));
//     gpio_set_direction(DATA_PIN_74165, GPIO_MODE_INPUT);
//     gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LATCH_PIN_74165, GPIO_MODE_OUTPUT);
//     gpio_set_direction(CLOCKINHIBITPIN_74165, GPIO_MODE_OUTPUT);
//     gpio_set_pull_mode(DATA_PIN_74165, GPIO_PULLUP_ONLY);

//     gpio_set_direction(DATA_PIN_4094, GPIO_MODE_OUTPUT);
//     gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LATCH_PIN_4094_1, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LATCH_PIN_4094_2, GPIO_MODE_OUTPUT);

//     ESP_ERROR_CHECK(ds1307_init_desc(&dev, 0, 23, 22));
//     ESP_ERROR_CHECK(ds1307_start(&dev, 0));
//     bool running;
//     ESP_ERROR_CHECK(ds1307_is_running(&dev, &running));

//     struct tm time;
//     if (!running)
//     {
//         ds1307_get_time(&dev, &time);
//         ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
//                                       time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec));
//         // ESP_ERROR_CHECK(ds1307_adjust(&dev, 2024, 4, 24, 10, 55, 30));
//     }

//     // uint8_t display[8] = {0};
//     uint8_t data[8] = {0};
//     for (int i = 0; i < 8; i++)
//     {
//         for (int j = 0; j <= 7; j++)
//         {
//             data[j] = digit[i];
//             show_led(data[j]);
//         }
//         vTaskDelay(pdMS_TO_TICKS(50));
//     }
//     bool flag = false;
//     static int digit_index = 0;
//     while (1)
//     {
//         // Đọc thời gian từ RTC
//         // int *result_ptr = (int *)arg;
//         // int last_button = *result_ptr;

//         int *last_button_ptr = (int *)pvParameters;
//         // Sử dụng giá trị của last_button ở đây
//         int last_button = *last_button_ptr;

//         ds1307_get_time(&dev, &time);
//         int hour = time.tm_hour;
//         int minute = time.tm_min;
//         int second = time.tm_sec;
//         int hc = (hour / 10) % 10;
//         int hv = hour % 10;
//         int mc = (minute / 10) % 10;
//         int mv = minute % 10;
//         int sc = (second / 10) % 10;
//         int sv = second % 10;

//         // Hiển thị thời gian và trạng thái nút lên LED
//         uint8_t data[8] = {
//             digit[(last_button / 10) % 10],
//             digit[last_button % 10],
//             digit[(hour / 10) % 10],
//             digit[hour % 10] | 0x40,
//             digit[(minute / 10) % 10],
//             digit[minute % 10] | 0x40,
//             digit[(second / 10) % 10],
//             digit[second % 10]};
//         ///////////////
//         static int current_index = 2;
//         if (last_button == 11)
//         {
//             // Serial.println(data[current_index]++);
//             // if (currentMillis - previousMillis >= interval)
//             // {
//             //   previousMillis = currentMillis; // Lưu thời gian hiện tại
//             //   data[current_index] = !data[current_index]
//             flag = true;
//             // Serial.println(flag);
//             // printf("%d\n", flag);
//         }
//         else if (flag)
//         {
//             if (last_button == 1)
//             {
//                 digit_index++;
//                 if (current_index == 2)
//                 {
//                     if (digit_index > 2)
//                         digit_index = 0;
//                     // if(data[current_index] == 0x3E)
//                     // flaghour = false;
//                     if (digit_index == 2 && hv > 3)
//                         hv = 0;
//                 }
//                 if (current_index == 3)
//                 {
//                     if (digit_index > 3 && hc == 2)
//                     {
//                         hc = 1;
//                     }
//                     if (digit_index < 0 && hc == 2)
//                     {
//                         hc = 1;
//                     }
//                     if (digit_index > 9)
//                         digit_index = 0;
//                     // if(flaghour)
//                     //   hc=1;
//                 }
//                 if (current_index != 3 && current_index % 2 != 0)
//                 {
//                     if (digit_index > 9)
//                         digit_index = 0;
//                 }
//                 if (current_index != 2 && current_index % 2 == 0)
//                 {
//                     if (digit_index > 5)
//                         digit_index = 0;
//                 }
//                 data[current_index] = digit[digit_index];
//                 // Serial.println(digit_index);
//                 // printf("%d\n", digit_index);
//                 switch (current_index)
//                 {
//                 case 2:
//                     hour = digit_index * 10 + hv;
//                     if (hour > 23)
//                         hour = 00;
//                     break;
//                 case 3:
//                     hour = hc * 10 + digit_index;
//                     // if(hour>=24) hour= 00;
//                     break;
//                 case 4:
//                     minute = digit_index * 10 + mv;
//                     // if(minute>=60) minute= 00;
//                     break;
//                 case 5:
//                     minute = mc * 10 + digit_index;
//                     // if(minute>=60) minute= 00;
//                     break;
//                 case 6:
//                     second = digit_index * 10 + sv;
//                     // if(second>=60) second= 00;
//                     break;
//                 case 7:
//                     second = sc * 10 + digit_index;
//                     // if(second>=60) second= 00;
//                     break;
//                 }
//                 // DateTime adjusted_time(now.year(), now.month(), now.day(), hour, minute, second);
//                 // rtc.adjust(adjusted_time);
//                 ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
//                                               time.tm_mday, hour, minute, second));
//                 // display[current_index] = 8;
//                 vTaskDelay(5);
//             }
//             // Bấm phím 2 để thay đổi giá trị tai vị trí con trỏ hiện tại của mảng data[] là 1 trong 10 giá trị của mảng digit[]
//             if (last_button == 2)
//             {
//                 // data[current_index] = digit[9];
//                 digit_index--;
//                 // if (digit_index < 0)
//                 //   digit_index = 9;
//                 // data[current_index] = digit[digit_index];
//                 if (current_index == 2)
//                 {
//                     if (digit_index < 0)
//                         digit_index = 2;
//                     if (digit_index == 2 && hv > 3)
//                         hv = 0;
//                 }
//                 if (current_index == 3)
//                 {
//                     if (digit_index > 3 && hc == 2)
//                     {
//                         hc = 1;
//                     }
//                     if (digit_index < 0 && hc == 2)
//                     {
//                         hc = 1;
//                     }
//                     if (digit_index > 9)
//                         digit_index = 0;
//                 }
//                 if (current_index % 2 != 0)
//                 {
//                     if (digit_index < 0)
//                         digit_index = 9;
//                 }
//                 if (current_index != 2 && current_index % 2 == 0)
//                 {
//                     if (digit_index < 0)
//                         digit_index = 5;
//                 }
//                 data[current_index] = digit[digit_index];
//                 // Serial.println(digit_index);
//                 // printf("%d\n", digit_index);
//                 switch (current_index)
//                 {
//                 case 2:
//                     hour = digit_index * 10 + hv;
//                     // if(hour>=24) hour= 00;
//                     break;
//                 case 3:
//                     hour = hc * 10 + digit_index;
//                     // if(hour>=24) hour= 00;
//                     break;
//                 case 4:
//                     minute = digit_index * 10 + mv;
//                     // if(minute>=60) minute= 00;
//                     break;
//                 case 5:
//                     minute = mc * 10 + digit_index;
//                     // if(minute>=60) minute= 00;
//                     break;
//                 case 6:
//                     second = digit_index * 10 + sv;
//                     // if(second>=60) second= 00;
//                     break;
//                 case 7:
//                     second = sc * 10 + digit_index;
//                     // if(second>=60) second= 00;
//                     break;
//                 }
//                 // DateTime adjusted_time(now.year(), now.month(), now.day(), hour, minute, second);
//                 // rtc.adjust(adjusted_time);
//                 // data[current_index] = digit[8];
//                 ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
//                                               time.tm_mday, hour, minute, second));
//                 // display[current_index] = 8;
//                 vTaskDelay(5);
//             }
//             // Bấm phím 3 để thay đổi vị trí con trỏ hiện tại của mảng data[]
//             if (last_button == 3)
//             {
//                 digit_index = 0;
//                 current_index++;
//                 if (current_index > 7)
//                     current_index = 2;
//                 // Serial.println(current_index);
//                 printf("%d\n", current_index);
//                 // delay(50);
//                 vTaskDelay(5);
//             }
//             // Bấm phím 3 để thay đổi vị trí con trỏ hiện tại của mảng data[]
//             if (last_button == 4)
//             {
//                 digit_index = 0;
//                 current_index--;
//                 if (current_index < 2)
//                     current_index = 7;
//                 // Serial.println(current_index);
//                 printf("%d\n", current_index);
//                 // delay(50);
//                 vTaskDelay(5);
//             }
//             if (last_button == 5)
//             {
//                 ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
//                                               time.tm_mday, hour, minute, second));
//                 vTaskDelay(5);
//             }
//             if (last_button == 6)
//             {
//                 // rtc.adjust(DateTime(2014, 4, 12, 17, 51, 0));
//                 ESP_ERROR_CHECK(ds1307_adjust(&dev, 2014, 4, 12, 11, 11, 11));
//                 vTaskDelay(5);
//             }
//             if (last_button == 12)
//             {
//                 // Serial.println(data[current_index]++);
//                 // if (currentMillis - previousMillis >= interval)
//                 // {
//                 //   previousMillis = currentMillis; // Lưu thời gian hiện tại
//                 //   data[current_index] = !data[current_index]
//                 flag = false;
//                 // Serial.println(flag);
//                 // printf("%d\n", flag);
//             }
//         }
//         ///////////////
//         for (int i = 0; i < 8; i++)
//         {
//             show_led(data[i]);
//         }
//         vTaskDelay(10);
//     }
// }
// // void button_task(void *pvParameters)
// // {
// //     uint8_t current_data = 0;
// //     uint8_t last_data = 0;
// //     TickType_t last_press = 0;
// //     TickType_t press_duration = 0;
// //     bool is_pressed = false;

// //     while (1)
// //     {
// //         // Đọc trạng thái nút
// //         if (current_data != 0)
// //             last_button = current_data;
// //         current_data = read74165();
// //         if (current_data && !last_data)
// //         {
// //             last_press = xTaskGetTickCount();
// //             is_pressed = true;
// //         }
// //         else if (!current_data && last_data && is_pressed)
// //         {
// //             press_duration = xTaskGetTickCount() - last_press;
// //             is_pressed = false;

// //             if (press_duration < HOLD_THRESHOLD)
// //             {
// //                 printf("Button %d pressed\n", last_button);
// //             }
// //             else
// //             {
// //                 last_button += 12;
// //                 printf("Button %d held\n", last_button);
// //             }
// //         }
// //         last_data = current_data;
// //         // printf("Button %d\n", last_button);
// //         vTaskDelay(10);
// //     }
// // }

// static int last_button = 0;
// int button_task(void *pvParameters)
// {
//     int *last_button_ptr = (int *)pvParameters; // Con trỏ đến biến last_button
//     uint8_t current_data = 0;
//     uint8_t last_data = 0;
//     TickType_t last_press = 0;
//     TickType_t press_duration = 0;
//     bool is_pressed = false;

//     while (1)
//     {
//         // Đọc trạng thái nút
//         gpio_set_level(CLOCKINHIBITPIN_74165, 1);
//         gpio_set_level(LATCH_PIN_74165, 0);
//         gpio_set_level(LATCH_PIN_74165, 1);
//         gpio_set_level(CLOCKINHIBITPIN_74165, 0);

//         uint8_t data = 0;
//         for (int i = 0; i < 8; i++)
//         {
//             gpio_set_level(CLOCK_PIN, 1);
//             gpio_set_level(CLOCK_PIN, 0);

//             bitWrite(data, 7 - i, gpio_get_level(DATA_PIN_74165));
//         }
//         gpio_set_level(CLOCKINHIBITPIN_74165, 1);
//         data = reverse_bits(data, 8) >> 3;

//         if (current_data != 0)
//             last_button = current_data;
//         current_data = data;
//         if (current_data && !last_data)
//         {
//             last_press = xTaskGetTickCount();
//             is_pressed = true;
//         }
//         else if (!current_data && last_data && is_pressed)
//         {
//             press_duration = xTaskGetTickCount() - last_press;
//             is_pressed = false;

//             if (press_duration < HOLD_THRESHOLD)
//             {
//                 printf("Button %d pressed\n", last_button);
//             }
//             else
//             {
//                 last_button += 12;
//                 printf("Button %d held\n", last_button);
//             }
//         }
//         last_data = current_data;
//         *last_button_ptr = last_button;
//         // printf("Button %d\n", last_button);
//         vTaskDelay(10);
//     }
//     return last_button;
// }

// /*
// // int button_task(void *pvParameters)
// // {
// //     uint8_t current_data = 0; // Dữ liệu đọc từ IC 74165
// //     uint8_t last_data = 0;    // Dữ liệu đọc từ IC 74165 ở lần đọc trước đó
// //     // uint8_t last_button = 0;  // Nút cuối cùng được nhấn
// //     TickType_t last_press = 0;
// //     TickType_t press_duration = 0;
// //     bool is_pressed = false;

// //     while (1)
// //     {
// //         current_data = read74165();

// //         if (current_data != last_data)
// //         {
// //             if (current_data && !last_data)
// //             {
// //                 // Nút được nhấn lần đầu tiên
// //                 last_press = xTaskGetTickCount();
// //                 is_pressed = true;
// //                 last_button = current_data;
// //             }
// //             else if (!current_data && last_data && is_pressed)
// //             {
// //                 // Nút được thả
// //                 press_duration = xTaskGetTickCount() - last_press;
// //                 is_pressed = false;

// //                 if (press_duration < HOLD_THRESHOLD)
// //                 {
// //                     // Nhấn một lần
// //                     printf("Button %d pressed\n", last_button);
// //                     return last_button;
// //                 }
// //                 else
// //                 {
// //                     // Giữ nút
// //                     printf("Button %d held\n", last_button);
// //                     return last_button + NUM_BUTTONS;
// //                 }
// //             }
// //         }

// //         last_data = current_data;

// //         vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY));
// //     }
// // }
// */

// void led_task(void *pvParameters)
// {
//     esp_rom_gpio_pad_select_gpio(BLINK_GPIO);
//     gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

//     while (1)
//     {
//         gpio_set_level(BLINK_GPIO, 1);
//         vTaskDelay(pdMS_TO_TICKS(50));
//         gpio_set_level(BLINK_GPIO, 0);
//         vTaskDelay(pdMS_TO_TICKS(50));
//     }
// }
// void app_main()
// {
//     int button_result = 0; // Variable to store the result of button press
//     ESP_ERROR_CHECK(i2cdev_init());
//     xTaskCreate(task1, "task1", 2048, &button_result, 5, NULL);
//     xTaskCreate(led_task, "led_task", 2048, NULL, 10, NULL);      // Không cần truyền tham số vào led_task
//     xTaskCreate(button_task, "button_task", 2048, &button_result, 5, NULL); // Không cần truyền tham số vào button_task
// }

////////////////////////////
///////////////////////////
// #include <string.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"
// #include "driver/gpio.h"

// #define DATA_PIN 32  // GPIO dùng để đọc dữ liệu từ IC 74165
// #define CLOCK_PIN 27 // GPIO dùng để đồng bộ clock với IC 74165
// #define LATCH_PIN 33 // GPIO dùng để lấy dữ liệu từ IC 74165
// #define CLOCKINHIBITPIN 25

// #define DEBOUNCE_DELAY 10 // Thời gian chống nhiễu (milliseconds)
// #define HOLD_THRESHOLD 30 // Ngưỡng thời gian giữ nút (milliseconds)

// #define NUM_BUTTONS 12

// #define bitRead(value, bit) (((value) >> (bit)) & 0x01)
// #define bitSet(value, bit) ((value) |= (1UL << (bit)))
// #define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
// #define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
// #define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

// uint8_t reverse_bits(uint8_t inval, int bits)
// {
//     if (bits > 0)
//     {
//         bits--;
//         return (reverse_bits(inval >> 1, bits) | ((inval & 1) << bits));
//     }
//     return 0;
// }

// typedef struct {
//     int last_button;
//     int status;
// } ButtonData;

// QueueHandle_t button_queue; // Khai báo hàng đợi để chia sẻ dữ liệu

// uint8_t read74165(ButtonData *buttonData)
// {
//     gpio_set_level(CLOCKINHIBITPIN, 1);
//     gpio_set_level(LATCH_PIN, 0);
//     gpio_set_level(LATCH_PIN, 1);
//     gpio_set_level(CLOCKINHIBITPIN, 0);

//     uint8_t data = 0;
//     for (int i = 0; i < 8; i++)
//     {
//         gpio_set_level(CLOCK_PIN, 1);
//         gpio_set_level(CLOCK_PIN, 0);
//         bitWrite(data, 7 - i, gpio_get_level(DATA_PIN));
//     }
//     gpio_set_level(CLOCKINHIBITPIN, 1);
//     data = reverse_bits(data, 8) >> 3;

//     if (data == 0)
//     {
//         buttonData->status = 0;
//         buttonData->last_button = 0;
//     }

//     return data;
// }

// void button_task(void *arg)
// {
//     ButtonData buttonData;

//     gpio_set_direction(DATA_PIN, GPIO_MODE_INPUT);
//     gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LATCH_PIN, GPIO_MODE_OUTPUT);
//     gpio_set_direction(CLOCKINHIBITPIN, GPIO_MODE_OUTPUT);

//     gpio_set_pull_mode(DATA_PIN, GPIO_PULLUP_ONLY);
//     uint8_t current_data = 0;
//     uint8_t last_data = 0;
//     uint8_t last_button;
//     TickType_t last_press = 0;
//     TickType_t press_duration = 0;
//     bool is_pressed = false;

//     while (1)
//     {
//         if (current_data != 0)
//             last_button = current_data;
//         current_data = read74165(&buttonData);
//         if (current_data && !last_data)
//         {
//             last_press = xTaskGetTickCount();
//             is_pressed = true;
//         }
//         else if (!current_data && last_data && is_pressed)
//         {
//             press_duration = xTaskGetTickCount() - last_press;
//             is_pressed = false;

//             if (press_duration < HOLD_THRESHOLD)
//             {
//                 buttonData.last_button = last_button;
//                 buttonData.status = 1;
//             }
//             else
//             {
//                 buttonData.last_button = last_button;
//                 buttonData.status = 2;
//             }
//         }
//         last_data = current_data;

//         // Gửi dữ liệu vào hàng đợi
//         xQueueSend(button_queue, &buttonData, portMAX_DELAY);

//         // vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY));
//     }
// }

// void task2(void *arg)
// {
//     ButtonData buttonData;

//     while (1)
//     {
//         // Nhận dữ liệu từ hàng đợi
//         if (xQueueReceive(button_queue, &buttonData, portMAX_DELAY) == pdPASS)
//         {
//             // In ra giá trị buttonData từ button_task()
//             printf("Button data: last_button = %d, status = %d\n", buttonData.last_button, buttonData.status);
//         }

//         // Delay một khoảng thời gian trước khi in lại giá trị (vd: 1000ms)
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
// }

// void app_main()
// {
//     // Tạo hàng đợi để chia sẻ dữ liệu
//     button_queue = xQueueCreate(1, sizeof(ButtonData));

//     xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);
//     xTaskCreate(task2, "task2", 2048, NULL, 10, NULL);
// }

//////////////////////////////
/////////////////////////////
// #include <stdio.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/queue.h>
// #include <string.h>
// #include "ds1307.h"

// #define MSBFIRST 1
// #define BLINK_GPIO 12

// #define CLOCK_PIN 27
// #define DATA_PIN_4094 19
// #define LATCH_PIN_4094_1 21
// #define LATCH_PIN_4094_2 14

// #define bitRead(value, bit) (((value) >> (bit)) & 0x01)
// #define bitSet(value, bit) ((value) |= (1UL << (bit)))
// #define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
// #define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
// #define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

// uint8_t reverse_bits(uint8_t inval, int bits)
// {
//     if (bits > 0)
//     {
//         bits--;
//         return (reverse_bits(inval >> 1, bits) | ((inval & 1) << bits));
//     }
//     return 0;
// }

// esp_err_t ds1307_adjust(i2c_dev_t *dev, int year, int month, int day, int hour, int minute, int second)
// {
//     if (!dev)
//     {
//         return ESP_ERR_INVALID_ARG;
//     }

//     struct tm time = {
//         .tm_year = year - 1900,
//         .tm_mon = month,
//         .tm_mday = day,
//         .tm_hour = hour,
//         .tm_min = minute,
//         .tm_sec = second};

//     ESP_ERROR_CHECK(ds1307_set_time(dev, &time));

//     return ESP_OK;
// }

// void shift_out(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
// {
//     uint8_t i;

//     for (i = 0; i < 8; i++)
//     {
//         if (bitOrder == MSBFIRST)
//             gpio_set_level(dataPin, !!(val & (1 << (7 - i))));
//         else
//             gpio_set_level(dataPin, !!(val & (1 << i)));

//         gpio_set_level(clockPin, 1);
//         gpio_set_level(clockPin, 0);
//     }
// }
// const uint8_t digit[12] = {
//     0xB7, // 0
//     0x11, // 1
//     0x3E, // 2
//     0x3B, // 3
//     0x99, // 4
//     0xAB, // 5
//     0xAF, // 6
//     0x31, // 7
//     0xBF, // 8
//     0xBB, // 9
//     0x00, // test 1
//     0xFF  // test 2
// };
// void show_led(uint8_t data)
// {
//         gpio_set_level(LATCH_PIN_4094_1, 0);
//         gpio_set_level(LATCH_PIN_4094_2, 0);
//         shift_out(DATA_PIN_4094, CLOCK_PIN, MSBFIRST, data);
//         gpio_set_level(LATCH_PIN_4094_1, 1);
//         gpio_set_level(LATCH_PIN_4094_2, 1);
// }

// void blink_at_index(uint8_t *data, int index, int blink_interval) {
//     while (1) {
//         data[index] = digit[11]; // Đặt giá trị nhấp nháy tại vị trí index
//         vTaskDelay(blink_interval); // Chờ một khoảng thời gian
//         data[index] = digit[index]; // Đặt lại giá trị ban đầu tại vị trí index
//         vTaskDelay(blink_interval); // Chờ một khoảng thời gian
//     }
// }

// void task1(void *arg)
// {
//     i2c_dev_t dev;
//     memset(&dev, 0, sizeof(i2c_dev_t));
//     gpio_set_direction(DATA_PIN_4094, GPIO_MODE_OUTPUT);
//     gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LATCH_PIN_4094_1, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LATCH_PIN_4094_2, GPIO_MODE_OUTPUT);

//     ESP_ERROR_CHECK(ds1307_init_desc(&dev, 0, 23, 22));
//     ESP_ERROR_CHECK(ds1307_start(&dev, 0));
//     bool running;
//     ESP_ERROR_CHECK(ds1307_is_running(&dev, &running));

//     struct tm time;
//     if (!running)
//     {
//         ds1307_get_time(&dev, &time);
//         ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
//                                       time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec));
//     }
//     uint8_t data[8] = {0};
//     for (int i = 0; i < 8; i++)
//     {
//         for (int j = 0; j <= 7; j++)
//         {
//             data[j] = digit[i];
//             show_led(data[j]);
//         }
//         vTaskDelay(pdMS_TO_TICKS(50));
//     }
//     xTaskCreate(blink_at_index, "blink_at_index", 2048, data, 10, NULL); // Tạo một task mới để nhấp nháy tại vị trí index trong mảng data
//     while (1)
//     {
//         ds1307_get_time(&dev, &time);
//         int hour = time.tm_hour;
//         int minute = time.tm_min;
//         int second = time.tm_sec;
//         int hc = (hour / 10) % 10;
//         int hv = hour % 10;
//         int mc = (minute / 10) % 10;
//         int mv = minute % 10;
//         int sc = (second / 10) % 10;
//         int sv = second % 10;

//         // Hiển thị thời gian và trạng thái nút lên LED
//         uint8_t data[8] = {
//             digit[0],
//             digit[0],
//             digit[(hour / 10) % 10],
//             digit[hour % 10] | 0x40,
//             digit[(minute / 10) % 10],
//             digit[minute % 10] | 0x40,
//             digit[(second / 10) % 10],
//             digit[second % 10]};
//         for (int i = 0; i < 8; i++)
//         {
//             show_led(data[i]);
//         }
//         blink_at_index( data, 2, 1000);
//         vTaskDelay(10);
//     }
// }
// void led_task(void *pvParameters)
// {
//     esp_rom_gpio_pad_select_gpio(BLINK_GPIO);
//     gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

//     while (1)
//     {
//         gpio_set_level(BLINK_GPIO, 1);
//         vTaskDelay(pdMS_TO_TICKS(50));
//         gpio_set_level(BLINK_GPIO, 0);
//         vTaskDelay(pdMS_TO_TICKS(50));
//     }
// }
// void app_main()
// {
//     ESP_ERROR_CHECK(i2cdev_init());
//     xTaskCreate(task1, "task1", 2048, NULL, 10, NULL);
//     xTaskCreate(led_task, "led_task", 2048, NULL, 10, NULL);      // Không cần truyền tham số vào led_task
// }
////////////////////////////////////
//////////////////////////////OKKKKKKKKKKKK
// #include <stdio.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/queue.h>
// #include <string.h>
// #include "ds1307.h"

// #define MSBFIRST 1
// #define DATA_PIN_74165 32
// #define CLOCK_PIN 27
// #define LATCH_PIN_74165 33
// #define CLOCKINHIBITPIN_74165 25

// #define BLINK_GPIO 12

// #define DATA_PIN_4094 19
// #define LATCH_PIN_4094_1 21
// #define LATCH_PIN_4094_2 14

// #define DEBOUNCE_DELAY 10
// #define HOLD_THRESHOLD 30

// #define NUM_BUTTONS 12

// #define bitRead(value, bit) (((value) >> (bit)) & 0x01)
// #define bitSet(value, bit) ((value) |= (1UL << (bit)))
// #define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
// #define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
// #define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

// uint8_t reverse_bits(uint8_t inval, int bits)
// {
//     if (bits > 0)
//     {
//         bits--;
//         return (reverse_bits(inval >> 1, bits) | ((inval & 1) << bits));
//     }
//     return 0;
// }

// esp_err_t ds1307_adjust(i2c_dev_t *dev, int year, int month, int day, int hour, int minute, int second)
// {
//     if (!dev)
//     {
//         return ESP_ERR_INVALID_ARG;
//     }

//     struct tm time = {
//         .tm_year = year - 1900,
//         .tm_mon = month,
//         .tm_mday = day,
//         .tm_hour = hour,
//         .tm_min = minute,
//         .tm_sec = second};

//     ESP_ERROR_CHECK(ds1307_set_time(dev, &time));

//     return ESP_OK;
// }

// void shift_out(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
// {
//     uint8_t i;

//     for (i = 0; i < 8; i++)
//     {
//         if (bitOrder == MSBFIRST)
//             gpio_set_level(dataPin, !!(val & (1 << (7 - i))));
//         else
//             gpio_set_level(dataPin, !!(val & (1 << i)));

//         gpio_set_level(clockPin, 1);
//         gpio_set_level(clockPin, 0);
//     }
// }
// const uint8_t digit[10] = {
//     0xB7, // 0
//     0x11, // 1
//     0x3E, // 2
//     0x3B, // 3
//     0x99, // 4
//     0xAB, // 5
//     0xAF, // 6
//     0x31, // 7
//     0xBF, // 8
//     0xBB, // 9
//     // 0x00, // test 1
//     // 0xFF  // test 2
// };
// void show_led(uint8_t data)
// {
//         gpio_set_level(LATCH_PIN_4094_1, 0);
//         gpio_set_level(LATCH_PIN_4094_2, 0);
//         shift_out(DATA_PIN_4094, CLOCK_PIN, MSBFIRST, data);
//         gpio_set_level(LATCH_PIN_4094_1, 1);
//         gpio_set_level(LATCH_PIN_4094_2, 1);
// }
// void button_task(void *arg);
// typedef struct {
//     int last_button;
//     int status;
// } ButtonData;
// QueueHandle_t button_queue; // Khai báo hàng đợi để chia sẻ dữ liệu

// void task1(void *arg)
// {
//     i2c_dev_t dev;
//     memset(&dev, 0, sizeof(i2c_dev_t));
//     ButtonData buttonData;
//     gpio_set_direction(DATA_PIN_74165, GPIO_MODE_INPUT);
//     gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LATCH_PIN_74165, GPIO_MODE_OUTPUT);
//     gpio_set_direction(CLOCKINHIBITPIN_74165, GPIO_MODE_OUTPUT);
//     gpio_set_pull_mode(DATA_PIN_74165, GPIO_PULLUP_ONLY);

//     gpio_set_direction(DATA_PIN_4094, GPIO_MODE_OUTPUT);
//     gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LATCH_PIN_4094_1, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LATCH_PIN_4094_2, GPIO_MODE_OUTPUT);

//     ESP_ERROR_CHECK(ds1307_init_desc(&dev, 0, 23, 22));
//     ESP_ERROR_CHECK(ds1307_start(&dev, 0));
//     bool running;
//     ESP_ERROR_CHECK(ds1307_is_running(&dev, &running));

//     struct tm time;
//     if (!running)
//     {
//         ds1307_get_time(&dev, &time);
//         ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
//                                       time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec));
//         // ESP_ERROR_CHECK(ds1307_adjust(&dev, 2024, 4, 24, 10, 55, 30));
//     }

//     // uint8_t display[8] = {0};
//     uint8_t data[8] = {0};
//     for (int i = 0; i < 8; i++)
//     {
//         for (int j = 0; j <= 7; j++)
//         {
//             data[j] = digit[i];
//             show_led(data[j]);
//         }
//         vTaskDelay(pdMS_TO_TICKS(50));
//     }
//     bool flag = false;
//     static int digit_index = 0;
//     int last_button =0;
//     while (1)
//     {
//         // int *last_button_ptr = (int *)pvParameters;
//         // // Sử dụng giá trị của last_button ở đây
//         // int last_button = *last_button_ptr;
//         // Nhận dữ liệu từ hàng đợi
//         if (xQueueReceive(button_queue, &buttonData, portMAX_DELAY) == pdPASS)
//         {
//             // In ra giá trị buttonData từ button_task()
//             //printf("Button data: last_button = %d, status = %d\n", buttonData.last_button, buttonData.status);
//             last_button = buttonData.last_button;
//         }
//         ds1307_get_time(&dev, &time);
//         int hour = time.tm_hour;
//         int minute = time.tm_min;
//         int second = time.tm_sec;
//         int hc = (hour / 10) % 10;
//         int hv = hour % 10;
//         int mc = (minute / 10) % 10;
//         int mv = minute % 10;
//         int sc = (second / 10) % 10;
//         int sv = second % 10;

//         // Hiển thị thời gian và trạng thái nút lên LED
//         static int current_index = 0;
//         uint8_t data[8] = {
//             digit[(current_index / 10) % 10],
//             digit[current_index % 10],
//             digit[(hour / 10) % 10],
//             digit[hour % 10] | 0x40,
//             digit[(minute / 10) % 10],
//             digit[minute % 10] | 0x40,
//             digit[(second / 10) % 10],
//             digit[second % 10]};
//         ///////////////
//         if (last_button == 11)
//         {
//             flag = true;
//             current_index = 2;
//         }
//         else if (flag)
//         {
//             if (last_button == 1)
//             {
//                 digit_index++;
//                 if (current_index == 2)
//                 {
//                     if (digit_index > 2)
//                         digit_index = 0;
//                     if (digit_index == 2 && hv > 3)
//                         hv = 0;
//                 }
//                 if (current_index == 3)
//                 {
//                     if (digit_index > 3 && hc == 2)
//                     {
//                         hc = 1;
//                     }
//                     if (digit_index < 0 && hc == 2)
//                     {
//                         hc = 1;
//                     }
//                     if (digit_index > 9)
//                         digit_index = 0;
//                 }
//                 if (current_index != 3 && current_index % 2 != 0)
//                 {
//                     if (digit_index > 9)
//                         digit_index = 0;
//                 }
//                 if (current_index != 2 && current_index % 2 == 0)
//                 {
//                     if (digit_index > 5)
//                         digit_index = 0;
//                 }
//                 data[current_index] = digit[digit_index];
//                 switch (current_index)
//                 {
//                 case 2:
//                     hour = digit_index * 10 + hv;
//                     if (hour > 23)
//                         hour = 00;
//                     break;
//                 case 3:
//                     hour = hc * 10 + digit_index;
//                     break;
//                 case 4:
//                     minute = digit_index * 10 + mv;
//                     break;
//                 case 5:
//                     minute = mc * 10 + digit_index;
//                     break;
//                 case 6:
//                     second = digit_index * 10 + sv;
//                     break;
//                 case 7:
//                     second = sc * 10 + digit_index;
//                     break;
//                 }
//                 ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
//                                               time.tm_mday, hour, minute, second));
//                 vTaskDelay(5);
//             }
//             // Bấm phím 2 để thay đổi giá trị tai vị trí con trỏ hiện tại của mảng data[] là 1 trong 10 giá trị của mảng digit[]
//             if (last_button == 2)
//             {
//                 digit_index--;
//                 if (current_index == 2)
//                 {
//                     if (digit_index < 0)
//                         digit_index = 2;
//                     if (digit_index == 2 && hv > 3)
//                         hv = 0;
//                 }
//                 if (current_index == 3)
//                 {
//                     if (digit_index > 3 && hc == 2)
//                     {
//                         hc = 1;
//                     }
//                     if (digit_index < 0 && hc == 2)
//                     {
//                         hc = 1;
//                     }
//                     if (digit_index > 9)
//                         digit_index = 0;
//                 }
//                 if (current_index % 2 != 0)
//                 {
//                     if (digit_index < 0)
//                         digit_index = 9;
//                 }
//                 if (current_index != 2 && current_index % 2 == 0)
//                 {
//                     if (digit_index < 0)
//                         digit_index = 5;
//                 }
//                 data[current_index] = digit[digit_index];
//                 switch (current_index)
//                 {
//                 case 2:
//                     hour = digit_index * 10 + hv;
//                     break;
//                 case 3:
//                     hour = hc * 10 + digit_index;
//                     break;
//                 case 4:
//                     minute = digit_index * 10 + mv;
//                     break;
//                 case 5:
//                     minute = mc * 10 + digit_index;
//                     break;
//                 case 6:
//                     second = digit_index * 10 + sv;
//                     break;
//                 case 7:
//                     second = sc * 10 + digit_index;
//                     break;
//                 }
//                 ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
//                                               time.tm_mday, hour, minute, second));
//                 vTaskDelay(5);
//             }
//             // Bấm phím 3 để thay đổi vị trí con trỏ hiện tại của mảng data[]
//             if (last_button == 3)
//             {
//                 digit_index = 0;
//                 current_index++;
//                 if (current_index > 7)
//                     current_index = 2;
//                 printf("%d\n", current_index);
//                 vTaskDelay(5);
//             }
//             // Bấm phím 4 để thay đổi vị trí con trỏ hiện tại của mảng data[]
//             if (last_button == 4)
//             {
//                 digit_index = 0;
//                 current_index--;
//                 if (current_index < 2)
//                     current_index = 7;
//                 printf("%d\n", current_index);
//                 vTaskDelay(5);
//             }
//             if (last_button == 5)
//             {
//                 ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
//                                               time.tm_mday, hour, minute, second));
//                 vTaskDelay(5);
//             }
//             if (last_button == 6)
//             {
//                 ESP_ERROR_CHECK(ds1307_adjust(&dev, 2014, 4, 12, 11, 11, 11));
//                 vTaskDelay(5);
//             }
//             if (last_button == 12)
//             {
//                 flag = false;
//                 current_index = 0;
//             }
//         }
//         for (int i = 0; i < 8; i++)
//         {
//             show_led(data[i]);
//         }
//         vTaskDelay(10);
//     }
// }

// QueueHandle_t button_queue; // Khai báo hàng đợi để chia sẻ dữ liệu

// uint8_t read74165(ButtonData *buttonData)
// {
//     gpio_set_level(CLOCKINHIBITPIN_74165, 1);
//     gpio_set_level(LATCH_PIN_74165, 0);
//     gpio_set_level(LATCH_PIN_74165, 1);
//     gpio_set_level(CLOCKINHIBITPIN_74165, 0);

//     uint8_t data = 0;
//     for (int i = 0; i < 8; i++)
//     {
//         gpio_set_level(CLOCK_PIN, 1);
//         gpio_set_level(CLOCK_PIN, 0);
//         bitWrite(data, 7 - i, gpio_get_level(DATA_PIN_74165));
//     }
//     gpio_set_level(CLOCKINHIBITPIN_74165, 1);
//     data = reverse_bits(data, 8) >> 3;

//     if (data == 0)
//     {
//         buttonData->status = 0;
//         buttonData->last_button = 0;
//     }

//     return data;
// }

// void button_task(void *arg)
// {
//     ButtonData buttonData;

//     gpio_set_direction(DATA_PIN_74165, GPIO_MODE_INPUT);
//     gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LATCH_PIN_74165, GPIO_MODE_OUTPUT);
//     gpio_set_direction(CLOCKINHIBITPIN_74165, GPIO_MODE_OUTPUT);

//     gpio_set_pull_mode(DATA_PIN_74165, GPIO_PULLUP_ONLY);
//     uint8_t current_data = 0;
//     uint8_t last_data = 0;
//     uint8_t last_button;
//     TickType_t last_press = 0;
//     TickType_t press_duration = 0;
//     bool is_pressed = false;

//     while (1)
//     {
//         if (current_data != 0)
//             last_button = current_data;
//         current_data = read74165(&buttonData);
//         if (current_data && !last_data)
//         {
//             last_press = xTaskGetTickCount();
//             is_pressed = true;
//         }
//         else if (!current_data && last_data && is_pressed)
//         {
//             press_duration = xTaskGetTickCount() - last_press;
//             is_pressed = false;

//             if (press_duration < HOLD_THRESHOLD)
//             {
//                 buttonData.last_button = last_button;
//                 buttonData.status = 1;
//             }
//             else
//             {
//                 buttonData.last_button = last_button;
//                 buttonData.status = 2;
//             }
//         }
//         last_data = current_data;

//         // Gửi dữ liệu vào hàng đợi
//         xQueueSend(button_queue, &buttonData, portMAX_DELAY);

//         // vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY));
//     }
// }

// void led_task(void *pvParameters)
// {
//     esp_rom_gpio_pad_select_gpio(BLINK_GPIO);
//     gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

//     while (1)
//     {
//         gpio_set_level(BLINK_GPIO, 1);
//         vTaskDelay(pdMS_TO_TICKS(50));
//         gpio_set_level(BLINK_GPIO, 0);
//         vTaskDelay(pdMS_TO_TICKS(50));
//     }
// }
// void app_main()
// {
//     // int button_result = 0; // Variable to store the result of button press
//     ESP_ERROR_CHECK(i2cdev_init());
//     button_queue = xQueueCreate(1, sizeof(ButtonData));
//     xTaskCreate(task1, "task1", 2048, NULL, 10, NULL);
//     xTaskCreate(led_task, "led_task", 2048, NULL, 10, NULL);      // Không cần truyền tham số vào led_task
//     xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL); // Không cần truyền tham số vào button_task
// }

//////////////////////////// OKKKKKKKKKKKK
////////////////////////////


////////////////////////FINAL_1///////////////////////////

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <string.h>
#include "ds1307.h"

#define MSBFIRST 1
#define DATA_PIN_74165 32
#define LATCH_PIN_74165 33
#define CLOCKINHIBITPIN_74165 25
#define BLINK_GPIO 12

#define CLOCK_PIN 27
#define DATA_PIN_4094 19
#define LATCH_PIN_4094_1 21
#define LATCH_PIN_4094_2 14

#define DATA_PIN_74165 32
#define LATCH_PIN_74165 33
#define CLOCKINHIBITPIN_74165 25

#define DEBOUNCE_DELAY 10
#define HOLD_THRESHOLD 30
#define NUM_BUTTONS 12
#define LOW 0x00
#define HIGH 0x01

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
    gpio_set_level(LATCH_PIN_4094_1, LOW);
    gpio_set_level(LATCH_PIN_4094_2, LOW);
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
    gpio_set_level(LATCH_PIN_4094_1, HIGH);
    gpio_set_level(LATCH_PIN_4094_2, HIGH);
}
uint8_t raw_data[8] = {0};

uint8_t test_led[10] = {0xF7U, 0x51U, 0x7EU, 0x7BU, 0xD9U, 0xEBU, 0xEFU, 0x71U, 0xFFU, 0xFBU};

void synch_led_7()
{
    uint8_t data = 0;
    for (int i = 0; i < 8; i++)
    {
        data = raw_data[i];
        shift_out(DATA_PIN_4094, CLOCK_PIN, MSBFIRST, data);
    }
}

void synch_test_led(uint8_t data[])
{
    int i = 0;
    while (data[i] != 0) // Sử dụng giá trị 0 để đánh dấu kết thúc của mảng
    {
        for (int j = 0; j < 8; j++)
        {
            shift_out(DATA_PIN_4094, CLOCK_PIN, MSBFIRST, data[i]);
        }
        vTaskDelay(10);
        i++;
    }
}

void convert_char7(uint8_t idx, char c)
{
    switch (c)
    {
    case '0':
        raw_data[idx] = 0xB7U; // 0
        break;
    case '1':
        raw_data[idx] = 0x11U; // 1
        break;
    case '2':
        raw_data[idx] = 0x3EU; // 2
        break;
    case '3':
        raw_data[idx] = 0x3BU; // 3
        break;
    case '4':
        raw_data[idx] = 0x99U; // 4
        break;
    case '5':
        raw_data[idx] = 0xABU; // 5
        break;
    case '6':
        raw_data[idx] = 0xAFU; // 6
        break;
    case '7':
        raw_data[idx] = 0x31U; // 7
        break;
    case '8':
        raw_data[idx] = 0xBFU; // 8
        break;
    case '9':
        raw_data[idx] = 0xBBU; // 9
        break;
    case ' ':
        raw_data[idx] = 0x00U; // Khoảng trắng
        break;
    case '-':
        raw_data[idx] = 0x08U; // Dấu gạch ngang
        break;
    case 'H':
        raw_data[idx] = 0x9DU; // H
        break;
    case 'I':
        raw_data[idx] = 0x11U; // I
        break;
    case 'E':
        raw_data[idx] = 0xAEU; // E
        break;
    case 'N':
        raw_data[idx] = 0xB5U; // N
        break;
    case 'A':
        raw_data[idx] = 0xBDU; // A
        break;
    default:
        // Xử lý khi ký tự không phù hợp
        break;
    }
}

void convert_led7(uint8_t *str)
{
    for (int i = 0; i < 8; i++)
    {
        if (str[i] == '\0')
        {
            break;
        }
        convert_char7(i, str[i]);
    }
}

void int_to_str(int button, int hour, int minute, int second, char *str)
{
    sprintf(str, "%02d%02d%02d%02d", button, hour, minute, second);
}
void show_display_mode(int button, int hour, int minute, int second)
{
    // Hiển thị thời gian và trạng thái nút lên LED
    char time_str[8];
    int_to_str(button, hour, minute, second, time_str);
    // Hiển thị thời gian lên LED
    convert_led7((uint8_t *)time_str);
    raw_data[0] = 0x1Fu;
    raw_data[1] = 0xFCu;
    raw_data[3] = raw_data[3] | 0x40u;
    raw_data[5] = raw_data[5] | 0x40u;
    synch_led_7();
    vTaskDelay(10);
}
uint32_t GetMs(void)
{
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void show_setting_mode(int button, int hour, int minute, int second, uint8_t current_index)
{
    // Hiển thị thời gian và trạng thái nút lên LED
    char time_str[8];
    int_to_str(button, hour, minute, second, time_str);
    // Hiển thị thời gian lên LED
    convert_led7((uint8_t *)time_str);
    raw_data[0] = 0xABu;
    raw_data[1] = 0xCEu;
    raw_data[3] = raw_data[3] | 0x40u;
    raw_data[5] = raw_data[5] | 0x40u;
    if (GetMs() % 200 < 100)
    {
        // Không làm cái gì cả
    }
    else
    {
        // Làm cái gì đó
        convert_char7(current_index, ' ');
    }
    synch_led_7();
    vTaskDelay(10);
    // show_display_mode(button, hour, minute, second);
}
uint8_t mode = 0;
uint8_t current_index = 2;
int last_button = 0;
int button_status = 0;
bool flag = false;
int hour, minute, second;
int hc, hv, mc, mv, sc, sv;

void config_setting_mode(int button)
{
    // if (last_button == 11 && button_status == 2)
    // {
    //     mode = 1;
    //     flag = true;
    //     current_index = 2;
    // }
    if (flag)
    {
        if (last_button == 11 && button_status == 1)
        {
            current_index++;
            if (current_index > 7)
                current_index = 2;
        }
        if (last_button == 12 && button_status == 1)
        {
            current_index--;
            if (current_index < 2)
                current_index = 7;
        }
        
        if (current_index == 2)
        {
            if (last_button == 1 && button_status == 1)
                hc = 1;
            else if (last_button == 2 && button_status == 1)
                hc = 2;
            else if (last_button == 10 && button_status == 1)
                hc = 0;
        }
        if (current_index == 3)
        {
            if (hc == 0 || hc == 1)
            {
                if (last_button == 1 && button_status == 1)
                    hv = 1;
                else if (last_button == 2 && button_status == 1)
                    hv = 2;
                else if (last_button == 3 && button_status == 1)
                    hv = 3;
                else if (last_button == 4 && button_status == 1)
                    hv = 4;
                else if (last_button == 5 && button_status == 1)
                    hv = 5;
                else if (last_button == 6 && button_status == 1)
                    hv = 6;
                else if (last_button == 7 && button_status == 1)
                    hv = 7;
                else if (last_button == 8 && button_status == 1)
                    hv = 8;
                else if (last_button == 9 && button_status == 1)
                    hv = 9;
                else if (last_button == 10 && button_status == 1)
                    hv = 0;
            }
            if (hc == 2)
            {
                if (last_button == 1 && button_status == 1)
                    hv = 1;
                else if (last_button == 2 && button_status == 1)
                    hv = 2;
                else if (last_button == 3 && button_status == 1)
                    hv = 3;
                else if (last_button == 10 && button_status == 1)
                    hv = 0;
            }
        }
        if (current_index == 4)
        {
            if (last_button == 1 && button_status == 1)
                mc = 1;
            else if (last_button == 2 && button_status == 1)
                mc = 2;
            else if (last_button == 3 && button_status == 1)
                mc = 3;
            else if (last_button == 4 && button_status == 1)
                mc = 4;
            else if (last_button == 5 && button_status == 1)
                mc = 5;
            else if (last_button == 10 && button_status == 1)
                mc = 0;
        }
        if (current_index == 5)
        {
            if (last_button == 1 && button_status == 1)
                mv = 1;
            else if (last_button == 2 && button_status == 1)
                mv = 2;
            else if (last_button == 3 && button_status == 1)
                mv = 3;
            else if (last_button == 4 && button_status == 1)
                mv = 4;
            else if (last_button == 5 && button_status == 1)
                mv = 5;
            else if (last_button == 6 && button_status == 1)
                mv = 6;
            else if (last_button == 7 && button_status == 1)
                mv = 7;
            else if (last_button == 8 && button_status == 1)
                mv = 8;
            else if (last_button == 9 && button_status == 1)
                mv = 9;
            else if (last_button == 10 && button_status == 1)
                mv = 0;
        }
        if (current_index == 6)
        {
            if (last_button == 1 && button_status == 1)
                sc = 1;
            else if (last_button == 2 && button_status == 1)
                sc = 2;
            else if (last_button == 3 && button_status == 1)
                sc = 3;
            else if (last_button == 4 && button_status == 1)
                sc = 4;
            else if (last_button == 5 && button_status == 1)
                sc = 5;
            else if (last_button == 10 && button_status == 1)
                sc = 0;
        }
        if (current_index == 7)
        {
            if (last_button == 1 && button_status == 1)
                sv = 1;
            else if (last_button == 2 && button_status == 1)
                sv = 2;
            else if (last_button == 3 && button_status == 1)
                sv = 3;
            else if (last_button == 4 && button_status == 1)
                sv = 4;
            else if (last_button == 5 && button_status == 1)
                sv = 5;
            else if (last_button == 6 && button_status == 1)
                sv = 6;
            else if (last_button == 7 && button_status == 1)
                sv = 7;
            else if (last_button == 8 && button_status == 1)
                sv = 8;
            else if (last_button == 9 && button_status == 1)
                sv = 9;
            else if (last_button == 10 && button_status == 1)
                sv = 0;
        }
        hour = hc * 10 + hv;
        minute = mc * 10 + mv;
        second = sc * 10 + sv;
    }
    if (last_button == 12 && button_status == 2)
    {
        mode = 0;
        flag = false;
    }
}

void button_task(void *arg);
typedef struct
{
    int last_button;
    int status;
} ButtonData;
QueueHandle_t button_queue; // Khai báo hàng đợi để chia sẻ dữ liệu

void task1(void *arg)
{
    i2c_dev_t dev;
    memset(&dev, 0, sizeof(i2c_dev_t));
    ButtonData buttonData;
    gpio_set_direction(DATA_PIN_4094, GPIO_MODE_OUTPUT);
    gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LATCH_PIN_4094_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LATCH_PIN_4094_2, GPIO_MODE_OUTPUT);

    gpio_set_direction(DATA_PIN_74165, GPIO_MODE_INPUT);
    gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LATCH_PIN_74165, GPIO_MODE_OUTPUT);
    gpio_set_direction(CLOCKINHIBITPIN_74165, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(DATA_PIN_74165, GPIO_PULLUP_ONLY);

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
        // ESP_ERROR_CHECK(ds1307_adjust(&dev, 2024, 5, 2, 9, 7, 30));
    }
    // for (int i = 00; i <= 99; i += 11)
    // {
    //     char testled_str[8];
    //     int_to_str(i, i, i, i, testled_str);
    //     convert_led7((uint8_t *)testled_str);
    //     synch_led_7();
    //     vTaskDelay(10);
    // }
    synch_test_led(test_led);
    uint8_t str[] = "HIEN-ANH";
    convert_led7(str);
    synch_led_7(); // Gọi hàm để hiển thị dữ liệu lên LED
    vTaskDelay(50);
    uint8_t str2[] = "09112000";
    convert_led7(str2);
    synch_led_7(); // Gọi hàm để hiển thị dữ liệu lên LED
    vTaskDelay(50);

    // static int digit_index = 0;
    while (1)
    {
        int button_save = 0;    
        if (xQueueReceive(button_queue, &buttonData, portMAX_DELAY) == pdPASS)
        {
            // In ra giá trị buttonData từ button_task()
            // printf("Button data: last_button = %d, status = %d\n", buttonData.last_button, buttonData.status);
            last_button = buttonData.last_button;
            button_status = buttonData.status;
            if (last_button!=0) button_save = last_button;
        }
        // int button = 00;
        ds1307_get_time(&dev, &time);
        hour = time.tm_hour;
        minute = time.tm_min;
        second = time.tm_sec;
        hc = (hour / 10) % 10;
        hv = hour % 10;
        mc = (minute / 10) % 10;
        mv = minute % 10;
        sc = (second / 10) % 10;
        sv = second % 10;

        if (mode == 0)
        {
            show_display_mode(button_save, hour, minute, second);
            if (last_button == 11 && button_status == 2)
            {
                mode = 1;
                flag = true;
                current_index = 2;
            }
            if(last_button == 1 && button_status == 1) 
                synch_test_led(test_led);
        }
        else
        {
            show_setting_mode(last_button, hour, minute, second, current_index);
            config_setting_mode(last_button);
            ESP_ERROR_CHECK(ds1307_adjust(&dev, time.tm_year + 1900, time.tm_mon,
                                          time.tm_mday, hour, minute, second));
        }
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

void app_main()
{
    // int button_result = 0; // Variable to store the result of button press
    ESP_ERROR_CHECK(i2cdev_init());
    button_queue = xQueueCreate(1, sizeof(ButtonData));
    xTaskCreate(task1, "task1", 2048, NULL, 10, NULL);
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL); // Không cần truyền tham số vào button_task
}


////////////////////////FINAL_1///////////////////////////

