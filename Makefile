CC            = gcc
CFLAGS        = -O2 -march=native -Wall -I/usr/local/include
DEST          = /usr/local/bin
LDFLAGS       = -L/usr/local/lib
LIBS          = -lm
OBJS          = lcg.o main.o
PROGRAM       = lcg

all:		$(PROGRAM)

$(PROGRAM):	$(OBJS)
		$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $(PROGRAM)
		chmod 755 $(PROGRAM)

clean:
		rm -f *.o *~ $(PROGRAM)

install:	$(PROGRAM)
		install -s $(PROGRAM) $(DEST)

uninstall:	
		rm -f $(DEST)/$(PROGRAM)

clean-asm:
		rm -f *.s
