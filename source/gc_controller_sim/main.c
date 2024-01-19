#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

extern uint8_t POLL_RESPONSE_START[8];

extern void controller_sim();

void controller_spi_manager();

void main()
{	
    stdio_init_all();

    spi_init(spi0, 40000000); // 40 MHz

    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    // unsure how necessary this is
    bi_decl(bi_4pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI));

/*
    const uint dma_rx_1 = dma_claim_unused_channel(true);
    const uint dma_rx_2 = dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(dma_rx_1);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi_default, false));
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_chain_to(&c, dma_rx_2);
    dma_channel_configure(dma_rx_1, &c,
                          POLL_RESPONSE_START, // write address
                          &spi_get_hw(spi_default)->dr, // read address
                          8, // element count (each element is of size transfer_data_size)
                          false); // don't start yet

    c = dma_channel_get_default_config(dma_rx_2);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi_default, false));
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_chain_to(&c, dma_rx_1);
    dma_channel_configure(dma_rx_2, &c,
                          POLL_RESPONSE_START, // write address
                          &spi_get_hw(spi_default)->dr, // read address
                          8, // element count (each element is of size transfer_data_size)
                          true);
*/

	//multicore_launch_core1(controller_spi_manager);

	controller_sim();
}

void controller_spi_manager() {
    while(true)
    {
        spi_read_blocking(spi_default, 0, POLL_RESPONSE_START, 8);
    }
}