// rf69_client.cpp
//
// Example program showing how to use RH_RF69 on Raspberry Pi
// Uses the bcm2835 library to access the GPIO pins to drive the RFM69 module
// Requires bcm2835 library to be already installed
// http://www.airspayce.com/mikem/bcm2835/
// Use the Makefile in this directory:
// cd example/raspi/rf69
// make
// sudo ./rf69_client
//
// Contributed by Charles-Henri Hallard based on sample RH_NRF24 by Mike Poublon

#include <bcm2835.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <RH_RF69.h>

// define hardware used change to fit your need
// Uncomment the board you have, if not listed 
// uncommment custom board and set wiring tin custom section

// LoRasPi board 
// see https://github.com/hallard/LoRasPI
#define BOARD_LORASPI

// iC880A and LinkLab Lora Gateway Shield (if RF module plugged into)
// see https://github.com/ch2i/iC880A-Raspberry-PI
//#define BOARD_IC880A_PLATE

// Raspberri PI Lora Gateway for multiple modules 
// see https://github.com/hallard/RPI-Lora-Gateway
//#define BOARD_PI_LORA_GATEWAY

// Dragino Raspberry PI hat
// see https://github.com/dragino/Lora
//#define BOARD_DRAGINO_PIHAT

// Now we include RasPi_Boards.h so this will expose defined 
// constants with CS/IRQ/RESET/on board LED pins definition
#include "../RasPiBoards.h"

// Our RFM69 Configuration 
#define RF_FREQUENCY  433.00
#define RF_GROUP_ID   69 // Moteino default is 100, I'm using 69 on all my house
#define RF_GATEWAY_ID 1  // Were to send packet
#define RF_NODE_ID    10 // our own ID

// Create an instance of a driver
RH_RF69 rf69(RF_CS_PIN, RF_IRQ_PIN);
//RH_RF69 rf69(RF_CS_PIN);

//Flag for Ctrl-C
volatile sig_atomic_t force_exit = false;

void sig_handler(int sig)
{
  printf("\n%s Break received, exiting!\n", __BASEFILE__);
  force_exit=true;
}

//Main Function
int main (int argc, const char* argv[] )
{
  static unsigned long last_millis;
  static unsigned long led_blink = 0;
  
  signal(SIGINT, sig_handler);
  printf( "%s\n", __BASEFILE__);

  if (!bcm2835_init()) {
    fprintf( stderr, "%s bcm2835_init() Failed\n\n", __BASEFILE__ );
    return 1;
  }
  
  printf( "RF69 CS=GPIO%d", RF_CS_PIN);

#ifdef RF_LED_PIN
  pinMode(RF_LED_PIN, OUTPUT);
  digitalWrite(RF_LED_PIN, HIGH );
#endif

#ifdef RF_IRQ_PIN
  printf( ", IRQ=GPIO%d", RF_IRQ_PIN );
  // IRQ Pin input/pull down 
  // But it's not used in this client send test
  pinMode(RF_IRQ_PIN, INPUT);
  bcm2835_gpio_set_pud(RF_IRQ_PIN, BCM2835_GPIO_PUD_DOWN);
#endif
  
#ifdef RF_RST_PIN
  printf( ", RST=GPIO%d", RF_RST_PIN );
  // Pulse a reset on module
  pinMode(RF_RST_PIN, OUTPUT);
  digitalWrite(RF_RST_PIN, LOW );
  bcm2835_delay(150);
  digitalWrite(RF_RST_PIN, HIGH );
  bcm2835_delay(100);
#endif

#ifdef RF_LED_PIN
  printf( ", LED=GPIO%d", RF_LED_PIN );
  digitalWrite(RF_LED_PIN, LOW );
#endif

  if (!rf69.init()) {
    fprintf( stderr, "\nRF69 module init failed, Please verify wiring/module\n" );
  } else {
    printf( "\nRF69 module seen OK!\r\n");

#ifdef RF_IRQ_PIN
    // Since we may check IRQ line with bcm_2835 Rising edge detection
    // In case radio already have a packet, IRQ is high and will never
    // go to low so never fire again 
    // Except if we clear IRQ flags and discard one if any by checking
    rf69.available();

    // Now we can enable Rising edge detection
    bcm2835_gpio_ren(RF_IRQ_PIN);
#endif

    
    // Defaults after init are 434.0MHz, +13dBm, no encryption, GFSK=250kbps, Fdev=250kHz

    // The default transmitter power is 13dBm. If you are using
    // High power version (RFM69HW or RFM69HCW) you need to set 
    // transmitter power to at least 14 dBm up to 20dBm
    rf69.setTxPower(20); 

    // Now we change back to Moteino setting to be 
    // compatible with RFM69 library from lowpowerlabs 
    rf69.setModemConfig( RH_RF69::FSK_MOTEINO);

    // set Network ID (by sync words)
    uint8_t syncwords[2];
    syncwords[0] = 0x2d;
    syncwords[1] = RF_GROUP_ID;
    rf69.setSyncWords(syncwords, sizeof(syncwords));

    // Adjust Frequency
    rf69.setFrequency( RF_FREQUENCY );

    // This is our Node ID
    rf69.setThisAddress(RF_NODE_ID);
    rf69.setHeaderFrom(RF_NODE_ID);
    
    // Where we're sending packet
    rf69.setHeaderTo(RF_GATEWAY_ID);  

    printf("RF69 GroupID=%d, node #%d init OK @ %3.2fMHz\n", RF_GROUP_ID, RF_NODE_ID, RF_FREQUENCY );

    last_millis = millis();

    //Begin the main body of code
    while (!force_exit) {

      //printf( "millis()=%ld last=%ld diff=%ld\n", millis() , last_millis,  millis() - last_millis );

      // Send every 5 seconds
      if ( millis() - last_millis > 5000 ) {
        last_millis = millis();

#ifdef RF_LED_PIN
        led_blink = millis();
        digitalWrite(RF_LED_PIN, HIGH);
#endif
        
        // Send a message to rf69_server
        uint8_t data[] = "Hi Raspi!";
        uint8_t len = sizeof(data);
        
        printf("Sending %02d bytes to node #%d => ", len, RF_GATEWAY_ID );
        printbuffer(data, len);
        printf("\n" );
        rf69.send(data, len);
        rf69.waitPacketSent();
/*
        // Now wait for a reply
        uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);

        if (rf69.waitAvailableTimeout(1000)) { 
          // Should be a reply message for us now   
          if (rf69.recv(buf, &len)) {
            printf("got reply: ");
            printbuffer(buf,len);
            printf("\nRSSI: %d\n", rf69.lastRssi());
          } else {
            printf("recv failed");
          }
        } else {
          printf("No reply, is rf69_server running?\n");
        }
*/
        
      }

#ifdef RF_LED_PIN
      // Led blink timer expiration ?
      if (led_blink && millis()-led_blink>200) {
        led_blink = 0;
        digitalWrite(RF_LED_PIN, LOW);
      }
#endif
      
      // Let OS doing other tasks
      // Since we do nothing until each 5 sec
      bcm2835_delay(100);
    }
  }

#ifdef RF_LED_PIN
  digitalWrite(RF_LED_PIN, LOW );
#endif
  printf( "\n%s Ending\n", __BASEFILE__ );
  bcm2835_close();
  return 0;
}

