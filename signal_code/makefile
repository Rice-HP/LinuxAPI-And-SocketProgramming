src_file = $(wildcard *.c)
des_file = $(patsubst %.c, %,$(src_file))

all:$(des_file)
	
%:%.c
	gcc -o $@ $^ -g -std=gnu99

.PHONY clean:
	rm -f $(des_file)

