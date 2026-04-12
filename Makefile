CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
TARGET = test_energy

all: $(TARGET)

$(TARGET): energy.c test_energy.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

clean:
	rm -f $(TARGET)

test: $(TARGET)
	./$(TARGET)
