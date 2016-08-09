
// build with gcc -o spi_wiringpi spi_wiringpi.c -l wiringPi

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

// chip Select (CE0)
#define SPI_CHANNEL 0
#define SS_GPIO 8

uint8_t readRegister(uint8_t addr)
{
  uint8_t spibuf[2];
  spibuf[0] = addr & 0x7F;
  spibuf[1] = 0x00;

  digitalWrite(SS_GPIO, LOW);
  if ( wiringPiSPIDataRW(SPI_CHANNEL, spibuf, sizeof(spibuf)) == -1 ) {
    fprintf (stderr, "SPI failure: %s\r\n", strerror (errno)) ;
  }
  digitalWrite(SS_GPIO, HIGH);
  return spibuf[1];
}

int main (void)
{
  int myFd = 0;
  unsigned char myData ;

  wiringPiSetup();

  // CE0 at 4MHz
  if ((myFd = wiringPiSPISetup (0, 4000000)) < 0) {
    fprintf (stderr, "Can't open the SPI bus: %s\n", strerror (errno)) ;
    exit (EXIT_FAILURE) ;
  }

  myData = 0x42; // RFM9x version
  printf("Sent to SPI: 0x%02X", myData);
  myData = readRegister( myData);
  printf("=0x%02X\r\n", myData);

  myData = 0x10; // RFM69 version
  printf("Sent to SPI: 0x%02X", myData);
  myData = readRegister( myData);
  printf("=0x%02X\r\n", myData);

  close (myFd) ;
  return 0 ;
}
