CC = gcc

CCFLAGS = -lauto -lraauto -mcrt=newlib -Wall -pedantic -O3

VBAGUI:
	$(CC) source/gui.c -o vbagui $(CCFLAGS)

