// RasPiRF69.cpp
//
// Example program showing how to use RH_RF69 on Raspberry Pi
// Uses the bcm2835 library to access the GPIO pins to drive the RFM69 module
// Requires bcm2835 library to be already installed
// http://www.airspayce.com/mikem/bcm2835/
// Use the Makefile in this directory:
// cd example/raspi/
// make
// sudo ./RasPiRF69
//
// Creates a RHReliableDatagram manager and listens and prints for reliable datagrams
// sent to it on the default Channel 2.
//
// Contributed by Charles-Henri Hallard based on sample RH_NRF24 by Mike Poublon

#include <bcm2835.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <RH_RF69.h>

// Our RFM69 Configuration 
#define RF_NODE_ID    1
#define RF_GROUP_ID   69
#define RF_FREQUENCY  433.00

// Create an instance of a driver
// Chip enable is pin 22
// Slave Select is pin 24
RH_RF69 rf69(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24);

//Flag for Ctrl-C
volatile sig_atomic_t flag = 0;

void sig_handler(int sig)
{
  printf("\n---CTRL-C Caught - Exiting---\n");
  flag=1;
}

void printbuffer(uint8_t buff[], int len)
{
  for (int i = 0; i< len; i++)
  {
    printf(" %2X", buff[i]);
  }
}

//Main Function
int main (int argc, const char* argv[] )
{
  signal(SIGINT, sig_handler);

  if (!bcm2835_init())
  {
    printf( "\n\nRasPiRF69 Tester Startup Failed\n\n" );
    return 1;
  }

  printf( "\nRasPiRF69 Tester Startup\n\n" );

  if (!rf69.init()) {
    Serial.println("init failed");
  } else {
    // Change default radioHead modem configuration to moteino one
    rf69.setModemConfig( (RH_RF69::ModemConfigChoice) FSK_MOTEINO);

    // Set TX power mainly to all stuff if RFM69 or RFMH69
    rf69.setTxPower(13);

    // Adjust Frequency
    rf69.setFrequency( RF_FREQUENCY );

    // set Network ID (by sync words)
    syncwords[0] = 0x2d;
    syncwords[1] = RF_GROUP_ID;
    rf69.setSyncWords(syncwords, sizeof(syncwords));

    // set Node ID
    rf69.setThisAddress(RF_NODE_ID);  // filtering address when receiving
    rf69.setHeaderFrom(cRF_NODE_ID);  // Transmit From Node

    //Begin the main body of code
    while (!flag)
    {
      uint8_t len = sizeof(buf);
      uint8_t from, to, id, flags;

      if (rf69.available())
      {
        // Should be a message for us now
        //uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);
        if (rf69.recv(buf, &len))
        {
          Serial.print("got request: ");
          Serial.println((char*)buf);
          Serial.println("");
        }
        else
        {
          Serial.println("recv failed");
        }
      }
      //sleep(1);
      delay(25);
    }
  }

  printf( "\nRasPiRF69 Tester Ending\n" );
  bcm2835_close();
  return 0;
}
