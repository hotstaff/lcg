CC            = gcc
CXX           = g++
CFLAGS        = -O2 -Wall -I/usr/local/include
CXXFLAGS      = -Wall -I/usr/local/include
LDFLAGS       = -L/usr/local/lib
LIBS          = -lm
OBJS          = lcg.o 
PROGRAM       = lcg

all:		$(PROGRAM)

$(PROGRAM):	$(OBJS)
		$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $(PROGRAM)
		chmod 755 $(PROGRAM)

clean:
		rm -f *.o *~ $(PROGRAM)
