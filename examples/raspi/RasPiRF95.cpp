// RasPiRF95.cpp
//
// Example program showing how to use RH_RF95 on Raspberry Pi
// Uses the bcm2835 library to access the GPIO pins to drive the RFM95 module
// Requires bcm2835 library to be already installed
// http://www.airspayce.com/mikem/bcm2835/
// Use the Makefile in this directory:
// cd example/raspi/
// make
// sudo ./RasPiRF95
//
// Contributed by Charles-Henri Hallard based on sample RH_NRF24 by Mike Poublon

#include <bcm2835.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <RH_RF95.h>

// define hardware used change to fit your need
// Uncomment the board you have, if not listed 
// uncommment custom board and set wiring tin custom section

// LoRasPi board 
// see https://github.com/hallard/LoRasPI
#define BOARD_LORASPI

// iC880A and LinkLab Lora Gateway Shield (if RF module plugged into)
// see https://github.com/ch2i/iC880A-Raspberry-PI
//#define BOARD_IC880A_PLATE

// Raspberri PI Lora Gateway BoardiC880A and LinkLab Lora Gateway Shield (if RF module plugged into)
// see https://github.com/ch2i/iC880A-Raspberry-PI
//#define BOARD_IC880A_PLATE

// Dragino Raspberry PI hat
// see https://github.com/dragino/Lora
//#define BOARD_DRAGINO_PIHAT

#if defined (BOARD_LORASPI)
#define RF_LED_PIN RPI_V2_GPIO_P1_16 // Led on GPIO23 so P1 connector pin #16
#define RF_CS_PIN  RPI_V2_GPIO_P1_24 // Slave Select on CE0 so P1 connector pin #24
#define RF_IRQ_PIN RPI_V2_GPIO_P1_22 // IRQ on GPIO24 so P1 connector pin #22

#elif defined (BOARD_IC880A_PLATE)
#define RF_LED_PIN RPI_V2_GPIO_P1_18 // Led on GPIO24 so P1 connector pin #18
#define RF_CS_PIN  RPI_V2_GPIO_P1_24 // Slave Select on CE0 so P1 connector pin #24
#define RF_IRQ_PIN RPI_V2_GPIO_P1_35 // IRQ on GPIO19 so P1 connector pin #35

#elif defined (BOARD_PI_LORA_GATEWAY)
#define RF_LED_PIN RPI_V2_GPIO_P1_35 // Led on GPIO19 so P1 connector pin #35
#define RF_CS_PIN  RPI_V2_GPIO_P1_37 // Slave Select on GPIO26 so P1 connector pin #37
#define RF_IRQ_PIN RPI_V2_GPIO_P1_16 // IRQ on GPIO23 so P1 connector pin #16

#elif defined (BOARD_DRAGINO_PIHAT)
#define RF_CS_PIN  RPI_V2_GPIO_P1_31 // Slave Select on GPIO6 so P1 connector pin #31
#define RF_IRQ_PIN RPI_V2_GPIO_P1_11 // IRQ on GPIO7 so P1 connector pin #11

#elif defined (BOARD_CUSTOM)
// Define custom pin here
#define RF_CS_PIN  RPI_V2_GPIO_P1_24 // Slave Select on CE0 so P1 connector pin #24
#else
#error "Please define Hardware Board"
#endif

// Our RFM95 Configuration 
#define RF_NODE_ID    1
#define RF_FREQUENCY  433.00

// Create an instance of a driver
RH_RF95 rf95(RF_CS_PIN, RF_IRQ_PIN);

//Flag for Ctrl-C
volatile sig_atomic_t flag = 0;
unsigned char cs_pin = 0;

void sig_handler(int sig)
{
  printf("\n---CTRL-C Caught - Exiting---\n");
  flag=1;
}

void printbuffer(uint8_t buff[], int len)
{
  for (int i = 0; i< len; i++){
    printf(" %2X", buff[i]);
  }
}

//Main Function
int main (int argc, const char* argv[] )
{
  signal(SIGINT, sig_handler);

  if (!bcm2835_init()) {
    fprintf( stderr, "RF95 bcm2835_init() Failed\n\n" );
    return 1;
  }

  printf( "RF95 listener (CS=GPIO%d, IRQ=GPIO%d)\n", RF_CS_PIN, RF_IRQ_PIN );

  if (!rf95.init()) {
    fprintf( stderr, "RF95 module init failed, Please verify wiring/module\n" );
  } else {
    printf( "RF95 module seen OK!\r\n");

    // Adjust Frequency
    rf95.setFrequency( RF_FREQUENCY );

    // set Node ID
    rf95.setThisAddress(RF_NODE_ID);  // filtering address when receiving
    rf95.setHeaderFrom(RF_NODE_ID);  // Transmit From Node

    printf( "RF95 init OK, Node %d @ %3.2fMHz\n", RF_NODE_ID, RF_FREQUENCY );
    printf( "Listening packet...\n" );

    //Begin the main body of code
    while (!flag) {
      if (rf95.available()) {
        // Should be a message for us now
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t from, to, id, flags;
        uint8_t len = sizeof(buf);

        if (rf95.recv(buf, &len)) {
          printf("got request: ");
          printbuffer(buf, len);
        } else {
          Serial.print("recv failed");
        }
        printf("\n");
      }
      //sleep(1);
      delay(25);
    }
  }

  printf( "\nRF95 Ending\n" );
  bcm2835_close();
  return 0;
}

