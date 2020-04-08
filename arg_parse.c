#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "arg_parse.h"

//helper function for arg_parse that counts the number of arguments in array
int argument_counter(char *line) {
	int totalargs = 0;
	int index = 0;
	while (line[index] != '\0') {
		if (line[index] == ' ') {
			index++;
			continue;
		}
		totalargs++;
		while (line[index] != '\0' && line[index] != ' ') {
			index++;
		}
	}
	return totalargs;
}

//this function returns an array of pointers that point to the start of each of the arguments
char** arg_parse(char *line, int *argcp) {
	int argument_count = argument_counter(line);
	*argcp = argument_count;

	char** dynamicArray = malloc((argument_count + 1) * sizeof(char*));

	if (dynamicArray == NULL) {
		fprintf(stderr, "Failed to allocate");
	}
	int lineIndex = 0;
	int mallocIndex = 0;

	if (isalpha(line[0])) {
		dynamicArray[mallocIndex] = &line[lineIndex];
		lineIndex++;
		mallocIndex++;
	}

	while (line[lineIndex] != '\0') {
		if (isspace(line[lineIndex])) {
			line[lineIndex] = '\0';
		}

		if (isalpha(line[lineIndex]) || line[lineIndex] == '-'
				|| line[lineIndex] == '/' || isdigit(line[lineIndex])
				|| line[lineIndex] == '>' || line[lineIndex] == '<') {
			if (line[lineIndex - 1] == '\0') {
				dynamicArray[mallocIndex] = &line[lineIndex];
				mallocIndex++;
			}
		}
		lineIndex++;
	}

	dynamicArray[mallocIndex] = '\0';
	return dynamicArray;
}