sqlite: sqlite_interface.o sqlite3.o
	gcc -lpthread -ldl -o sqlite sqlite_interface.o sqlite3.o

sqlite_interface.o: sqlite_interface.c sqlite3.h
	gcc -c sqlite_interface.c

sqlite3.o: sqlite3.c sqlite3.h
	gcc -c sqlite3.c

clean:
	rm -f sqlite sqlite_interface.o sqlite3.o
