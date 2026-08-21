
CC = gcc
CFLAGS = -Wall -g -Og
LDLIBS = -lpthread

TARGET = myshell
OBJS = myshell.c csapp.c

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)

clean:
	rm -f $(TARGET) *~
