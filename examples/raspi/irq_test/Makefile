# Makefile
# Sample for spi irq leve testing for Hope RF modules on Raspberry Pi
# Caution: requires bcm2835 library to be already installed
# http://www.airspayce.com/mikem/bcm2835/

CC            = g++
CFLAGS        = -DRASPBERRY_PI -DBCM2835_NO_DELAY_COMPATIBILITY
LIBS          = -lbcm2835 
RADIOHEADBASE = ../../..
INCLUDE       = -I$(RADIOHEADBASE)

all: irq_test

irq_test.o: irq_test.c
				$(CC) $(CFLAGS) -c $(INCLUDE) $<

irq_test: irq_test.o
				$(CC) $^ $(LIBS) -o irq_test

clean:
				rm -rf *.o irq_test
