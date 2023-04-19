# tool macros
# add to CFLAGS later -> -std=gnu99 -Wall -Wextra -Werror -pedantic
CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -pedantic
CLIBS = -pthread -lrt

# path macros
EXE = proj2
SRC = proj2.c

# compile macros
$(EXE): $(SRC) process_table.o
	$(CC) $(CFLAGS) -o $(EXE) $(SRC) process_table.o $(CLIBS)

process_table.o: process_table.c process_table.h
	$(CC) $(CFLAGS) -c process_table.c

clean:
	rm -f $(EXE) $(SRC:.c=.o) process_table.o