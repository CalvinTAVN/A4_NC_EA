#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3 implementation
# Author: Calvin Nguyen & Alex Eng
#--------------------------------------------------------------------

#---------------------------------------------
#advanced version (based on Makefile.sampleft)
#---------------------------------------------

#CC=gcc217

#all: ft

#clean:
#	rm -f ft

#clobber: clean
#	rm -f ft_client.o dynarray.o *~

#ft: dynarray.o ft.o ft_client.o
#	$(CC) dynarray.o ft.o ft_client.o -o ft

#---------------------------------------------
#simple version (based on Makefile for A3)
#---------------------------------------------

all: ft
clean:
	rm -f ft
clobber: clean
	rm -f ft_client.o dynarray.o node.o *~

ft: dynarray.o node.o ft.o ft_client.o
	gcc217 dynarray.o node.o ft.o ft_client.o -o ft
dynarray.o: dynarray.c dynarray.h
	gcc217 -c -g dynarray.c
node.o: node.c node.h
	gcc217 -c -g node.c
ft.o: ft.c ft.h a4def.h
	gcc217 -c -g ft.c
ft_client.o: ft_client.c
	gcc217 -c -g ft_client.c