/*
Archan Rupela
Micro-make program
*/

/*
All Sources:
http://www.zentut.com/c-tutorial/c-linked-list/
https://www.learn-c.org/en/Linked_lists
https://www.opentechguides.com/how-to/article/c/141/linkedlist-add-del-print-count.html
https://overiq.com/c-programming/101/the-strcpy-function-in-c/
http://www.informit.com/articles/article.aspx?p=23618&seqNum=3
https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "target.h"
#include "arg_parse.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* CONSTANTS */

/* PROTOTYPES */
char** arg_parse(char *line, int *argcp);
void processline(char* line);
int expand(char *orig, char *new, int newSize);
char** IO_redirection(char** newArray);

/* Process Line
 * line   The command line to execute.
 * This function interprets line as a command line.  It creates a new child
 * process to execute the line and waits for that process to complete.
 */

/* Main entry point.
 * argc    A count of command-line arguments
 * argv    The command-line argument values
 *
 * Micro-make (umake) reads from the uMakefile in the current working
 * directory.  The file is read one line at a time.  Lines with a leading tab
 * character ('\t') are interpreted as a command and passed to processline minus
 * the leading tab.
 */

int main(int argc, const char* argv[]) {
	FILE* makefile = fopen("./uMakefile", "r");
	if (makefile == NULL) {
		fprintf(stderr, "Failed to open the makefile");
		exit(1);
	}

	targetList head = NULL;
	targetList currentL = NULL;

	size_t bufsize = 0;
	char* line = NULL;
	ssize_t linelen = getline(&line, &bufsize, makefile);
	while (-1 != linelen) {
		if (line[linelen - 1] == '\n') {
			linelen -= 1;
			line[linelen] = '\0';
		}
		char *splitColon = strchr(line, ':');
		char *splitEquals = strchr(line, '=');

		if (splitEquals != NULL) {
			int i = 0;
			while (line[i] != '\0') {
				if (line[i] == '=') {
					line[i] = ' ';
				}
				i++;
			}
			int argcc;
			char ** passed = arg_parse(line, &argcc);
			int retFail = setenv(passed[0], passed[1], 1);
			if (retFail != 0) {
				fprintf(stderr, "setenv has failed!");
			}
		}

		if (splitColon != NULL) {
			int i = 0;
			while (line[i] != '\0') {
				if (line[i] == ':') {
					line[i] = ' ';
				}
				i++;
			}
			currentL = createTargets(&head, line);
		}

		if (line[0] == '\t') {
			addRules(line, currentL);
		}

		linelen = getline(&line, &bufsize, makefile);
	}

	for (int i = 1; i < argc; i++) {
		executeRule(processline, &head, strdup(argv[i]));
	}

	free(line);
	return EXIT_SUCCESS;
}

/*
 Process Line
 */
void processline(char* line) {
	int newSize = 1024;
	char newArray[1024];

	int expandRetVal = 0;
	expandRetVal = expand(line, newArray, newSize);

	if (expandRetVal != 0) {
		fprintf(stderr, "Failed to expand!\n");
	}

	int argcount;
	char** newLine = arg_parse(newArray, &argcount);

	if (argcount > 0) {
		const pid_t cpid = fork();
		switch (cpid) {
		case -1: {
			perror("fork");
			break;
		}
		case 0: {
			char** finalLine = IO_redirection(newLine);
			execvp(finalLine[0], finalLine);
			perror("execvp");
			free(finalLine);
			exit(EXIT_FAILURE);
			break;
		}
		default: {
			int status;
			const pid_t pid = wait(&status);
			if (-1 == pid) {
				perror("wait");
			} else if (pid != cpid) {
				fprintf(stderr,
						"wait: expected process %d, but waited for process %d",
						cpid, pid);
			}
			break;
		}
		}
	}
	free(newLine);
}

//this function expands environment variables
int expand(char *orig, char *new, int newSize) {
	char tempEnv[1024];
	int oIndex = 0;
	int nIndex = 0;
	int envIndex = 0;

	memset(new, '\0', newSize);
	memset(tempEnv, '\0', sizeof(tempEnv));
	for (int i = 0; orig[i]; i++) {

		if (orig[oIndex] == '#') {
			while (orig[oIndex] != '\0') {
				oIndex++;
			}
		}
		if (orig[oIndex - 1] != '$' && orig[oIndex] == '{') {
			fprintf(stderr, "Error needs $ before { to be an environment!\n");
			return -1;
		}
		if (orig[oIndex] == '$' && orig[oIndex + 1] != '{') {
			fprintf(stderr, "Error needs $ before { to be an environment!\n");
			return -1;
		}

		if (orig[oIndex] == '$' && orig[oIndex + 1] == '{') {
			oIndex = oIndex + 2;
			while (orig[oIndex] != '\0' && orig[oIndex] != '}') {
				tempEnv[envIndex] = orig[oIndex];
				oIndex++;
				envIndex++;
			}
			if (orig[oIndex] == '}') {
				oIndex++;
				char *envValue = getenv(tempEnv);
				if (envValue != NULL) {
					int valLen = strlen(envValue);
					strcpy(new + nIndex, envValue);
					nIndex += valLen;
				}
			} else if (orig[oIndex] != '}') {
				fprintf(stderr, "Could not find closing bracket");
				return -1;
			}
		}

		int mySize = nIndex + 1;
		if (mySize < newSize) {
			new[nIndex] = orig[oIndex];
			oIndex++;
			nIndex++;
		}
	}

	return 0;
}

//this function redirects program IO to files
//works with append (>>), truncate (>), and redirect input (<)
char** IO_redirection(char** line) {
	for (int index = 0; line[index] != NULL; index++) {
		if (line[index][0] == '<') {
			line[index] = NULL;

			int redirectFd = open(line[index + 1], O_RDONLY, 0666);
			if (redirectFd == -1) {
				fprintf(stderr, "Failed to open.");
			}
			if (dup2(redirectFd, 0) == -1) {
				fprintf(stderr, "Failed to duplicate.");
			}
			if (close(redirectFd) == -1) {
				fprintf(stderr, "Failed to close.");
			}
		} else if (line[index][0] == '>') {
			if (line[index][1] == '>') {
				line[index] = NULL;
				int appendFd = open(line[index + 1],
				O_RDWR | O_APPEND | O_CREAT, 0666);
				if (appendFd == -1) {
					fprintf(stderr, "Failed to open.");
				}
				if (dup2(appendFd, 1) == -1) {
					fprintf(stderr, "Failed to duplicate.");
				}
				if (close(appendFd) == -1) {
					fprintf(stderr, "Failed to close.");
				}
			} else {
				line[index] = NULL;
				int truncFd = open(line[index + 1], O_RDWR | O_TRUNC | O_CREAT, 0666);
				if (truncFd == -1) {
					fprintf(stderr, "Failed to open.");
				}
				if (dup2(truncFd, 1) == -1) {
					fprintf(stderr, "Failed to duplicate.");
				}
				if (close(truncFd) == -1) {
					fprintf(stderr, "Failed to close.");
				}

			}
		}

	}
	return line;
}
