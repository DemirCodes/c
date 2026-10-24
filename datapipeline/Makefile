CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = pipeline
OBJS = main.o veri.o kurallar.o sifreleme.o buffer.o yonlendirme.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all run clean