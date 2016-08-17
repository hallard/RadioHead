# Makefile
# Sample for rpi_lora_gw on Raspberry Pi
# Caution: requires bcm2835 library to be already installed
# http://www.airspayce.com/mikem/bcm2835/

CC            = g++
CFLAGS        = -DRASPBERRY_PI -DBCM2835_NO_DELAY_COMPATIBILITY
LIBS          = -lbcm2835
RADIOHEADBASE = ../../..
INCLUDE       = -I$(RADIOHEADBASE)

all: rpi_lora_gw

RasPi.o: $(RADIOHEADBASE)/RHutil/RasPi.cpp
				$(CC) $(CFLAGS) -c $(RADIOHEADBASE)/RHutil/RasPi.cpp $(INCLUDE)

RH_RF69.o: $(RADIOHEADBASE)/RH_RF69.cpp
				$(CC) $(CFLAGS) -c $(INCLUDE) $<

RH_RF95.o: $(RADIOHEADBASE)/RH_RF95.cpp
				$(CC) $(CFLAGS) -c $(INCLUDE) $<

RHHardwareSPI.o: $(RADIOHEADBASE)/RHHardwareSPI.cpp
				$(CC) $(CFLAGS) -c $(INCLUDE) $<

RHSPIDriver.o: $(RADIOHEADBASE)/RHSPIDriver.cpp
				$(CC) $(CFLAGS) -c $(INCLUDE) $<

RHGenericDriver.o: $(RADIOHEADBASE)/RHGenericDriver.cpp
				$(CC) $(CFLAGS) -c $(INCLUDE) $<

RHGenericSPI.o: $(RADIOHEADBASE)/RHGenericSPI.cpp
				$(CC) $(CFLAGS) -c $(INCLUDE) $<

rpi_lora_gw.o: rpi_lora_gw.cpp
				$(CC) $(CFLAGS) -c $(INCLUDE) $<

rpi_lora_gw: rpi_lora_gw.o RasPi.o RHHardwareSPI.o RH_RF69.o RH_RF95.o RHSPIDriver.o RHGenericDriver.o RHGenericSPI.o
				$(CC) $^ $(LIBS) -o rpi_lora_gw

clean:
				rm -rf *.o rpi_lora_gw
