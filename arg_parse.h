#ifndef Arg_parse
#define Arg_parse

//helper function for arg_parse that counts the number of arguments in array
int argument_counter(char *line);

//this function returns an array of pointers that point to the start of each of the arguments
char** arg_parse(char *line, int *argcp);

#endif