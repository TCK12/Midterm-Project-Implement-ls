CC = gcc
CFLAGS = -Wall -Wextra -Werror -g
TARGET = my_ls
OBJS = main.o options.o core.o display.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)