echo "start"
# make clean
# make all

g++ -Iinclude ./lib/board.c ./lib/*.o -o main -D D

#g++ -Iinclude ./lib/board.c client.c ./lib/*.o -o main
