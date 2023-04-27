# Author: Nikolas Nosál, (xnosal01@stud.fit.vutbr.cz)
# Brief: Makefile for Projekt 2 (synchronizace).
# How to use: [ $ make ] or [ $ make clean ]

# tool macros
CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
CLIBS = -pthread -lrt

# path macros
EXE = proj2
SRC = proj2.c

# compile macros
$(EXE): $(SRC) process_table.o
	$(CC) $(CFLAGS) -o $(EXE) $(SRC) process_table.o $(CLIBS)

# compile process_table
process_table.o: process_table.c process_table.h
	$(CC) $(CFLAGS) -c process_table.c

# clean
clean:
	rm -f $(EXE) $(SRC:.c=.o) process_table.o