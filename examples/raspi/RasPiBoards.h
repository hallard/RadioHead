// defined pins definition of different hardware boards used
// Contributed by Charles-Henri Hallard http://hallard.me

// If a pin is not used just define it to NOT_A_PIN
// or do not define it, both works
// ie : #define RF_LED_PIN NOT_A_PIN

// LoRasPi board 
//==============
// see https://github.com/hallard/LoRasPI
#if defined (BOARD_LORASPI)
#define RF_LED_PIN RPI_V2_GPIO_P1_16 // Led on GPIO23 so P1 connector pin #16
#define RF_CS_PIN  RPI_V2_GPIO_P1_24 // Slave Select on CE0 so P1 connector pin #24
#define RF_IRQ_PIN RPI_V2_GPIO_P1_22 // IRQ on GPIO25 so P1 connector pin #22
#define RF_RST_PIN RPI_V2_GPIO_P1_15 // IRQ on GPIO22 so P1 connector pin #15


// Raspberri PI Lora Gateway Board iC880A and LinkLab Lora Gateway Shield (if RF module plugged into)
// ==================================================================================================
// see https://github.com/ch2i/iC880A-Raspberry-PI
#elif defined (BOARD_IC880A_PLATE)
#define RF_LED_PIN RPI_V2_GPIO_P1_18 // Led on GPIO24 so P1 connector pin #18
#define RF_CS_PIN  RPI_V2_GPIO_P1_24 // Slave Select on CE0 so P1 connector pin #24
#define RF_IRQ_PIN RPI_V2_GPIO_P1_35 // IRQ on GPIO19 so P1 connector pin #35


// Raspberri PI Lora Gateway for multiple modules 
// ==============================================
// see https://github.com/hallard/RPI-Lora-Gateway
#elif defined (BOARD_PI_LORA_GATEWAY)

// Module 1, 2 and 3 are example of module type soldered on the board
// change to fit your needs

// Module 1 on board RFM95 868 MHz (example)
#define MOD1_LED_PIN RPI_V2_GPIO_P1_07 // Led on GPIO4 so P1 connector pin #7
#define MOD1_CS_PIN  RPI_V2_GPIO_P1_24 // Slave Select on CE0 so P1 connector pin #24
#define MOD1_IRQ_PIN RPI_V2_GPIO_P1_22 // IRQ on GPIO25 so P1 connector pin #22
#define MOD1_RST_PIN RPI_V2_GPIO_P1_29 // Reset on GPIO5 so P1 connector pin #29

// Module 2 on board RFM95 433 MHz (example)
#define MOD2_LED_PIN RPI_V2_GPIO_P1_11 // Led on GPIO17 so P1 connector pin #11
#define MOD2_CS_PIN  RPI_V2_GPIO_P1_26 // Slave Select on CE1 so P1 connector pin #26
#define MOD2_IRQ_PIN RPI_V2_GPIO_P1_36 // IRQ on GPIO16 so P1 connector pin #36
#define MOD2_RST_PIN RPI_V2_GPIO_P1_31 // Reset on GPIO6 so P1 connector pin #31

// Module 3 on board RFM69HW (example)
#define MOD3_LED_PIN RPI_V2_GPIO_P1_35 // Led on GPIO19 so P1 connector pin #11
#define MOD3_CS_PIN  RPI_V2_GPIO_P1_37 // Slave Select on GPIO26  so P1 connector pin #37
#define MOD3_IRQ_PIN RPI_V2_GPIO_P1_16 // IRQ on GPIO23 so P1 connector pin #16
#define MOD3_RST_PIN RPI_V2_GPIO_P1_33 // reset on GPIO13 so P1 connector pin #33

// Default set to module 1 (in case used just with one module examples)
#define RF_LED_PIN MOD1_LED_PIN
#define RF_CS_PIN  MOD1_CS_PIN
#define RF_IRQ_PIN MOD1_IRQ_PIN
#define RF_RST_PIN MOD1_RST_PIN

// Dragino Raspberry PI hat (no obboard led)
// =========================================
// see https://github.com/dragino/Lora
#elif defined (BOARD_DRAGINO_PIHAT)
#define RF_CS_PIN  RPI_V2_GPIO_P1_22 // Slave Select on GPIO25 so P1 connector pin #22
#define RF_IRQ_PIN RPI_V2_GPIO_P1_07 // IRQ on GPIO4 so P1 connector pin #7
#define RF_RST_PIN RPI_V2_GPIO_P1_11 // Reset on GPIO17 so P1 connector pin #11
#define RF_LED_PIN NOT_A_PIN				 // No onboard led to drive

#else
#error "RasPiBoards.h => Please define Hardware Board"
#endif
