# Which compiler to use
CC = gcc
# Compiler flags
CFLAGS = -m32 -I. -Og -g -Werror=vla
# Final executable
LIBSO = libvm.so
TARGET = vmtest
# Constituent object files
OBJS = vminit.o vmalloc.o vmfree.o utils.o
OBJTEST = vmtest.o

# Default Make recipe
default: $(TARGET)

$(OBJS): %.o : %.c
	$(CC) -c $(CFLAGS) -Wall -fpic $<

$(LIBSO): $(OBJS)
	$(CC) -shared $(CFLAGS) -Wall -o $(LIBSO) $(OBJS)

$(TARGET): $(LIBSO) $(OBJTEST)
	$(CC) $(CFLAGS) -Wno-unused-variable -Xlinker -rpath=. -o $(TARGET) $(OBJTEST) -L. -lvm 

# Clean recipe: removes all build artifacts
clean:
	$(RM) $(TARGET) $(OBJS) $(LIBSO) $(OBJTEST)
