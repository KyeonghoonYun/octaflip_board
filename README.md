Welcome to Octaflip!!!

How to Compile
1. make
2. bash compile.sh
(
g++ -Iinclude board.c ./lib/*.o -o board -D D
g++ -Iinclude board.c cjson.c client.c ./lib/*.o -o client -lpthread
)