// rpi_lora_gw.cpp
//
// Example program showing how to use multiple module RH_RF69/RH_RF95 on Raspberry Pi
// Requires bcm2835 library to be already installed
// http://www.airspayce.com/mikem/bcm2835/
// Use the Makefile in this directory:
// cd example/raspi/rpi_lora_gw
// make
// sudo ./rpi_lora_gw
//
// Contributed by Charles-Henri Hallard based on sample RH_NRF24 by Mike Poublon

#include <bcm2835.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <RH_RF69.h>
#include <RH_RF95.h>

// Raspberri PI Lora Gateway for multiple modules 
// see https://github.com/hallard/RPI-Lora-Gateway

// If a pin is not used just define it to NOT_A_PIN
// ie : #define MOD1_RST_PIN NOT_A_PIN

// Module 1 on board RFM95 868 MHz
#define MOD1_LED_PIN RPI_V2_GPIO_P1_07 // Led on GPIO4 so P1 connector pin #7
//#define MOD1_LED_PIN RPI_V2_GPIO_P1_21 // Led on GPIO19 so P1 connector pin #21
#define MOD1_CS_PIN  RPI_V2_GPIO_P1_24 // Slave Select on CE0 so P1 connector pin #24
#define MOD1_IRQ_PIN RPI_V2_GPIO_P1_22 // IRQ on GPIO25 so P1 connector pin #22
#define MOD1_RST_PIN RPI_V2_GPIO_P1_29 // Reset on GPIO5 so P1 connector pin #29

// Module 2 on board RFM95 433 MHz
#define MOD2_LED_PIN RPI_V2_GPIO_P1_11 // Led on GPIO17 so P1 connector pin #11
#define MOD2_CS_PIN  RPI_V2_GPIO_P1_26 // Slave Select on CE1 so P1 connector pin #26
#define MOD2_IRQ_PIN RPI_V2_GPIO_P1_36 // IRQ on GPIO16 so P1 connector pin #36
#define MOD2_RST_PIN RPI_V2_GPIO_P1_31 // Reset on GPIO6 so P1 connector pin #31

// Module 3 on board RFM69HW
#define MOD3_LED_PIN RPI_V2_GPIO_P1_35 // Led on GPIO19 so P1 connector pin #11
#define MOD3_CS_PIN  RPI_V2_GPIO_P1_37 // Slave Select on GPIO26  so P1 connector pin #37
#define MOD3_IRQ_PIN RPI_V2_GPIO_P1_16 // IRQ on GPIO23 so P1 connector pin #16
#define MOD3_RST_PIN RPI_V2_GPIO_P1_33 // reset on GPIO13 so P1 connector pin #33

// User LED
#define LED_PIN  RPI_V2_GPIO_P1_12     // Led on GPIO18 so P1 connector pin #12

// Our RF95 module 1 Configuration 
#define RF95_1_NODE_ID    1
#define RF95_1_FREQUENCY  868.00

// Our RF95 module 3 Configuration 
#define RF95_2_NODE_ID    1
#define RF95_2_FREQUENCY  433.00

// Our RFM69 module 3 Configuration 
#define RF69_NODE_ID    1
#define RF69_GROUP_ID   69
#define RF69_FREQUENCY  433.00

bool  IRQ_states[3] = {0,0,0}; //IRQ Lines check memorized

// For module index table
enum Mod_idx { IDX_MOD1 = 0, IDX_MOD2 = 1, IDX_MOD3 = 2 };

// Module pins table index
#define NB_MODULES 3
uint8_t IRQ_pins[NB_MODULES] = { MOD1_IRQ_PIN, MOD2_IRQ_PIN, MOD3_IRQ_PIN};
uint8_t LED_pins[NB_MODULES] = { MOD1_LED_PIN, MOD2_LED_PIN, MOD3_LED_PIN};
uint8_t RST_pins[NB_MODULES] = { MOD1_RST_PIN, MOD2_RST_PIN, MOD3_RST_PIN};
uint8_t CS_pins[NB_MODULES]  = { MOD1_CS_PIN , MOD2_CS_PIN , MOD3_CS_PIN };

// Create an instance of a driver for 3 modules
// In our case RadioHead code does not use IRQ
// bcm2835 does provde such function, so we
// manually pool IRQ line in loop
RH_RF95 rf95_1(MOD1_CS_PIN, MOD1_IRQ_PIN);
RH_RF95 rf95_2(MOD2_CS_PIN, MOD2_IRQ_PIN);
RH_RF69 rf69_3(MOD3_CS_PIN, MOD3_IRQ_PIN);

//Flag for Ctrl-C
volatile sig_atomic_t force_exit = 0;

// default led blink on receive for 200 ms
#define LED_BLINK_MS  200

void sig_handler(int sig)
{
  printf("\nBreak received, exiting!\n");
  force_exit=true;
}

void printbuffer(uint8_t buff[], int len)
{
  int i;
  bool ascii = true;
  
  // Check for only printable characters
  for (i = 0; i< len; i++) {
    if ( buff[i]<32 || buff[i]>127) {
      ascii = false;
      break;
    }
  }

  // now do real display according to buffer type
  // note each char one by one because we're not sure 
  // string will have \0 on the end
  for (int i = 0; i< len; i++) {
    if (ascii) {
      printf("%c", buff[i]);
    } else {
      printf(" %02X", buff[i]);
    }
  }
}

bool hasIRQ(uint8_t module) 
{
  if ( IRQ_states[module] != digitalRead(IRQ_pins[module]) ) {
    IRQ_states[module] = !IRQ_states[module];
    printf("IRQ Module %d = %d\n", module+1, IRQ_states[module]);
    if (IRQ_states[module]) {
       return true;
    }
  }
  return false;
}

//Main Function
int main (int argc, const char* argv[] )
{
  unsigned long led_blink[3] = {0,0,0};
  
  signal(SIGINT, sig_handler);
  printf( "Rpi Lora Gateway listener\n");
  printf( "=========================\n");

  if (!bcm2835_init()) {
    fprintf( stderr, "bcm2835_init() Failed\n\n" );
    return 1;
  }

  // Set reset line to high and IRQ pin to input
  // and associated module LED to off
  for (uint8_t i=0 ; i<3; i++) {
    pinMode(RST_pins[i], OUTPUT);
    digitalWrite(RST_pins[i], HIGH);
    pinMode(LED_pins[i], OUTPUT);
    digitalWrite(LED_pins[i], LOW);
    pinMode(IRQ_pins[i], INPUT);
  }

  // light onboard LEDs 
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // Init RF95 Module 1
  printf( "RF95 module 1 (CS=GPIO%d, IRQ=GPIO%d)", MOD1_CS_PIN, MOD1_IRQ_PIN );
  if (!rf95_1.init()) {
    fprintf( stderr, " RF95 module 1 init failed, Please verify wiring/module\n" );
    force_exit = true;
  } else {
    printf( " seen OK!");

    // Adjust Frequency
    rf95_1.setFrequency( RF95_1_FREQUENCY );

    // set Node ID
    rf95_1.setThisAddress(RF95_1_NODE_ID); // filtering address when receiving
    rf95_1.setHeaderFrom(RF95_1_NODE_ID);  // Transmit From Node

    printf( " init OK, NodeID=%d @ %3.2fMHz\n", RF95_1_NODE_ID, RF95_1_FREQUENCY );
  }

  // Init RF95 Module 2
  printf( "RF95 module 2 (CS=GPIO%d, IRQ=GPIO%d)", MOD2_CS_PIN, MOD2_IRQ_PIN );
  if (!rf95_2.init()) {
    fprintf( stderr, " RF95 module 2 init failed, Please verify wiring/module\n" );
    force_exit = true;
  } else {
    printf( " seen OK!");
 
    // Adjust Frequency
    rf95_2.setFrequency( RF95_2_FREQUENCY );

    // set Node ID
    rf95_2.setThisAddress(RF95_2_NODE_ID);  // filtering address when receiving
    rf95_2.setHeaderFrom(RF95_2_NODE_ID);  // Transmit From Node

    printf( " init OK, NodeID=%d @ %3.2fMHz\n", RF95_2_NODE_ID, RF95_2_FREQUENCY );
  }

  // Init RF69 Module 3
  printf( "RFM69 module 3 (CS=GPIO%d, IRQ=GPIO%d)", MOD3_CS_PIN, MOD3_IRQ_PIN );
  if (!rf69_3.init()) {
    fprintf( stderr, " RF69 module init failed, Please verify wiring/module\n" );
    force_exit = true;
  } else {
    printf( " seen OK!");

    // Change default radioHead modem configuration to moteino one
    rf69_3.setModemConfig( RH_RF69::FSK_MOTEINO);

    // Set TX power to High power RFMH69
    //rf69.setTxPower(13);
    rf69_3.setTxPower(20);

    // Adjust Frequency
    rf69_3.setFrequency( RF69_FREQUENCY );

    // set Network ID (by sync words)
    uint8_t syncwords[2];
    syncwords[0] = 0x2d;
    syncwords[1] = RF69_GROUP_ID;
    rf69_3.setSyncWords(syncwords, sizeof(syncwords));

    // set Node ID
    rf69_3.setThisAddress(RF69_NODE_ID);  // filtering address when receiving
    rf69_3.setHeaderFrom(RF69_NODE_ID);  // Transmit From Node

    printf( " init OK! NetworkID=%d NodeID=%d @ %3.2fMHz\n", RF69_GROUP_ID, RF69_NODE_ID, RF69_FREQUENCY );
  }

  printf( "Entering main loop\nListening packet...\n" );

  //Begin the main body of code 
  while (!force_exit) { 
    
    if (hasIRQ(IDX_MOD1)) {
      led_blink[IDX_MOD1] = millis();
      digitalWrite(LED_pins[IDX_MOD1], HIGH);
      if (rf95_1.available()) {
        // Should be a message for us now
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t from, to, id, flags;
        uint8_t len = sizeof(buf);

        if (rf95_1.recv(buf, &len)) {
          printf("payload mod1 :");
          printbuffer(buf, len);
        } else {
          printf("recv failed");
        }
        printf("\n");
      }      
    }

    if (hasIRQ(IDX_MOD2)) {
      led_blink[IDX_MOD2] = millis();
      digitalWrite(LED_pins[IDX_MOD2], HIGH);
      if (rf95_2.available()) {
        // Should be a message for us now
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t from, to, id, flags;
        uint8_t len = sizeof(buf);

        if (rf95_2.recv(buf, &len)) {
          printf("payload mod2 :");
          printbuffer(buf, len);
        } else {
          printf("recv failed");
        }
        printf("\n");
      }
    }

    //if (hasIRQ(IDX_MOD3)) {
      if (rf69_3.available()) {
        led_blink[IDX_MOD3] = millis();
        digitalWrite(LED_pins[IDX_MOD3], HIGH);
        // Should be a message for us now
        uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
        uint8_t from, to, id, flags;
        uint8_t len = sizeof(buf);

        if (rf69_3.recv(buf, &len)) {
          printf("payload mod3 from node %d :", from);
          printbuffer(buf, len);
        } else {
          printf("recv failed");
        }
        printf("\n");
      }
    //}

    //sleep(1);
    // Led blink timer expired ?
    for (uint8_t i=0; i<3; i++) {
      if (led_blink[i] && millis()-led_blink[i]>LED_BLINK_MS) {
        led_blink[i] = 0;
        digitalWrite(LED_pins[i], LOW);
      }
    }

    digitalWrite(LED_PIN, (millis()%1000)<500?LOW:HIGH);
    bcm2835_delay(10);
  }

  // Light off all LED
  digitalWrite(MOD1_LED_PIN, LOW);
  digitalWrite(MOD2_LED_PIN, LOW);
  digitalWrite(MOD3_LED_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  printf( "\nRpi Lora Gateway, done my job!\n" );
  bcm2835_close();
  return 0;
}
