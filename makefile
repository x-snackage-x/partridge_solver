IDIR=include
SDIR=src
CC=cc
CFLAGS=-Wall -ggdb
INC=-I$(IDIR)
LIBS=-lc

# Headers
_DEPS=elhaylib.h vis.h puz.h sol.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

# Default
all: vis puz sol

# --------------------
# VIS
# --------------------
VIS_ODIR=obj/vis
VIS_OBJS=$(VIS_ODIR)/elhaylib.o $(VIS_ODIR)/vis.o

$(VIS_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(VIS_ODIR)
	$(CC) -c $(INC) $(CFLAGS) -DBUILD_VIS $< -o $@

$(VIS_ODIR):
	mkdir -p $@

vis: $(VIS_OBJS)
	$(CC) -o vis.e $^ $(LIBS)

# --------------------
# PUZ
# --------------------
PUZ_ODIR=obj/puz
PUZ_OBJS=$(PUZ_ODIR)/elhaylib.o $(PUZ_ODIR)/puz.o

$(PUZ_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(PUZ_ODIR)
	$(CC) -c $(INC) $(CFLAGS) -DBUILD_PUZ $< -o $@

$(PUZ_ODIR):
	mkdir -p $@

puz: $(PUZ_OBJS)
	$(CC) -o puz.e $^ $(LIBS)

# --------------------
# SOL
# --------------------
SOL_ODIR=obj/sol
SOL_OBJS=$(SOL_ODIR)/elhaylib.o $(SOL_ODIR)/vis.o $(SOL_ODIR)/puz.o $(SOL_ODIR)/sol.o

$(SOL_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(SOL_ODIR)
	$(CC) -c $(INC) $(CFLAGS) $< -o $@

$(SOL_ODIR):
	mkdir -p $@

sol: $(SOL_OBJS)
	$(CC) -o sol.e $^ $(LIBS)

# --------------------
clean:
	rm -rf obj *.e
