echo "start"
# make clean
# make all

g++ -c -Iinclude ./lib/board.c -o ./lib/board.o
g++ -c -Iinclude temp.c -o temp.o
echo "compile successed"

g++ temp.o ./lib/*.o -o temp
echo "link successed"

./temp