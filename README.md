Welcome to Octaflip!!!

How to Compile???

sudo bash compile.sh




make
g++ -Iinclude board.c ./lib/*.o -o board -D D
g++ -Iinclude board.c cJSON.c client.c ./lib/*.o -o client -lpthread
