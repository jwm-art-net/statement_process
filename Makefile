#----------------user/install options----------------
# alter only if you're not clueless and want XorCurses
# installed in some other location or you're packaging
# it:

PREFIX=/usr/local/
BINDIR=$(PREFIX)bin/

CFLAGS:= -Wall -Wextra

CFLAGS:=$(CFLAGS) -O0 -ggdb #-DDEBUG

PROG:=statement_process
CC:=gcc

SRC    :=$(wildcard *.c)
OBJS   :=$(patsubst %.c, %.o, $(SRC))
HEADERS:=$(wildcard *.h)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROG) $(LIBS)
	
%.o : %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

main.o: main.c $(HEADERS)

clean:
	rm -f $(PROG) $(OBJS)

install: $(PROG)
	install $(PROG) $(BINDIR)

uninstall:
	rm -f $(BINDIR)$(PROG)

