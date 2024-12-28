CC=gcc
CFLAGS=-Wall -Wextra -g

all: test_mock

test_mock: test_mock.o mock_uefi.o
	$(CC) $(CFLAGS) -o test_mock test_mock.o mock_uefi.o

test_mock.o: test_mock.c mock_uefi.h
	$(CC) $(CFLAGS) -c test_mock.c

mock_uefi.o: mock_uefi.c mock_uefi.h
	$(CC) $(CFLAGS) -c mock_uefi.c

clean:
	rm -f test_mock *.o

test: test_mock
	./test_mock

.PHONY: all clean test 