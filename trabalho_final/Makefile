CC           = gcc
LD           = gcc
AR           = ar
ARFLAGS      = rcs
CFLAGS       = -Wall -Os -c
SDFLAGS      = -Wall -Os -Wl,-Map,sequential.map
PFLAGS       = -Wall -Os -Wl,-Map,parallel.map

OBJCOPYFLAGS = -j .text -O ihex
OBJCOPY      = objcopy

# include path to AVR library
INCLUDE_PATH = /usr/lib/avr/include
# splint static check
SPLINT       = splint test.c aes.c -I$(INCLUDE_PATH) +charindex -unrecog

default: test.elf

.SILENT:
.PHONY:  lint clean

aes.o : aes.c aes.h
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o $@ $<


sequential.o : sequential.c aes.h aes.o
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o  $@ $<

sequential.elf : aes.o sequential.o
	echo [LD] $@
	$(LD) $(SDFLAGS) -o $@ $^


parallel.o : parallel.c aes.h aes.o 
	echo [CC] $@ $(CFLAGS) -pthread
	$(CC) -pthread $(CFLAGS) -o $@ $<

parallel.elf : aes.o parallel.o 
	echo [LD] $@
	$(LD) $(PFLAGS) -o $@ $^


clean:
	rm -f *.OBJ *.LST *.o *.gch *.out *.hex *.map *.elf *.a

parallel:
	make clean && make parallel.elf && ./parallel.elf

sequential:
	make clean && make sequential.elf && ./sequential.elf

all:
	make clean && make sequential.elf && ./sequential.elf
	make clean && make parallel.elf && ./parallel.elf

lint:
	$(call SPLINT)