CC           = gcc
LD           = gcc
AR           = ar
ARFLAGS      = rcs
CFLAGS       = -Wall -Os -c
LDFLAGS      = -Wall -Os -Wl,-Map,test.map
MFLAGS       = -Wall -Os -Wl,-Map,main.map
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

test.hex : test.elf
	echo copy object-code to new image and format in hex
	$(OBJCOPY) ${OBJCOPYFLAGS} $< $@

test.o : test.c aes.h aes.o
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o  $@ $<

main.o : main.c aes.h aes.o 
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o $@ $<

parallel.o : parallel.c aes.h aes.o 
	echo [CC] $@ $(CFLAGS) -pthread
	$(CC) -pthread $(CFLAGS) -o $@ $<
aes.o : aes.c aes.h
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o $@ $<

test.elf : aes.o test.o
	echo [LD] $@
	$(LD) $(LDFLAGS) -o $@ $^

main.elf : aes.o main.o
	echo [LD] $@
	$(LD) $(MFLAGS) -o $@ $^

parallel.elf : aes.o parallel.o 
	echo [LD] $@
	$(LD) $(PFLAGS) -o $@ $^
clean:
	rm -f *.OBJ *.LST *.o *.gch *.out *.hex *.map *.elf *.a

test:
	make clean && make && ./test.elf

lint:
	$(call SPLINT)