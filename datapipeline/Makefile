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

test: $(TARGET) test_aes
	@echo "Testler calisiyor..."
	@./test_aes
	@bash tests/test_pipeline.sh

test_aes: test_aes.c aes.o sifreleme.o
	$(CC) $(CFLAGS) -o test_aes test_aes.c aes.o sifreleme.o $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJS) test_aes

.PHONY: all test clean