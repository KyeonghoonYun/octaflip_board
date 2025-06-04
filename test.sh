echo "start"
# make clean
# make all

g++ -DDD -Iinclude ./lib/board.c ./lib/*.o -o main

#g++ -Iinclude ./lib/board.c client.c ./lib/*.o -o main
