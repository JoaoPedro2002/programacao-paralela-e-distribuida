## Paralelização e Distribuição do CTR mode
### Executar

#### Sequencial, Paralelo e MPI
```bash
make sequential
# ou
make parallel
# ou
make mpi
```
#### Sockets
```bash
make tcp
# ou
make udp
```
Devem ser iniciados os servidores contando a partir do 0 até o valor de `N_SERVERS - 1` em `client_tcp.c` ou `client_udp.c`.
```bash
./server_tcp.elf 0
./server_tcp.elf 1
...
# ou
./server_udp.elf 0
./server_udp.elf 1
...
```

Por fim, executar o cliente:
```bash
./client_tcp.elf
# ou
./client_udp.elf
```
### TODO

* [x] Implementar AES 
* [x] Paralelo
* [x] Distribuído com sockets
  * [x] TCP
  * [x] UDP
* [x] MPI