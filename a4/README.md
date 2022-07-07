### Compilar: 
```bash
bash compile.sh
```

### Executar

Executar n servers em n terminais.
A varíavel `N_SERVERS` do client.c deve ser igual ao número de servers instanciados
Cada server deve ser instanciado com um id. A contagem deve começar a partir do 0. Exemplo:
```bash
./server 0
```
```bash
./server 1
```
```bash
./server 2
```
```bash
./server 3
```

Executar em outro terminal o código do cliente. Exemplo:
```bash
./client 10000
```

