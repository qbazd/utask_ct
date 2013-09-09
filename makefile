GCC=gcc
CFLAGS=-Os -Wall -pedantic -std=c99
UTASK_DIR=../utask

all: examples

clean: 
	rm utask_ct_example_1

examples:
	$(GCC) $(CFLAGS) -I$(UTASK_DIR) $(UTASK_DIR)/utask.c  utask_ct_example_1.c -o utask_ct_example_1 -lpthread 

