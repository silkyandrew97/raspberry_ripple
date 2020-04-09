# Start of makefile
# Define directories (Create subdirectory for object files)
IDIR = ../include
ODIR = ../src/obj;
# Define compiler
CCOMP = gcc
# Define compiler flags
CFLAGS = -Wall -O3 -I$(IDIR) -I.
# Define linker flags
LDFLAGS = -lm -ljack
# Define targets (will create usr/bin folder)
TARGET_COMPRESSOR = ../usr/bin/test_compressor
TARGET_OVERDRIVE = ../usr/bin/test_overdrive
TARGET_TOGETHER = ../usr/bin/test_together
# Define paths to directories
_INCLUDE = compressor.h interface.h overdrive.h
INCLUDE = $(patsubst %,$(IDIR)/%,$(_INCLUDE))
_OBJS = compressor.o interface.o overdrive.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))
# Make all
all: $(OBJS)
    $(CCOMP) $(OBJS) test_compressor.o -o $(TARGET_COMPRESSOR) $(LDFLAGS)
    $(CCOMP) $(OBJS) test_overdrive.o -o $(TARGET_OVERDRIVE) $(LDFLAGS)
    $(CCOMP) $(OBJS) test_together.o -o $(TARGET_TOGETHER) $(LDFLAGS)
# Build objects
$(ODIR)/%.o: %.c $(INCLUDE) test.h
    $(CCOMP) -c -o $@ $< $(CFLAGS)
# Make clean (delete all objects and target)
clean:
    rm -f $(ODIR)/*.o $(TARGET_COMPRESSOR) $(TARGET_OVERDRIVE) $(TARGET_TOGETHER)
# End of makefile
