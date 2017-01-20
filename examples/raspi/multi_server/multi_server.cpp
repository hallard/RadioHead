// multi_server.cpp
//
// Example program showing how to use multiple module RH_RF69/RH_RF95 on Raspberry Pi
// Requires bcm2835 library to be already installed
// http://www.airspayce.com/mikem/bcm2835/
// Use the Makefile in this directory:
// cd example/raspi/multi_server
// make
// sudo ./multi_server
//
// Contributed by Charles-Henri Hallard based on sample RH_NRF24 by Mike Poublon

#include <bcm2835.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include <RHGenericDriver.h>
#include <RH_RF69.h>
#include <RH_RF95.h>

// Raspberri PI Lora Gateway for multiple modules 
// see https://github.com/hallard/RPI-Lora-Gateway
#define BOARD_PI_LORA_GATEWAY

// Now we include RasPi_Boards.h so this will expose defined 
// constants with CS/IRQ/RESET/on board LED pins definition
#include "../RasPiBoards.h"

// User Led on GPIO18 so P1 connector pin #12
#define LED_PIN  RPI_V2_GPIO_P1_12     

// Our RF95 module 1 Configuration 
#define RF95_1_NODE_ID    1
#define RF95_1_FREQUENCY  868.00

// Our RF95 module 3 Configuration 
#define RF95_2_NODE_ID    1
#define RF95_2_FREQUENCY  433.00

// Our RFM69 module 3 Configuration 
#define RF69_3_NODE_ID    1
#define RF69_3_GROUP_ID   69
#define RF69_3_FREQUENCY  433.00

// For module index table
enum Mod_idx { IDX_MOD1 = 0, IDX_MOD2 = 1, IDX_MOD3 = 2 };

// Modules table index
#define NB_MODULES 3
uint8_t IRQ_pins[NB_MODULES] = { MOD1_IRQ_PIN, MOD2_IRQ_PIN, MOD3_IRQ_PIN};
uint8_t LED_pins[NB_MODULES] = { MOD1_LED_PIN, MOD2_LED_PIN, MOD3_LED_PIN};
uint8_t RST_pins[NB_MODULES] = { MOD1_RST_PIN, MOD2_RST_PIN, MOD3_RST_PIN};
uint8_t CSN_pins[NB_MODULES] = { MOD1_CS_PIN , MOD2_CS_PIN , MOD3_CS_PIN };

const char * MOD_name[]      = { "1 RF95 868",     "2 RF95 433" ,    "3 RFM69HW 433"  };
float MOD_freq[NB_MODULES]   = { RF95_1_FREQUENCY, RF95_2_FREQUENCY, RF69_3_FREQUENCY };
uint8_t MOD_id[NB_MODULES]   = { RF95_1_NODE_ID,   RF95_2_NODE_ID,   RF69_3_NODE_ID   };

// Pointer table on radio driver
RHGenericDriver * drivers[NB_MODULES];

// Create an instance of a driver for 3 modules
// In our case RadioHead code does not use IRQ
// callback, bcm2835 does provide such function, 
// but keep line state providing function to test
// Rising/falling/change on GPIO Pin
RH_RF95 rf95_1(MOD1_CS_PIN, MOD1_IRQ_PIN);
RH_RF95 rf95_2(MOD2_CS_PIN, MOD2_IRQ_PIN);
RH_RF69 rf69_3(MOD3_CS_PIN, MOD3_IRQ_PIN);

//Flag for Ctrl-C
volatile sig_atomic_t force_exit = 0;

// default led blink on receive for 200 ms
#define LED_BLINK_MS  200

/* ======================================================================
Function: sig_handler
Purpose : Intercept CTRL-C keyboard to close application
Input   : signal received
Output  : -
Comments: -
====================================================================== */
void sig_handler(int sig)
{
  printf("\nBreak received, exiting!\n");
  force_exit=true;
}

/* ======================================================================
Function: getReceivedData
Purpose : Get module received data and display it
Input   : Module Index from modules table
Output  : -
Comments: -
====================================================================== */
void getReceivedData( uint8_t index) 
{
  RHGenericDriver * driver = drivers[index];
  if (driver->available()) {
    // RH_RF95_MAX_MESSAGE_LEN is > RH_RF69_MAX_MESSAGE_LEN, 
    // So we take the maximum size to be able to handle all
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len  = sizeof(buf);
    uint8_t from = driver->headerFrom();
    uint8_t to   = driver->headerTo();
    uint8_t id   = driver->headerId();
    uint8_t flags= driver->headerFlags();;
    int8_t rssi  = driver->lastRssi();

    if (driver->recv(buf, &len)) {
      time_t timer;
      char time_buff[16];
      struct tm* tm_info;

      time(&timer);
      tm_info = localtime(&timer);

      strftime(time_buff, sizeof(time_buff), "%H:%M:%S", tm_info);

      printf("%s Mod%s [%02d] #%d => #%d %ddB: ", 
                time_buff, MOD_name[index], len, from, to, rssi);
      printbuffer(buf, len);
    } else {
      printf("receive failed");
    }
    printf("\n");
  }
}

/* ======================================================================
Function: initRadioModule
Purpose : initialize a module
Input   : Module Index from modules table
Output  : -
Comments: -
====================================================================== */
bool initRadioModule( uint8_t index) {
  RHGenericDriver * driver = drivers[index];
  //RH_RF69 * driver = (RH_RF69 *) drivers[index];

  // Module Information
  printf( "%s (CS=GPIO%d, IRQ=GPIO%d, RST=GPIO%d, LED=GPIO%d)", 
          MOD_name[index], CSN_pins[index], IRQ_pins[index], RST_pins[index], LED_pins[index] );

  // Light Module LED
  pinMode(LED_pins[index], OUTPUT);

  // IRQ Pin, as input with Pull down (in case no module connected)
  pinMode(IRQ_pins[index], INPUT);
  bcm2835_gpio_set_pud(IRQ_pins[index], BCM2835_GPIO_PUD_DOWN);

  // we enable rising edge detection 
  bcm2835_gpio_ren(IRQ_pins[index]);

  // Reset module and blink the module LED 
  digitalWrite(LED_pins[index], HIGH);
  pinMode(RST_pins[index], OUTPUT);
  digitalWrite(RST_pins[index], LOW);
  bcm2835_delay(150);
  digitalWrite(RST_pins[index], HIGH);
  bcm2835_delay(100);
  digitalWrite(LED_pins[index], LOW);

  if (!driver->init()) {
    fprintf( stderr, "\n%s init failed, Please verify wiring/module\n", MOD_name[index] );
  } else {
    // Since we'll check IRQ line with bcm_2835 Rising edge detection
    // In case radio already have a packet, IRQ is high and will never
    // go to low until cleared, so never fire IRQ LOW to HIGH again 
    // Except if we clear IRQ flags and discard packet if any!
    // ==> This is now done before reset
    //driver->available();
    // now we can enable rising edge detection 
    //bcm2835_gpio_ren(IRQ_pins[index]);

    // set Node ID
    driver->setThisAddress(MOD_id[index]); // filtering address when receiving
    driver->setHeaderFrom(MOD_id[index]);  // Transmit From Node

    // Get all frame, we're in demo mode
    driver->setPromiscuous(true);   

    // We need to check module type since generic driver does
    // not expose driver specific methods
    switch (index) {
      // RF95
      case IDX_MOD1:
      case IDX_MOD2:
        // Defaults RF95 after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
        // The default transmitter power is 13dBm, using PA_BOOST.
        // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
        // you can set transmitter powers from 5 to 23 dBm:
        // check your country max power useable, in EU it's +14dB
        ((RH_RF95 *) driver)->setTxPower(14, false);
        // If you are using Modtronix inAir4 or inAir9,or any other module which uses the
        // transmitter RFO pins and not the PA_BOOST pins
        // then you can configure the power transmitter power for -1 to 14 dBm and with useRFO true.
        // Failure to do that will result in extremely low transmit powers.
        // rf95.setTxPower(14, true);

        // You can optionally require this module to wait until Channel Activity
        // Detection shows no activity on the channel before transmitting by setting
        // the CAD timeout to non-zero:
        //rf95.setCADTimeout(10000);

        // Adjust Frequency
        ((RH_RF95 *) driver)->setFrequency( MOD_freq[index] );
      break;

      // RF69
      case IDX_MOD3:
        // Adjust Frequency
        ((RH_RF69 *) driver)->setFrequency( MOD_freq[index] );
        
        // Set TX power to High power RFMH69
        //((RH_RF69 *) driver)->setTxPower(13);
        ((RH_RF69 *) driver)->setTxPower(20);

        // Change default radioHead modem configuration to moteino one
        ((RH_RF69 *) driver)->setModemConfig( RH_RF69::FSK_MOTEINO);

        // set Network ID (by sync words)
        uint8_t syncwords[2];
        syncwords[0] = 0x2d;
        syncwords[1] = RF69_3_GROUP_ID;
        ((RH_RF69 *) driver)->setSyncWords(syncwords, sizeof(syncwords));
      break;
    }

    printf( " OK!, NodeID=%d @ %3.2fMHz\n", MOD_id[index], MOD_freq[index] );
    return true;
  }
  
  return false;
}

/* ======================================================================
Function: main
Purpose : not sure ;)
Input   : command line parameters
Output  : -
Comments: -
====================================================================== */
int main (int argc, const char* argv[] )
{
  // LED blink ms timer saving
  unsigned long led_blink[NB_MODULES] = {0,0,0};
  
  // caught CTRL-C to do clean-up
  signal(SIGINT, sig_handler);
  
  // Display app name
  printf( "%s\n", __BASEFILE__);
  for (uint8_t i=0; i<strlen(__BASEFILE__); i++) {
    printf( "=");
  }
  printf("\n");

  // Init GPIO bcm
  if (!bcm2835_init()) {
    fprintf( stderr, "bcm2835_init() Failed\n\n" );
    return 1;
  }

  //save driver instances pointer
  drivers[IDX_MOD1] = &rf95_1;
  drivers[IDX_MOD2] = &rf95_2;
  drivers[IDX_MOD3] = &rf69_3;

  // light onboard LEDs 
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // configure all modules I/O CS pins to 1 before anything else
  // to avoid any problem with SPI sharing
  for (uint8_t i=0 ; i<NB_MODULES; i++) {
    // CS Ping as output and set to 1
    pinMode(CSN_pins[i], OUTPUT);
    digitalWrite(CSN_pins[i], HIGH);
  }

  // configure all modules I/O pins 
  for (uint8_t i=0 ; i<NB_MODULES; i++) {
    // configure all modules
    if (!initRadioModule(i)){
      force_exit = true;
    }
  }

  // All init went fine, continue specific init if any
  if (!force_exit) {
    // Set all modules in receive mode
    rf95_1.setModeRx();
    rf95_2.setModeRx();
    rf69_3.setModeRx();
    printf( "Listening for incoming packets...\n" );
  }

  // Begin the main loop code 
  // ========================
  while (!force_exit) { 
    // Loop thru modules
    for (uint8_t idx=0 ; idx<NB_MODULES ; idx++) {
      // Rising edge fired ?
      if (bcm2835_gpio_eds(IRQ_pins[idx])) {
        // Now clear the eds flag by setting it to 1
        bcm2835_gpio_set_eds(IRQ_pins[idx]);
        //printf("Packet Received, Rising event detect for pin GPIO%d\n", IRQ_pins[idx]);
  
        // Start associated led blink
        led_blink[idx] = millis();
        digitalWrite(LED_pins[idx], HIGH);
        getReceivedData(idx); 
      } // has IRQ

      // A module led blink timer expired ? Light off
      if (led_blink[idx] && millis()-led_blink[idx]>LED_BLINK_MS) {
        led_blink[idx] = 0;
        digitalWrite(LED_pins[idx], LOW);
      } // Led timer expired

      getReceivedData(idx); 

    } // For Modules
    
    // On board led blink (500ms off / 500ms On)
    digitalWrite(LED_PIN, (millis()%1000)<500?LOW:HIGH);

    // Let OS doing other tasks
    // For timed critical appliation receiver, you can reduce or delete
    // this delay, but this will charge CPU usage, take care and monitor
    bcm2835_delay(5);
  }

  // We're here because we need to exit, do it clean
  // Light off on board LED
  digitalWrite(LED_PIN, LOW);
  // All module LEDs off, all modules CS line High
  for (uint8_t i=0 ; i<NB_MODULES; i++) {
    digitalWrite(LED_pins[i], LOW);
    digitalWrite(CSN_pins[i], HIGH);
  }
  printf( "\n%s, done my job!\n", __BASEFILE__ );
  bcm2835_close();
  return 0;
}
