IDIR=include

CC=cc
CFLAGS=-Wall 
INC=-I$(IDIR)

ODIR=obj
SDIR=src

LIBS=-lc

_DEPS=vis.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))
     
_OBJS=vis.o 
OBJS=$(patsubst %,$(ODIR)/%,$(_OBJS))

obj/%.o: $(SDIR)/%.c $(DEPS) | obj
	$(CC) -c $(INC) -o $@ $< $(CFLAGS) 

obj:
	mkdir -p $@

vis: $(OBJS)
	$(CC) -o vis $(ODIR)/vis.o $(CFLAGS) $(LIBS)

solver: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 