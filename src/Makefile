# Start of makefile
# Define dependencies
IDIR = ../include
ODIR = obj;
# Define source files
SRCS = compressor.c interface.c main.c overdrive.c
# Define object files
# Define compiler
CCOMP = gcc
# Define compiler flags
CFLAGS = -Wall -O3 -I$(INCLUDE)
# Define target (will create usr/bin folder)
TARGET = ../usr/bin/raspberry_ripple
# Define linker flags
LDFLAGS = -lm -ljack
# Actions
_INCLUDE = compressor.h interface.h overdrive.h
INCLUDE = $(patsubst %,$(IDIR)/%,$(_INCLUDE))

_OBJ = compressor.o interface.o main.o overdrive.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: $(OBJS)
	$(CCOMP) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJS)/%.o: %.c $(INCLUDE)
	$(CCOMP) -c -o $@ $< $(CFLAGS)
# Delete all objects and target
clean:
	rm -f $(ODIR)/*.o $(TARGET)
# End of makefile
