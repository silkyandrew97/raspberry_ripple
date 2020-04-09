# define the name of your source file(s)
SRCS =

# define the name of the object files(s) - we can do this automatically
OBJS = final.c

# tell MAKE which compiler to use
CCOMP = gcc

# flags for the compiler
CFLAGS =

# the name of your executable file (the target) - here we put it in the top directory
TARGET = final

# flags for the linker - note -lm for the math library
LDFLAGS = -lm -ljack

# actions
all: $(OBJS)
	$(CCOMP) $(OBJS) -o $(TARGET)  $(LDFLAGS)



# delete all objects and target
clean:
	rm -f $(OBJS) $(TARGET)
