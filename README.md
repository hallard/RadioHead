RadioHead Packet Radio library for embedded microprocessors
===========================================================

###Version 1.67

This is a fork of the original RadioHead Packet Radio library for embedded microprocessors. It provides a complete object-oriented library for sending and receiving packetized messages via a variety of common data radios and other transports on a range of embedded microprocessors.

**Please read the full documentation and licensing from the original author [site][3]**

### features added with this fork
=================================

**Compatible with boards**    

[LoRasPI][10], [Raspberry PI Lora Gateway][12], [Dragino Lora GPS HAT][13]

<img src="https://raw.githubusercontent.com/hallard/LoRasPI/master/images/LoRasPI-on-Pi.jpg" height="25%" width="25%" alt="LoRasPI">&nbsp;
<img src="https://raw.githubusercontent.com/hallard/RPI-Lora-Gateway/master/images/RPI-Lora-Gateway-mounted.jpg" height="25%" width="25%" alt="Raspberry PI Lora Gateway/Node">&nbsp;
<img src="http://wiki.dragino.com/images/d/d6/Lora_GPS_HAT.png" height="25%" width="25%" alt="Raspberry PI Lora Gateway/Node">   

- Added moteino modem setting on RF69 to be compatible with lowpowerlab RF69 configuration library
- Added possibility to work with no IRQ connected for RF69 and RF95
  - for example to get one more GPIO free 
  - on Raspberry Pi, we do not have `attachInterrupt()` like with bcm2835 library
- Added samples for multiples Raspberry Pi boards with RF69 and RF95 modules such as 
  - [LoRasPI][10], simple RFM9x or RFM69HCW shield
  - [iC880A or Linklabs Raspberry PI shield][11] with RFM9x or RFM69HCW onboard 
  - [Raspberry PI Lora Gateway][12] with multiple RFM9x or RFM69HCW shield
  - [Dragino Lora shield][13]
  - Sample code are in [rf95][21], [rf69][20], [nrf24][22] and [multi_server][23], note that old sample NRF24 sample has been moved to nrf24 folder for consistency.
- Added 2 samples test tools (for Raspberry PI) do detect RF69 and RF95 modules and check IRQ rising edge
  - [spi_scan][9] sample code, scan and try to detect connected modules
  - [irq_test][8] sample code, check a rising edge on a GPIO

Sample code for Raspberry PI is located under [RadioHead/examples/raspi][7] folder.

### Installation on Raspberry PI
================================

Clone repository
```shell
git clone https://github.com/hallard/RadioHead
```

To avoid system hangs/instability starting with kernel 4.14, disable all GPIO kernel interrupts by adding this line to your `/boot/config.txt`:
```
dtoverlay=gpio-no-irq
```
This works around an issue with the design of the bcm2835 library and how it handles rising/falling edge detection events, but has some downsides as well.  For more information, see [this issue][30] and [this discussion][31].

**Connection and pins definition**

Boards pins (Chip Select, IRQ line, Reset and LED) definition are set in the new [RadioHead/examples/raspi/RasPiBoards.h][24] file. In your code, you need to define board used and then, include the file definition like this
```cpp
// LoRasPi board 
#define BOARD_LORASPI

// Now we include RasPi_Boards.h so this will expose defined 
// constants with CS/IRQ/RESET/on board LED pins definition
#include "../RasPiBoards.h"

// Your code start here
#ifdef RF_RST_PIN
// Blah blah do reset line
#endif

```

Then in your code you'll have exposed RF_CS_PIN, RF_IRQ_PIN, RF_RST_PIN and RF_LED_PIN and you'll be able to do some `#ifdef RF_LED_LIN` for example. See [rf95_client][25] sample code.

So you have 3 options to define the pins you want 

- The board you have is already defined so just need to define it your source code (as explained above)
- You can add your board into [RasPiBoards.h][24] and then define it your source code as above
- You can manually define pins in your code and remove the board definition and `#include "../RasPiBoards.h"`

To go further with examples :

go to example folder here spi_scan
```shell
cd RadioHead/examples/raspi/spi_scan
```
Build executable
```shell
root@pi03(rw):~/RadioHead/examples/raspi/spi_scan# make
g++ -DRASPBERRY_PI -DBCM2835_NO_DELAY_COMPATIBILITY -c -I../../.. spi_scan.c
g++ spi_scan.o -lbcm2835  -o spi_scan
root@pi03(rw):~/RadioHead/examples/raspi/spi_scan
```
And run 
```shell
root@pi03(rw):~/RadioHead/examples/raspi/spi_scan# ./spi_scan
Checking register(0x42) with CS=GPIO06 => Nothing!
Checking register(0x10) with CS=GPIO06 => Nothing!
Checking register(0x42) with CS=GPIO08 => SX1276 RF95/96 (V=0x12)
Checking register(0x10) with CS=GPIO08 => Nothing!
Checking register(0x42) with CS=GPIO07 => Nothing!
Checking register(0x10) with CS=GPIO07 => Nothing!
Checking register(0x42) with CS=GPIO26 => Nothing!
Checking register(0x10) with CS=GPIO26 => Nothing!
```
And voila! with [LoRasPi][10] board RFM95 dedected on SPI with GPIO8 (CE0)


If I'm doing same test with [PI Lora Gateway][12] with 2 RFM95 (one 433MHz and one 868MHz) and one RFMHW69 433MHz on board like this    

<img src="https://raw.githubusercontent.com/hallard/RPI-Lora-Gateway/master/images/RPI-Lora-Gateway-mounted.jpg" height="40%" width="40%" alt="Raspberry PI Lora Gateway/Node">   

Here are the results when trying to detect the onboard modules:

```shell
root@pi01(rw):~/RadioHead/examples/raspi/spi_scan# ./spi_scan
Checking register(0x42) with CS=GPIO06 => Nothing!
Checking register(0x10) with CS=GPIO06 => Nothing!
Checking register(0x42) with CS=GPIO08 => SX1276 RF95/96 (V=0x12)
Checking register(0x10) with CS=GPIO08 => Nothing!
Checking register(0x42) with CS=GPIO07 => SX1276 RF95/96 (V=0x12)
Checking register(0x10) with CS=GPIO07 => Nothing!
Checking register(0x42) with CS=GPIO26 => Unknown (V=0x01)
Checking register(0x10) with CS=GPIO26 => SX1231 RFM69 (V=0x24)
```

Voila! 3 modules are seen, now let's try listenning packets with PI Lora [Gateway][12].

My setup has another Raspberry Pi with RFM95 868MHZ [LoRasPI][10] shield running [`rf95_client`][25] sample and some [ULPnode][6] prototypes always running with on board RFM69 configured as Group ID 69 on 433MHz. I don't have a Lora 433MHz sender running so we won't receive anything on this one.

Here the results starting from scratch

**Client side**    

<img src="https://raw.githubusercontent.com/hallard/RadioHead/master/examples/raspi/pictures/rf95_client.png" alt="RF95 client">    

**multi server side**    

<img src="https://raw.githubusercontent.com/hallard/RadioHead/master/examples/raspi/pictures/multi_server.png" alt="RF95 client">   

It works! 

### Difference with original Author repo
========================================

Due to easier maintenance to keep in sync with original author lib, I've got 2 repo:    

- My master one (this one) https://github.com/hallard/RadioHead that is the one you need if you want to use my projects or lib added features.
-  The one above has been forked to https://github.com/ch2i/RadioHead where I put the original version released by the author.

Like this, I can do Pull Request from [ch2i][4] to [hallard][1] to add new features added by the author to my version. This mean that this [one][4] is just a github copy version of the latest original done by Mike, I don't do any change on this one. I know it's not the best way, but I didn't found a better solution for now, if you have better idea, just let me know.

[1]: https://github.com/hallard/RadioHead 
[2]: https://hallard.me
[3]: http://www.airspayce.com/mikem/arduino/RadioHead/
[4]: http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.67.zip
[5]: https://github.com/ch2i/RadioHead 
[6]: http://hallard.me/category/ulpnode/ 
[7]: https://github.com/hallard/RadioHead/tree/master/examples/raspi
[8]: https://github.com/hallard/RadioHead/tree/master/examples/raspi/irq_test
[9]: https://github.com/hallard/RadioHead/tree/master/examples/raspi/spi_scan

[10]: https://github.com/hallard/LoRasPI
[11]: https://github.com/ch2i/iC880A-Raspberry-PI
[12]: https://github.com/hallard/RPI-Lora-Gateway
[13]: https://github.com/dragino/Lora

[20]: https://github.com/hallard/RadioHead/tree/master/examples/raspi/rf69
[21]: https://github.com/hallard/RadioHead/tree/master/examples/raspi/rf95
[22]: https://github.com/hallard/RadioHead/tree/master/examples/raspi/nrf24
[23]: https://github.com/hallard/RadioHead/tree/master/examples/raspi/multi_server
[24]: https://github.com/hallard/RadioHead/tree/master/examples/raspi/RasPiBoards.h
[25]: https://github.com/hallard/RadioHead/tree/master/examples/raspi/rf95/rf95_client.cpp

[30]: https://github.com/raspberrypi/linux/issues/2550
[31]: https://groups.google.com/forum/#!topic/bcm2835/Y3D1mmp6vew
