// irq_test.cpp
//
// Example program showing how to use multiple module RH_RF69/RH_RF95 on Raspberry Pi
// Requires bcm2835 library to be already installed
// http://www.airspayce.com/mikem/bcm2835/
// Use the Makefile in this directory:
// cd example/raspi/irq_test
// make
// sudo ./irq_test
//
// Will check for rising edge on RF_IRQ_PIN defined below
//
// Contributed by Charles-Henri Hallard (hallard.me)
#include <bcm2835.h>
#include <stdio.h>

//#define RF_IRQ_PIN RPI_V2_GPIO_P1_22 // IRQ on GPIO25 so P1 connector pin #22
#define RF_IRQ_PIN RPI_V2_GPIO_P1_37 // IRQ on GPIO26 so P1 connector pin #37

int main(int argc, char **argv)
{
  // If you call this, it will not actually access the GPIO
  if (!bcm2835_init())
    return 1;
  
  // Set RPI pin to be an input
  bcm2835_gpio_fsel(RF_IRQ_PIN, BCM2835_GPIO_FSEL_INPT);
  //  with a puldown
  bcm2835_gpio_set_pud(RF_IRQ_PIN, BCM2835_GPIO_PUD_DOWN);
  // And a rising edge detect enable
  bcm2835_gpio_ren(RF_IRQ_PIN);
  
  while (1) {
    // we got it ?
    if (bcm2835_gpio_eds(RF_IRQ_PIN)) {
      // Now clear the eds flag by setting it to 1
      bcm2835_gpio_set_eds(RF_IRQ_PIN);
      printf("Rising event detect for pin GPIO%d\n", RF_IRQ_PIN);
    }
    // wait a bit
    bcm2835_delay(5); 
  }
  
  bcm2835_close();
  return 0;
}
