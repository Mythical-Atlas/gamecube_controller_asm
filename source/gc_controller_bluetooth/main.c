#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "btstack.h"

#include "bt_hid.h"

uint8_t POLL_RESPONSE_START[8];

int main() {
    stdio_init_all();

    spi_init(spi_default, 20000000); // 20 MHz
    spi_set_slave(spi_default, true);

    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    bi_decl(bi_4pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI));

    multicore_launch_core1(bt_main);

    while(true)
    {
        //spi_write_blocking(spi_default, POLL_RESPONSE_START, 8);
    }
}