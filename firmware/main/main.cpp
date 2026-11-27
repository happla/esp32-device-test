// Author: happla

#include <cstdio>
#include <cstring>
#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_attr.h"

namespace
{
    constexpr gpio_num_t LED_GPIO = GPIO_NUM_10;
    constexpr gpio_num_t LED_INPUT_GPIO = GPIO_NUM_3;

    constexpr const char* FIRMWARE_VERSION = "1.0.0";
    constexpr size_t COMMAND_BUFFER_SIZE = 64;

    volatile uint32_t pulse_count = 0;

   void IRAM_ATTR gpio_pulse_counter(void* arg)
{
    auto* count = static_cast<uint32_t*>(arg);
    *count = *count + 1;
}
}

class DeviceController
{
public:

    void initialize()
    {
        gpio_config_t output_config{};

        output_config.pin_bit_mask = (1ULL << LED_GPIO);
        output_config.mode = GPIO_MODE_OUTPUT;
        output_config.pull_up_en = GPIO_PULLUP_DISABLE;
        output_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
        output_config.intr_type = GPIO_INTR_DISABLE;

        ESP_ERROR_CHECK(gpio_config(&output_config));

        gpio_config_t input_config{};

        input_config.pin_bit_mask = (1ULL << LED_INPUT_GPIO);
        input_config.mode = GPIO_MODE_INPUT;
        input_config.pull_up_en = GPIO_PULLUP_DISABLE;
        input_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
        input_config.intr_type = GPIO_INTR_POSEDGE;

        ESP_ERROR_CHECK(gpio_config(&input_config));

        ESP_ERROR_CHECK(
            gpio_install_isr_service(ESP_INTR_FLAG_IRAM)
        );

        ESP_ERROR_CHECK(
            gpio_isr_handler_add(
                LED_INPUT_GPIO,
                gpio_pulse_counter,
               const_cast<uint32_t*>(&pulse_count)            )
        );

        set_led(false);

        std::printf("\r\nESP32-C3 Device Tester\r\n");
        std::printf("Firmware: %s\r\n", FIRMWARE_VERSION);
        std::printf("READY\r\n");

        const bool loopback_ok = verify_loopback();

        std::printf("LOOPBACK:%s\r\n",
            loopback_ok ? "PASS" : "FAIL");

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
        else if (std::strcmp(command, "VERIFY_LOOPBACK") == 0)
        {
            const bool result = verify_loopback();

            std::printf("LOOPBACK:%s\r\n",
                result ? "PASS" : "FAIL");
        }
        else if (std::strncmp(command, "MEASURE_PULSES ", 15) == 0)
        {
            measure_pulses(command);
        }
        else if (std::strncmp(command, "COUNT_BLINKS ", 13) == 0)
        {
            count_blinks(command);
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

    bool verify_loopback()
    {
        set_led(false);
        vTaskDelay(pdMS_TO_TICKS(10));

        const int low_state = gpio_get_level(LED_INPUT_GPIO);

        set_led(true);
        vTaskDelay(pdMS_TO_TICKS(10));

        const int high_state = gpio_get_level(LED_INPUT_GPIO);

        set_led(false);
        vTaskDelay(pdMS_TO_TICKS(10));

        const int final_state = gpio_get_level(LED_INPUT_GPIO);

        return low_state == 0 &&
               high_state == 1 &&
               final_state == 0;
    }

    uint32_t generate_pulses(
        uint32_t cycles,
        uint32_t half_period_ms
    )
    {
        set_led(false);
        vTaskDelay(pdMS_TO_TICKS(10));

        pulse_count = 0;

        ESP_ERROR_CHECK(gpio_intr_enable(LED_INPUT_GPIO));

        for (uint32_t i = 0; i < cycles; ++i)
        {
            set_led(true);
            vTaskDelay(pdMS_TO_TICKS(half_period_ms));

            set_led(false);
            vTaskDelay(pdMS_TO_TICKS(half_period_ms));
        }

        vTaskDelay(pdMS_TO_TICKS(10));

        ESP_ERROR_CHECK(gpio_intr_disable(LED_INPUT_GPIO));

        const uint32_t measured = pulse_count;

        set_led(false);

        return measured;
    }

    void measure_pulses(const char* command)
    {
        float duration_seconds = 0;
        uint32_t frequency = 0;

        if (std::sscanf(
                command,
                "MEASURE_PULSES %f %lu",
                &duration_seconds,
                &frequency
            ) != 2 ||
            duration_seconds <= 0 ||
            duration_seconds > 10 ||
            frequency == 0 ||
            frequency > 20)
        {
            std::printf("ERROR:INVALID_ARGUMENT\r\n");
            return;
        }

        const uint32_t cycles =
            static_cast<uint32_t>(
                duration_seconds * frequency + 0.5f
            );

        const uint32_t duration_ms =
            static_cast<uint32_t>(duration_seconds * 1000);

        if (cycles == 0)
        {
            std::printf("ERROR:INVALID_ARGUMENT\r\n");
            return;
        }

        const uint32_t half_period_ms =
            duration_ms / (cycles * 2);

        if (half_period_ms == 0)
        {
            std::printf("ERROR:INVALID_ARGUMENT\r\n");
            return;
        }

        const uint32_t measured =
            generate_pulses(cycles, half_period_ms);

        std::printf("PULSES:%lu\r\n",
            static_cast<unsigned long>(measured));
    }

    void count_blinks(const char* command)
    {
        uint32_t cycles = 0;

        if (std::sscanf(
                command,
                "COUNT_BLINKS %lu",
                &cycles
            ) != 1 ||
            cycles == 0 ||
            cycles > 100)
        {
            std::printf("ERROR:INVALID_ARGUMENT\r\n");
            return;
        }

        const uint32_t measured =
            generate_pulses(cycles, 100);

        std::printf("BLINKS:%lu\r\n",
            static_cast<unsigned long>(measured));
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

        if (character == '\r' || character == '\n')
        {
            if (command_length > 0)
            {
                command_buffer[command_length] = '\0';

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

