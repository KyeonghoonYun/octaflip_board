Welcome to Octaflip!!!

How to Compile???

sudo bash compile.sh

How to Execute???

board
./board

client
./client



(compile.sh)
make clean
make
g++ -Iinclude board.c ./lib/*.o -o board -D D
g++ -Iinclude board.c cJSON.c client.c ./lib/*.o -o client -lpthread
