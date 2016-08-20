RadioHead Packet Radio library for embedded microprocessors
===========================================================

This is a fork of the original RadioHead Packet Radio library for embedded microprocessors. It provides a complete object-oriented library for sending and receiving packetized messages via a variety of common data radios and other transports on a range of embedded microprocessors.

**Please read the full documentation and licensing from the original author [site][3]**

### features added with this fork
=================================

Here are the features added 

- Added moteino modem setting on RF69 to be compatible with lowpowerlab RF69 configuration library
- Added possibility to work with no IRQ connected for RF69 and RF95
  - for example do get one more GPIO free 
  - on Rasberry Pi, we do not have `attachInterrupt()` like with bcm2835 library
- Added samples for multiples Raspberry PI boards with RF69 and RF95 modules such as 
  - Simple RFM9x or RFM69HCW [LoRasPI][10] shield
  - iC880A or Linklabs Raspberry PI [shield][11] with RFM9x or RFM69HCW onboard 
  - Raspberry PI Lora [Gateway][12] with multiple RFM9x or RFM69HCW shield
  - Dragino Lora [shield][13]
  - Sample code are in [rf95][21], [rf69][20], [nrf24][22] and [multi_server][23], note that old sample NRF24 sample has been moved to nrf24 folder for consistency.
- Added 2 samples test tools (for Raspberry PI) do detect RF69 and RF95 modules and check IQR rising edge
  - [spi_scan][9] sample code, scan and try to detect connected modules
  - [irq_test][8] sample code, check a rising edge on a GPIO

Sample code for Raspberry PI is located under [RadioHead/examples/raspi][7] folder.

### Installation on Raspberry PI
================================

Clone repository
```shell
git clone https://github.com/hallard/RadioHead
```
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

If I'm doing same test with [PI Lora Gateway][12] with 2 RFM95 (one 433MHz and one 868MHz) and one RFMHW69 433MHz on board here are the results when trying to detect the onboard modules:

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

My setup is another Raspberry PI with [RFM95 868MHZ LoRasPI][10] shield running rf95_client sample and some [ULPnode][6] prototypes always running with on board RFM69 configured as Group ID 69 on 433MHz. I don't have a Lora 433MHz sender running so we won't receive anything on this one.


```shell
root@pi01(rw):~/RadioHead/examples/raspi/spi_scan# cd ../multi_server
root@pi01(rw):~/RadioHead/examples/raspi/multi_server# make
root@pi01(rw):~/RadioHead/examples/raspi/multi_server# ./multi_server
multi_server
============
1 RF95 868 (CS=GPIO8, IRQ=GPIO25, RST=GPIO5, LED=GPIO4) OK!, NodeID=1 @ 868.00MHz
2 RF95 433 (CS=GPIO7, IRQ=GPIO16, RST=GPIO6, LED=GPIO17) OK!, NodeID=1 @ 433.00MHz
3 RFM69HW 433 (CS=GPIO26, IRQ=GPIO23, RST=GPIO13, LED=GPIO19) OK!, NodeID=1 @ 433.00MHz
Listening for incoming packets...
13:34:57 Mod1 RF95 868 [10] #10 => #1 -54dB: Hi Raspi!
13:35:07 Mod1 RF95 868 [10] #10 => #1 -54dB: Hi Raspi!
13:35:16 Mod3 RFM69HW 433 [15] #18 => #1 -65dB:  11 20 CA 0B 21 CC 0B 24 EC 09 2C E2 02 40 00
13:35:17 Mod1 RF95 868 [10] #10 => #1 -54dB: Hi Raspi!
13:35:27 Mod1 RF95 868 [10] #10 => #1 -53dB: Hi Raspi!
13:35:37 Mod1 RF95 868 [10] #10 => #1 -54dB: Hi Raspi!
13:35:43 Mod3 RFM69HW 433 [21] #14 => #1 -60dB:  11 20 3F 04 21 3E 04 24 9C 09 2C 0D 04 40 00 25 9E 09 28 A8 02
13:35:47 Mod1 RF95 868 [10] #10 => #1 -54dB: Hi Raspi!
13:35:57 Mod3 RFM69HW 433 [15] #16 => #1 -61dB:  11 20 98 05 21 95 05 24 CE 09 2C 07 04 40 00
13:35:57 Mod1 RF95 868 [10] #10 => #1 -54dB: Hi Raspi!
13:36:07 Mod1 RF95 868 [10] #10 => #1 -54dB: Hi Raspi!
13:36:17 Mod1 RF95 868 [10] #10 => #1 -54dB: Hi Raspi!
13:36:27 Mod1 RF95 868 [10] #10 => #1 -54dB: Hi Raspi!
13:36:37 Mod1 RF95 868 [10] #10 => #1 -53dB: Hi Raspi!
13:36:48 Mod1 RF95 868 [10] #10 => #1 -54dB: Hi Raspi!
13:36:53 Mod1 RF95 868 [10] #10 => #1 -54dB: Hi Raspi!
13:36:58 Mod1 RF95 868 [10] #10 => #1 -53dB: Hi Raspi!
^C
Break received, exiting!
root@pi01(rw):~/RadioHead/examples/raspi/multi_server #
```
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
[4]: http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.61.zip
[5]: https://github.com/ch2i/RadioHead 
[6]: http://hallard.me/category/ulpnode/ 
[7]: https://github.com/hallard/RadioHead/tree/master/examples/raspi
[8]: https://github.com/hallard/RadioHead/tree/master/examples/raspi/irq_test
[9]: https://github.com/hallard/RadioHead/tree/master/examples/raspispi_scan

[10]: https://github.com/hallard/LoRasPI
[11]: https://github.com/ch2i/iC880A-Raspberry-PI
[12]: https://github.com/hallard/RPI-Lora-Gateway
[13]: https://github.com/dragino/Lora

[20]: https://github.com/hallard/RadioHead/tree/master/examples/rf69
[21]: https://github.com/hallard/RadioHead/tree/master/examples/rf95
[22]: https://github.com/hallard/RadioHead/tree/master/examples/nrf24
[23]: https://github.com/hallard/RadioHead/tree/master/examples/multi_server

