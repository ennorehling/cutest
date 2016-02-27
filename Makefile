# I am not very good at Makefiles.

CFLAGS += -Wall -g
INCLUDES = -I.

all: bin/CuTestTest

bin:
	mkdir -p $@

test: bin/CuTestTest
	@bin/CuTestTest

bin/CuTestTest: AllTests.c CuTestTest.c CuTest.c | bin
	$(CC) $(CFLAGS) $(INCLUDES) -Wno-address -lm -o $@ $^

clean:
	@rm -rf *~ bin
