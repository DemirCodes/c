CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lssl -lcrypto

TARGET = pipeline
OBJS = main.o veri.o kurallar.o sifreleme.o buffer.o yonlendirme.o sinyal.o hata.o logrotasyon.o aes.o cli.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test_aes: test_aes.c aes.o
	$(CC) $(CFLAGS) -o test_aes test_aes.c aes.o $(LDFLAGS)

test: test_aes
	./test_aes

clean:
	rm -f $(TARGET) $(OBJS) test_aes

.PHONY: all test clean