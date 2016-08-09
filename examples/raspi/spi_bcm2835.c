// build with gcc -o spi_bcm2835 spi.c -l bcm2835

#include <bcm2835.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    char send_data[2];
    char read_data[2];

    if (!bcm2835_init()) {
      printf("bcm2835_init failed. Are you running as root??\n");
    } else if (!bcm2835_spi_begin()) {
      printf("bcm2835_spi_begin failed\n");
    } else {
      bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
      bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
      bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default
      bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
      bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default

      send_data[0] = 0x42; send_data[1] = 0x00;
      bcm2835_spi_transfernb(send_data, read_data, 2);
      printf("Sent to SPI: 0x%02X, received 0x%02X\n", send_data[0], read_data[1]);

      send_data[0] = 0x10; send_data[1] = 0x00;
      bcm2835_spi_transfernb(send_data, read_data, 2);
      printf("Sent to SPI: 0x%02X, received 0x%02X\n", send_data[0], read_data[1]);
    }

    bcm2835_spi_end();
    bcm2835_close();
}

