echo "start"
# make clean
# make all

g++ -Iinclude ./lib/board.c -DDD ./lib/*.o -o

#g++ -Iinclude ./lib/board.c client.c ./lib/*.o -o main
