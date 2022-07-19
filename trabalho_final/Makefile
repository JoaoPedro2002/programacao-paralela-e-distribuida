CC           = gcc
LD           = gcc
AR           = ar
ARFLAGS      = rcs
CFLAGS       = -Wall -Os -c

SDFLAGS      = -Wall -Os -Wl,-Map,sequential.map

PFLAGS       = -Wall -Os -Wl,-Map,parallel.map

TFLAGS       = -Wall -OS -Wl,-Map,clienttcp.map
STFLAGS      = -Wall -OS -Wl,-Map,servertcp.map

UFLAGS       = -Wall -OS -Wl,-Map,clientudp.map
SUFLAGS      = -Wall -OS -Wl,-Map,serverudp.map

# include path to AVR library
INCLUDE_PATH = /usr/lib/avr/include
# splint static check
SPLINT       = splint test.c aes.c -I$(INCLUDE_PATH) +charindex -unrecog

default: all

.SILENT:
.PHONY:  lint clean

aes.o : aes.c aes.h
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o $@ $<


#Sequential
sequential.o : sequential.c aes.h aes.o
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o  $@ $<

sequential.elf : aes.o sequential.o
	echo [LD] $@
	$(LD) $(SDFLAGS) -o $@ $^


#Parallel
parallel.o : parallel.c aes.h aes.o 
	echo [CC] $@ $(CFLAGS) -pthread
	$(CC) -pthread $(CFLAGS) -o $@ $<

parallel.elf : aes.o parallel.o 
	echo [LD] $@
	$(LD) $(PFLAGS) -o $@ $^


#TCP
client_tcp.o: client_tcp.c aes.h aes.o
	echo [CC] $@ $(CFLAGS) -pthread
	$(CC) -pthread $(CFLAGS) -o $@ $<

client_tcp.elf: aes.o client_tcp.o
	echo [LD] $@
	$(LD) $(TFLAGS) -o $@ $^

server_tcp.o : server_tcp.c aes.h aes.o
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o $@ $<

server_tcp.elf: aes.o server_tcp.o
	echo [LD] $@
	$(LD) $(STFLAGS) -o $@ $^


#UDP
client_udp.o: client_udp.c aes.h aes.o
	echo [CC] $@ $(CFLAGS) -pthread
	$(CC) -pthread $(CFLAGS) -o $@ $<

client_udp.elf: aes.o client_udp.o
	echo [LD] $@
	$(LD) $(UFLAGS) -o $@ $^

server_udp.o : server_udp.c aes.h aes.o
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o $@ $<

server_udp.elf: aes.o server_udp.o
	echo [LD] $@
	$(LD) $(SUFLAGS) -o $@ $^

clean:
	rm -f *.OBJ *.LST *.o *.gch *.out *.hex *.map *.elf *.a

parallel:
	make clean && make parallel.elf && ./parallel.elf

sequential:
	make clean && make sequential.elf && ./sequential.elf

tcp:
	make clean && make client_tcp.elf && make server_tcp.elf

udp:
	make clean && make client_udp.elf && make server_udp.elf

all:
	make clean && make sequential.elf && ./sequential.elf
	make clean && make parallel.elf && ./parallel.elf
	make clean && make udp && make tcp

lint:
	$(call SPLINT)