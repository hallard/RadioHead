// spi_scan.cpp
//
// Example program showing how to detect multiple module RH_RF69/RH_RF95 on Raspberry Pi
// Requires bcm2835 library to be already installed
// http://www.airspayce.com/mikem/bcm2835/
// Use the Makefile in this directory:
// cd example/raspi/spi_scan
// make
// sudo ./spi_scan
//
// Will check for RFM92/95/96/98 or RFM69/RFM69HCW/RFM69W modules on SPI BUS
// scan with CS = GPIO6, CE0, CEA and GPIO26
// So it should detect the following boards
// LoRasPi board => https://github.com/hallard/LoRasPI
// RasPI Lora Gateway Board iC880A and LinkLab Lora => https://github.com/ch2i/iC880A-Raspberry-PI
// Raspberri PI Lora Gateway => https://github.com/hallard/RPI-Lora-Gateway
// Dragino Raspberry PI hat => https://github.com/dragino/Lora
//
// Contributed by Charles-Henri Hallard (hallard.me)

#include <bcm2835.h>
#include <stdio.h>

#include "RH_RF69.h"
#include "RH_RF95.h"

uint8_t readRegister(uint8_t cs_pin, uint8_t addr)
{
  char spibuf[2];
  spibuf[0] = addr & 0x7F;
  spibuf[1] = 0x00;

  bcm2835_gpio_write(cs_pin,0);
  bcm2835_spi_transfernb( spibuf, spibuf, sizeof(spibuf) );
  bcm2835_gpio_write(cs_pin,1);
  return spibuf[1];
}

void getModuleName(uint8_t version)
{ 
  printf(" => ");
  if (version==00 || version==0xFF )
    printf("Nothing!\n");
  else if (version == 0x12)
    printf("SX1276 RF95/96");
  else if (version == 0x22)
    printf("SX1272 RF92");
  else if (version == 0x24)
    printf("SX1231 RFM69");
  else 
    printf("Unknown");

  if (version!=00 && version!=0xFF )
    printf(" (V=0x%02X)\n", version);
}

void readModuleVersion(uint8_t cs_pin)
{
  uint8_t version;
  // CS line as output
  bcm2835_gpio_fsel(cs_pin, BCM2835_GPIO_FSEL_OUTP);

  // RFM9x version
  printf("Checking register(0x%02X) with CS=GPIO%02d", RH_RF95_REG_42_VERSION, cs_pin);
  getModuleName( readRegister( cs_pin, RH_RF95_REG_42_VERSION) );

  // RFM69 version
  printf("Checking register(0x%02X) with CS=GPIO%02d", RH_RF69_REG_10_VERSION, cs_pin);
  getModuleName ( readRegister( cs_pin, RH_RF69_REG_10_VERSION)) ;
}

int main(int argc, char **argv)
{

  if (!bcm2835_init()) {
    printf("bcm2835_init failed. Are you running as root??\n");

  } else if (!bcm2835_spi_begin()) {
    printf("bcm2835_spi_begin failed\n");

  } else {
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default

    // We control CS line manually don't assert CEx line!
    bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);

    readModuleVersion(  6 ); // GPIO6
    readModuleVersion(  8 ); // CE0
    readModuleVersion(  7 ); // CE1
    readModuleVersion( 26 ); // GPIO26

    bcm2835_spi_end();
  }

  bcm2835_close();
}


