SRCS=$(wildcard *.c)

OEXECUTABLES=$(SRCS:.c=.out)

all: compile
compile: $(OEXECUTABLES)

%.out: %.c
	gcc $< -std=c99 -lpthread -o $@ -O3

clean:
	rm -rf $(OEXECUTABLES)
