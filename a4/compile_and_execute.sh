#!/bin/bash
# gcc -pthread client.c -o client;
# gcc server.c -o server;

for i in {0..8}; 
  do ./server $i; 
done;
