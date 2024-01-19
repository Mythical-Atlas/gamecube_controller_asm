#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "btstack.h"
#include "hardware/dma.h"

#include "bt_hid.h"

uint8_t POLL_RESPONSE_START[8];

uint8_t fake_spi_rx[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void dma_handler()
{
    dma_start_channel_mask(3); // get the next transfer ready. this will only trigger when the master pico requests data
}

int main() {
    stdio_init_all();

    spi_init(spi_default, 10000000); // 10 MHz
    spi_set_slave(spi_default, true);

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
    dma_channel_configure(dma_rx, &c, fake_spi_rx, &spi_get_hw(spi_default)->dr, 8, false);

    /*dma_channel_set_irq0_enabled(dma_rx, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);*/

    c = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));
    dma_channel_configure(dma_tx, &c, &spi_get_hw(spi_default)->dr, POLL_RESPONSE_START, 8, false);

    //dma_handler();

    /*multicore_launch_core1(*/bt_main();

    /*while(true)
    {
        //spi_write_blocking(spi_default, POLL_RESPONSE_START, 8);
    }*/
}