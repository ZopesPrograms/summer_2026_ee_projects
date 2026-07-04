#include "spi.h"
#include "gpio.h"

#define SPI_BITS_PER_SEC 1000000

int main() {
    spi_init();
    spi_device_t *dev = spi_new(GPIO_PB0, SPI_MODE_3, SPI_BITS_PER_SEC);
    return 0;
}
