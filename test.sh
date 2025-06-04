echo "start"
# make clean
# make all

g++ -c -Iinclude ./lib/board.c -o main.o -DDD

g++ main.o ./lib/*.o -o main

g++ -c -Iinclude ./lib/board.c -o ./lib/board.o