echo "start"
# make clean
# make all

g++ -c -Iinclude ./lib/board.c -o ./lib/board.o
g++ -c -Iinclude temp.c -o temp.o
g++ -c -Iinclude coordinates.c -o coordinates.o
echo "compile successed"

g++ temp.o ./lib/*.o -o temp
g++ coordinates.o ./lib/*.o -o coordinates
echo "link successed"