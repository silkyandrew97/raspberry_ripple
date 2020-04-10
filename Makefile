# Start of makefile
# Define directories
SRC := src
SRC_TEST := test
IDIR := include
IDIR_TEST := test
ODIR := src/obj
ODIR_TEST := test/obj
TDIR :=usr/bin
# Define compiler
CC := gcc
# Define compiler flags
CFLAGS := -Wall -O3 -I$(IDIR) -g
CFLAGS_TEST := -Wall -O3 -I$(IDIR) -I$(IDIR_TEST) -g
# Define linker flags
LIBS := -lm -ljack
# Define targets
_TARGETS := raspberry_ripple test_compressor test_overdrive test_together
TARGETS = $(patsubst %,$(TDIR)/%,$(_TARGETS))
# Define paths to .o and .h files
_DEPS := compressor.h interface.h overdrive.h
DEPS := $(patsubst %,$(IDIR)/%,$(_DEPS))
_DEPS_TEST := test.h
DEPS_TEST := $(patsubst %,$(IDIR_TEST)/%,$(_DEPS_TEST))
_OBJS := compressor.o interface.o overdrive.o
OBJS := $(patsubst %,$(ODIR)/%,$(_OBJS))
_OBJS_TEST := test_compressor.o test_overdrive.o test_together.o
OBJS_TEST := $(patsubst %,$(ODIR_TEST)/%,$(_OBJS_TEST))
# Make all
all: $(OBJS) $(OBJS_TEST)
	$(CC) $(OBJS) $(ODIR)/main.o -o $(TDIR)/raspberry_ripple $(CFLAGS) $(LIBS)
	$(CC) $(OBJS) $(ODIR_TEST)/test_compressor.o -o $(TDIR)/test_compressor $(CFLAGS_TEST) $(LIBS)
	$(CC) $(OBJS) $(ODIR_TEST)/test_overdrive.o -o $(TDIR)/test_overdrive $(CFLAGS_TEST) $(LIBS)
	$(CC) $(OBJS) $(ODIR_TEST)/test_together.o -o $(TDIR)/test_together $(CFLAGS_TEST) $(LIBS)
# Build objects
$(ODIR)/%.o: $(SRC)/%.c $(DEPS) 
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR_TEST)/%.o: $(SRC_TEST)/%.c $(DEPS) $(DEPS_TEST)
	$(CC) -c -o $@ $< $(CFLAGS_TEST)
# Make clean (delete all objects and target)
clean:
	rm -f $(ODIR)/*.o $(ODIR_TEST)/*.o $(TARGETS)
# End of makefile
