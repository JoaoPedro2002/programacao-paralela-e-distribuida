#!/bin/bash
gcc -pthread client.c -o client;
gcc server.c -o server;
export N_SERVERS=8;
export WORD_SIZE=1000;


for ((i=0; i < $N_SERVERS; i++)); 
  do ./server $i; 
done;
