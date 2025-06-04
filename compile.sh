echo "compile start"
#make clean
#make all

g++ -Iinclude board.c ./lib/*.o -o board -D D

g++ -Iinclude board.c cJSON.c client.c ./lib/*.o -o client -lpthread

echo "compile finish"