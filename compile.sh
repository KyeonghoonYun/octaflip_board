echo "start"
# make clean
# make all

g++ -Iinclude board.c ./lib/*.o -o board -D D

#g++ -Iinclude board.c cjson.c client.c ./lib/*.o -o client -lpthread
