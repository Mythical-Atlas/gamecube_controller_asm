#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

extern uint8_t POLL_RESPONSE_START[8];

extern void controller_sim();

void controller_spi_manager();

void main()
{	
    stdio_init_all();

    save_and_disable_interrupts();

    spi_init(spi_default, 1000000); // 1 MHz

    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    // unsure how necessary this is
    bi_decl(bi_4pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI));

	multicore_launch_core1(controller_spi_manager);

	controller_sim();
}

void controller_spi_manager() {
    while(true)
    {
        spi_read_blocking(spi_default, 0, POLL_RESPONSE_START, 8);
    }
}