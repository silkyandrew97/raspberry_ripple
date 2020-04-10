# Start of makefile
# Define directories (Create subdirectories for object files)
IDIR := include
IDIR_TEST := test
ODIR := src/obj
ODIR_TEST := test/obj
# Define compiler
CC := gcc
# Define compiler flags
CFLAGS := -Wall -O3 -I$(IDIR) -g
CFLAGS_TEST := -Wall -O3 -I$(IDIR) -I$(IDIR_TEST) -g
# Define linker flags
LIBS := -lm -ljack
# Define targets (will create usr/bin folder)
TARGET := usr/bin/raspberry_ripple
TARGET_COMPRESSOR := usr/bin/test_compressor
TARGET_OVERDRIVE := usr/bin/test_overdrive
TARGET_TOGETHER := usr/bin/test_together
# Define paths to .o and .h files
_DEPS := compressor.h interface.h overdrive.h
DEPS := $(patsubst %,$(IDIR)/%,$(_DEPS))
_DEPS_TEST := test.h
DEPS_TEST := $(DEPS) $(patsubst %,$(IDIR_TEST)/%,$(_DEPS_TEST))
_OBJS := compressor.o interface.o main.o overdrive.o
OBJS := $(patsubst %,$(ODIR)/%,$(_OBJS))
_OBJS_TEST := test_compressor.o test_overdrive.o test_together.o
OBJS_TEST := $(patsubst %,$(ODIR_TEST)/%,$(_OBJS_TEST))
# Make all
all: $(OBJS) $(OBJS_TEST)
	$(CC) $(OBJS) -o $(TARGET) $(CFLAGS) $(LIBS)
	$(CC) $(OBJS_TEST) -o $(TARGET_COMPRESSOR) $(CFLAGS_TEST) $(LIBS)
	$(CC) $(OBJS_TEST) -o $(TARGET_OVERDRIVE) $(CFLAGS_TEST) $(LIBS)
	$(CC) $(OBJS_TEST) -o $(TARGET_TOGETHER) $(CFLAGS_TEST) $(LIBS)
# Build objects
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR_TEST)/%.o: %.c $(DEPS_TEST)
	$(CC) -c -o $@ $< $(CFLAGS_TEST)
# Make clean (delete all objects and target)
clean:
	rm -f $(ODIR)/*.o $(TARGET) $(ODIR_TEST)/*.o $(TARGET_COMPRESSOR) $(TARGET_OVERDRIVE) $(TARGET_TOGETHER)
# End of makefile
