echo "start"
# make clean
# make all

g++ -c -Iinclude ./lib/board.c -o ./lib/board.o
g++ -c -Iinclude main.c -o main.o
g++ -c -Iinclude coordinates.c -o coordinates.o
echo "compile successed"

g++ main.o ./lib/*.o -o main
g++ coordinates.o ./lib/*.o -o coordinates
echo "link successed"