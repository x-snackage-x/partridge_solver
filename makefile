IDIR=include

CC=cc
CFLAGS=-Wall
INC=-I$(IDIR)

ODIR=obj
SDIR=src

LIBS=-lc

_DEPS=elhaylib.h vis.h puz.h 
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))
     
_OBJS=elhaylib.o vis.o puz.o 
OBJS=$(patsubst %,$(ODIR)/%,$(_OBJS))

obj/%.o: $(SDIR)/%.c $(DEPS) | obj
	$(CC) -c $(INC) -o $@ $< $(CFLAGS) 

obj:
	mkdir -p $@

vis: $(OBJS)
	$(CC) -o vis.e $(ODIR)/vis.o $(CFLAGS) $(LIBS)

puz: $(OBJS)
	$(CC) -o puz.e $(ODIR)/puz.o $(ODIR)/elhaylib.o $(CFLAGS) $(LIBS)

solver: $(OBJS)
	$(CC) -o sol.e $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 