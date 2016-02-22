all: Rijndael

Rijndael:
	gcc -c Rijndael/rijndael.c -o bin/rijndael.o
	ar rcs bin/libRijndael.a bin/rijndael.o

#PsarcReader:
#	g++
#Settings:
#	g++
#Wem:
#	g++
#Audio:
#	g++
#opensmith:
#	g++
#Tuner:
#	g++
#Setup:
#	g++
#Test:
#	g++
