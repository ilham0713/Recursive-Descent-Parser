#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	FILE *ifp;
	char *input = calloc(ARRAY_SIZE, sizeof(char));
	char buffer;
	int i;
	
	lexeme *list;
	instruction *code;
	
	// read in the input file
	if (argc < 2)
	{
		printf("Error : please include the file name\n");
		return 0;
	}
	
	ifp = fopen(argv[1], "r");
	i = 0;
	while (fscanf(ifp, "%c", &buffer) != EOF)
		input[i++] = buffer;
	input[i] = '\0';
	fclose(ifp);
	
	// lexical analysis
	list = lex_analyze(input);
	// if there were any errors, stop
	if (list == NULL)
	{
		free(input);
		return 0;
	}
	
	// parsing
	code = parse(list);
	// if there were any errors, stop
	if (code == NULL)
	{
		free(input);
		free(list);
		return 0;
	}
	
	// execute code
	execute(code);
	
	free(input);
	free(list);
	free(code);
	return 0;
}
