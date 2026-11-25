// Author: happla

#include <cstdio>
#include <cstring>
#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_err.h"

namespace
{
    constexpr gpio_num_t LED_GPIO = GPIO_NUM_10;
    constexpr const char* FIRMWARE_VERSION = "1.0.0";
    constexpr size_t COMMAND_BUFFER_SIZE = 64;
}

class DeviceController
{
public:
    void initialize()
    {
        gpio_config_t config{};

        config.pin_bit_mask = (1ULL << LED_GPIO);
        config.mode = GPIO_MODE_OUTPUT;
        config.pull_up_en = GPIO_PULLUP_DISABLE;
        config.pull_down_en = GPIO_PULLDOWN_DISABLE;
        config.intr_type = GPIO_INTR_DISABLE;

        ESP_ERROR_CHECK(gpio_config(&config));

        set_led(false);

        std::printf("\r\nESP32-C3 Device Tester\r\n");
        std::printf("Firmware: %s\r\n", FIRMWARE_VERSION);
        std::printf("READY\r\n");

        std::fflush(stdout);
    }

    void process_command(const char* command)
    {
        if (std::strcmp(command, "PING") == 0)
        {
            std::printf("PONG\r\n");
        }
        else if (std::strcmp(command, "GET_VERSION") == 0)
        {
            std::printf("VERSION:%s\r\n", FIRMWARE_VERSION);
        }
        else if (std::strcmp(command, "GET_UPTIME") == 0)
        {
            const int64_t uptime_ms = esp_timer_get_time() / 1000;

            std::printf("UPTIME:%lld\r\n",
                        static_cast<long long>(uptime_ms));
        }
        else if (std::strcmp(command, "LED ON") == 0)
        {
            set_led(true);
            std::printf("LED:ON\r\n");
        }
        else if (std::strcmp(command, "LED OFF") == 0)
        {
            set_led(false);
            std::printf("LED:OFF\r\n");
        }
        else if (std::strcmp(command, "GET_LED") == 0)
        {
            std::printf("LED:%s\r\n",
                        led_state_ ? "ON" : "OFF");
        }
        else
        {
            std::printf("ERROR:UNKNOWN_COMMAND\r\n");
        }

        std::fflush(stdout);
    }

private:
    bool led_state_ = false;

    void set_led(bool enabled)
    {
        ESP_ERROR_CHECK(
            gpio_set_level(LED_GPIO, enabled ? 1 : 0)
        );

        led_state_ = enabled;
    }
};


extern "C" void app_main(void)
{
    static DeviceController device;

    device.initialize();

    char command_buffer[COMMAND_BUFFER_SIZE];
    size_t command_length = 0;

    while (true)
{
    const int character = std::fgetc(stdin);

    if (character == EOF)
    {
        std::clearerr(stdin);
        vTaskDelay(pdMS_TO_TICKS(10));
        continue;
    }

    // Temporary serial input diagnostic
    //std::printf("RX: %d (0x%02X)\r\n",
                //character,
                //static_cast<unsigned int>(character));

    if (character == '\r' || character == '\n')
    {
        if (command_length > 0)
        {
            command_buffer[command_length] = '\0';
            //std::printf("COMMAND: [%s]\r\n", command_buffer);
            device.process_command(command_buffer);
            command_length = 0;
        }
    }
    else if (command_length < sizeof(command_buffer) - 1)
    {
        command_buffer[command_length++] =
            static_cast<char>(character);
    }
    else
    {
        command_length = 0;
        std::printf("ERROR:COMMAND_TOO_LONG\r\n");
        std::fflush(stdout);
    }
}
}

