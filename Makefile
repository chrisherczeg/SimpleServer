.PHONY: Simple_Server
Simple_Server:
	gcc -g -Wall -Werror -pthread -o Simple_Server -ldl -lrt main.c
