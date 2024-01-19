#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

extern uint8_t POLL_RESPONSE_START[8];

uint8_t fake_spi_tx[8] __attribute__ ((aligned (8))) = {0, 0, 0, 0, 0, 0, 0, 0};

extern void controller_sim();

void main()
{	
    stdio_init_all();

    spi_init(spi_default, 10000000); // 10 MHz

    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    const uint dma_rx = 0;//dma_claim_unused_channel(true);
    const uint dma_tx = 1;//dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(dma_rx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi_default, false));
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_ring(&c, true, 3);
    dma_channel_configure(dma_rx, &c, POLL_RESPONSE_START, &spi_get_hw(spi_default)->dr, 8, false);

    c = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));
    channel_config_set_ring(&c, false, 3);
    dma_channel_configure(dma_tx, &c, &spi_get_hw(spi_default)->dr, fake_spi_tx, 8, false);

	controller_sim();
}