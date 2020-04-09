# Start of makefile
# Define directories (Create subdirectory for object files)
IDIR = ../include
ODIR = ../src/obj;
# Define compiler
CCOMP = gcc
# Define compiler flags
CFLAGS = -Wall -O3 -I$(IDIR)
# Define linker flags
LDFLAGS = -lm -ljack
# Define targets (will create usr/bin folder)
TARGET = ../usr/bin/raspberry_ripple
# Define paths to directories
_INCLUDE = compressor.h interface.h overdrive.h
INCLUDE = $(patsubst %,$(IDIR)/%,$(_INCLUDE))
_OBJS = compressor.o interface.o main.o overdrive.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))
# Make all
all: $(OBJS)
    $(CCOMP) $(OBJS) -o $(TARGET) $(LDFLAGS)
# Build objects
$(ODIR)/%.o: %.c $(INCLUDE)
    $(CCOMP) -c -o $@ $< $(CFLAGS)
# Make clean (delete all objects and target)
clean:
    rm -f $(ODIR)/*.o $(TARGET)
# End of makefile
