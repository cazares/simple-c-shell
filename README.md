# simple-c-shell

Lightweight commandline shell that only works on linux right now. This is just for practicing some C programming.

Allows for 1 foreground process and 20 background processes. Note: if using the 'cat' command, this program will use a custom implementation that is meant to behave the same way that the bash cat behaves.

## usage
	gcc -o psh psh.c
	./psh [OPTIONAL ARGUMENT]

	optional argument: prompt to be printed

	type 'exit' to exit
