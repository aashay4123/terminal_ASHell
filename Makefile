CC = gcc
CFLAGS = -Wall -Wextra -pedantic

AShell: AShell.c
	$(CC) $(CFLAGS) -o AShell AShell.c

clean:
	rm -f AShell
