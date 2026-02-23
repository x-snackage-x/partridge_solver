IDIR=include
SDIR=src
CC=cc
DEBUG_FLAGS = -g3 -Og -fno-omit-frame-pointer
CFLAGS=-Wall $(DEBUG_FLAGS)
PROD_FLAGS = -O2
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
	$(CC) -o vis.out $^ $(LIBS)

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
	$(CC) -o puz.out $^ $(LIBS)

# --------------------
# SOL
# --------------------

# Dev build
SOL_ODIR=obj/sol
SOL_OBJS= $(SOL_ODIR)/elhaylib.o \
        	$(SOL_ODIR)/vis.o \
        	$(SOL_ODIR)/puz.o \
        	$(SOL_ODIR)/sol.o

$(SOL_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(SOL_ODIR)
	$(CC) -c $(INC) $(CFLAGS) $< -o $@

$(SOL_ODIR):
	mkdir -p $@

sol: $(SOL_OBJS)
	$(CC) -o sol.out $^ $(LIBS)

# Prod Linux build 
sol_prod:CFLAGS = -Wall $(PROD_FLAGS)
SOL_PROD_ODIR=obj/sol_prod
SOL_OBJS= $(SOL_PROD_ODIR)/elhaylib.o \
		    $(SOL_PROD_ODIR)/vis.o \
		    $(SOL_PROD_ODIR)/puz.o \
		    $(SOL_PROD_ODIR)/sol.o

$(SOL_PROD_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(SOL_PROD_ODIR)
	$(CC) -c $(INC) $(CFLAGS) $< -o $@

$(SOL_PROD_ODIR):
	mkdir -p $@

sol_prod: $(SOL_OBJS)
	$(CC) -o sol_prod.out $^ $(LIBS)

# Prod MSVC Windows build
sol_prod_win:
	cl /O2 /I$(IDIR) /Fe:sol_prod.exe \
	   $(SDIR)/elhaylib.c \
	   $(SDIR)/vis.c \
	   $(SDIR)/puz.c \
	   $(SDIR)/sol.c

# --------------------
clean:
	rm -rf obj *.out
