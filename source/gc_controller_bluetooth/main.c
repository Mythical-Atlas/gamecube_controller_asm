/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Example of writing via DMA to the SPI interface and similarly reading it back via a loopback.

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "btstack.h"

#define TEST_SIZE 1024

//#define SLAVE_PICO

int main() {
    stdio_init_all();

#ifdef SLAVE_PICO
    printf("SPI Slave (Bluetooth Pico)\n");
#else
    printf("SPI Master (GameCube Pico)\n");
#endif

    spi_init(spi_default, 1000 * 1000);

#ifdef SLAVE_PICO
    spi_set_slave(spi_default, true);
#endif

    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    bi_decl(bi_4pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI));

    uint8_t controller_data[8];

#ifdef SLAVE_PICO
    for(uint8_t i = 0; i < 8; i++)
    {
        controller_data[i] = 0x55;
    }
#else
    for(uint8_t i = 0; i < 8; i++)
    {
        controller_data[i] = 0xdd;
    }
#endif

    while(true)
    {
#ifdef SLAVE_PICO
        spi_write_blocking(spi_default, controller_data, 8);
#else
        sleep_ms(10000);
        spi_read_blocking(spi_default, 0, controller_data, 8);
        printf_hexdump(controller_data, 8);
#endif
    }
}