#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_IDENTIFIER_LEN 11
#define MAX_DIGIT_LEN 5
#define MAX_TABLE_LEN 512

int symbolCount = 1;
int myIndex = 0;
int jpcIndex = 0;
int loopIndex = 0;
int symIndex = 0;
int assemblyCount = 1;

typedef struct
{
  int OP;
  int L;
  int M;
} InstructionRegister;

InstructionRegister assemblyTable[MAX_TABLE_LEN];

typedef struct {
  char identifier[MAX_IDENTIFIER_LEN];
  int tokenValue;
  int integer;
} token;

typedef struct
{
  int kind; // const = 1, var = 2
  char name[10];
  int val;   // number ASCII
  int level; // hardcode 0
  int addr;  // M address
  int mark; // hardcode 1
} symbol;

int checkSymbol(char *string, symbol **symbolTable)
{
  for (int i = 0; i < symbolCount; i++)
  {
    if (strcmp(string, symbolTable[i]->name) == 0)
    {
      return i;
    }
  }
  return -1;
}

int variableDeclaration(symbol **symbolTable, token **tokens);

void constantDeclaration(symbol **symbolTable, token **tokens);

void expression(token **tokens, symbol **symbolTable);

void emit(int OP, int M);

int getToken(char *buffer);

symbol *makeSymbol(int kind, char *name, int value, int address)
{
  symbol *workingSymbol = malloc(sizeof(symbol));

  workingSymbol->kind = kind;
  strcpy(workingSymbol->name, name);
  workingSymbol->val = value;
  workingSymbol->addr = address;
  workingSymbol->level = 0;
  workingSymbol->mark = 1;

  return workingSymbol;
}

void printTable(symbol **symbolTable)
{
  printf("\n");
	printf("Symbol Table:\n\n");
	printf("Kind | Name        | Value | Level | Address | Mark\n");
	printf("---------------------------------------------------\n");

	for (int i = 0; i < symbolCount; i++)
		printf("%4d | %11s | %5d | %5d | %5d | %5d\n", symbolTable[i]->kind, symbolTable[i]->name, symbolTable[i]->val, symbolTable[i]->level, symbolTable[i]->addr, 1);
	printf("\n");
}

void factor(token **tokens, symbol **symbolTable)
{
  //printf("myindex top of factor:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);
  if (tokens[myIndex]->tokenValue == 2) // identsym
  {
    symIndex = checkSymbol(tokens[myIndex]->identifier, symbolTable);
    if (symIndex == -1)
    {
      printf("ERROR: Undeclared identifier\n");
      exit(0);
    }
    //printf("kind = %d\n", symbolTable[symIndex]->kind == 1);

    if (symbolTable[symIndex]->kind == 1) //constant
    {
      //printf("in lit\n");
      emit(1, tokens[myIndex]->integer); // lit
    }
    else
    {
      emit(3, symbolTable[symIndex]->addr); // lod
    }
    myIndex++;
  }

  else if (tokens[myIndex]->tokenValue == 3) // number
  {
    emit(1, tokens[myIndex]->integer);
    myIndex++;
  }
  else if (tokens[myIndex]->tokenValue == 15) // left paren
  {
    myIndex++;
    expression(tokens, symbolTable);

    if (tokens[myIndex]->tokenValue != 16) // right paren
    {
      printf("ERROR: Left parenthesis must be followed by right parenthesis\n");
      exit(0);
    }
    myIndex++;
  }
  else
  {
    //printf("tokenValue = %d\n", tokens[myIndex]->tokenValue);
    printf("ERROR: Arithmetic equations must contain operands, parenthesis, numbers, or symbols\n");
    exit(0);
  }
  //printf("myindex bottom of factor:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);
}

void term (token **tokens, symbol **symbolTable)
{
  //printf ("\n token in term is %s\n\n" , tokens[myIndex]->identifier);

  //printf("myindex top of term:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);
  factor(tokens, symbolTable);
  // mult, slash, mod
  while (tokens[myIndex]->tokenValue == 6 || tokens[myIndex]->tokenValue == 7)
  {
    if (tokens[myIndex]->tokenValue == 6) // mult
    {
      myIndex++;
      factor(tokens, symbolTable);
      emit(2, 3);
    }
    else if (tokens[myIndex]->tokenValue == 7) // slash
    {
      myIndex++;
      factor(tokens, symbolTable);
      emit(2, 4);
    }
    else
    {
      printf("ERROR: Arithmetic operations must contain operands, parenthesis, numbers, or symbols\n");
      exit (0);
    }
  }
  //printf("myindex bottom of term:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);
}

void expression (token **tokens, symbol **symbolTable)
{
  if (tokens[myIndex]->tokenValue == 5) // minus
  {
    myIndex++;
    term(tokens, symbolTable);
    while (tokens[myIndex]->tokenValue == 4 || tokens[myIndex]->tokenValue == 5) // plus or minus
    {
      if (tokens[myIndex]->tokenValue == 4) // plus
      {
        myIndex++;
        term(tokens, symbolTable);
        emit(1, 0);
      }
      else
      {
        myIndex++;
        term(tokens, symbolTable);
        emit(2, 0);
      }
    }
  }

  else
  {
    if (tokens[myIndex]->tokenValue == 4) // plus
    {
      myIndex++;
    }

    term(tokens, symbolTable);
    while (tokens[myIndex]->tokenValue == 4 || tokens[myIndex]->tokenValue == 5) // plus or minus
    {
      if (tokens[myIndex]->tokenValue == 4) // plus
      {
        myIndex++;
        term(tokens, symbolTable);
        emit(2, 1); // ADD
      }
      else
      {
        myIndex++;
        term(tokens, symbolTable);
        emit(2, 2); // SUB
      }
    }
  }
  //printf("myindex bottom of expression:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);
}

void condition(token **tokens, symbol **symbolTable)
{
  //printf("myindex top of condition:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);
  if (tokens[myIndex]->tokenValue == 8) // odd
  {
    myIndex++;
    expression(tokens, symbolTable);
    emit(2, 11); // ODD
  }
  else
  {
    expression(tokens, symbolTable);
    if (tokens[myIndex]->tokenValue == 9) // equal
    {
      myIndex++;
      expression(tokens, symbolTable);
      emit(2, 5);
    }
    else if (tokens[myIndex]->tokenValue == 10) // not equal
    {
      myIndex++;
      expression(tokens, symbolTable);
      emit(2, 6);
    }
    else if (tokens[myIndex]->tokenValue == 11) // less
    {
      myIndex++;
      expression(tokens, symbolTable);
      emit(2, 7);
    }
    else if (tokens[myIndex]->tokenValue == 12) // less equal
    {
      myIndex++;
      expression(tokens, symbolTable);
      emit(2, 8);
    }
    else if(tokens[myIndex]->tokenValue == 13) // greater
    {
      myIndex++;
      expression(tokens, symbolTable);
      emit(2, 9);
    }
    else if (tokens[myIndex]->tokenValue == 14) // greater equal
    {
      myIndex++;
      expression(tokens, symbolTable);
      emit(2, 10);
    }
    else
    {
      printf("ERROR: Condition must contain comparison operators\n");
      exit (0);
    }
    //printf("myindex bottom of condition:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);
  }
}

void statement(token **tokens, symbol **symbolTable)
{
  //printf("myindex top of statement:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);
  if (tokens[myIndex]->tokenValue == 2) // indentsym
  {
    //printf("statement identsym = %s\n" , tokens[myIndex]->identifier);
    symIndex = checkSymbol(tokens[myIndex]->identifier, symbolTable);
    if (symIndex == -1)
    {
      printf("ERROR: Undeclared identifier\n");
      exit(0);
    }

    if (symbolTable[symIndex]->kind != 2)
    {
      printf("ERROR: Only variable values may be altered\n");
      exit(0);
    }
    myIndex++;
    if (tokens[myIndex]->tokenValue != 20) // becomsym
    {
      printf("ERROR: Assignment statements must use ':='\n");
      exit (0);
    }
    myIndex++;


    expression(tokens, symbolTable);
    emit(4, symbolTable[symIndex]->addr);
    return;
  }

  if (tokens[myIndex]->tokenValue == 21) // begin
  {
    do {
      myIndex++;

      if (tokens[myIndex]->tokenValue == 19)
      {
        printf("ERROR: Begin must be followed by end\n");
        exit (0);
      }

      statement(tokens, symbolTable);
    } while(tokens[myIndex]->tokenValue != 18);


    myIndex++;

    if (tokens[myIndex]->tokenValue != 22) // end
    {
      printf("ERROR: Begin must be followed by end\n");
      exit (0);
    }
    myIndex++;
    return;
  }

  if (tokens[myIndex]->tokenValue == 23) // if
  {
    myIndex++;
    condition(tokens, symbolTable);
    jpcIndex = assemblyCount;
    emit(8, 0);
    if (tokens[myIndex]->tokenValue != 24) // then
    {
      printf("ERROR: If must be followed by then\n");
      exit (0);
    }
    myIndex++;
    statement(tokens, symbolTable);
    assemblyTable[jpcIndex].M = assemblyCount;
    return;
  }

  if (tokens[myIndex]->tokenValue == 25) // while
  {
    myIndex++;
    loopIndex = assemblyCount;
    condition(tokens, symbolTable);
    if (tokens[myIndex]->tokenValue != 26) // do
    {
      printf("ERROR: While must be followed by do\n");
      exit (0);
    }
    myIndex++;
    jpcIndex = assemblyCount;
    emit(8, 0);
    statement(tokens, symbolTable);
    emit(7, loopIndex);
    assemblyTable[jpcIndex].M = assemblyCount;
    return;
  }

  if (tokens[myIndex]->tokenValue == 32) // read
  {
    myIndex++;
    if (tokens[myIndex]->tokenValue != 2) // identsym
    {
      printf("ERROR: Const, var, and read must be followed by an identifier\n");
      exit (0);
    }

    symIndex = checkSymbol(tokens[myIndex]->identifier, symbolTable);
    if (symIndex == -1)
    {
      printf("ERROR: Undeclared identifier\n");
      exit(0);
    }

    if (symbolTable[symIndex]->kind != 2)
    {
      printf("ERROR: Only variable values may be altered\n");
      exit(0);
    }
    myIndex++;
    emit(9, 2);
    emit(4, symbolTable[symIndex]->addr);
    return;
  }

  //printf("myindex in statement:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);

  if (tokens[myIndex]->tokenValue == 31) // write
  {
    myIndex++;
    expression(tokens, symbolTable);
    emit(9, 1);
    return;
  }

  //printf("myindex bottom of statement:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);
}

int variableDeclaration (symbol **symbolTable, token **tokens)
{
  int numVars = 0;

  //printf("myindex top of varDec:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);

  if (tokens[myIndex]->tokenValue == 29) //var
  {
    do {
      numVars++;
      myIndex++;
      if (tokens[myIndex]->tokenValue != 2) // identsym
      {
        printf("ERROR: Const, var, and read must be followed by an identifier\n");
        exit (0);
      }
      if (checkSymbol(tokens[myIndex]->identifier, symbolTable) != -1)
      {
          printf("ERROR: Symbol name has already been declared\n");
          exit (0);
      }
      //printf("making symbol %s\n", tokens[myIndex]->identifier);
      symbolTable[symbolCount++] = makeSymbol(2, tokens[myIndex]->identifier, 0, (numVars + 2));
      myIndex++;
    } while(tokens[myIndex]->tokenValue == 17);

    if (tokens[myIndex]->tokenValue != 18)
    {
      printf("ERROR: Constant and variable declarations must be followed by a semicolon\n");
      exit (0);
    }
    myIndex++;
  }
  //printf("myindex bottom of varDec:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);
  return numVars;
}

void constantDeclaration (symbol **symbolTable, token **tokens)
{
  //printf("myindex top of constDec:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);

  if (tokens[myIndex]->tokenValue == 28)
  {
    if (tokens[myIndex + 1]->tokenValue != 2)
    {
      //printf("token value is %d\n ", tokens[myIndex + 1]->tokenValue);
      printf("ERROR: Const, var, and read must be followed by an identifier\n");
      exit(0);
    }

    if (checkSymbol(tokens[myIndex + 1]->identifier, symbolTable) != -1)
    {
      printf("ERROR: Symbol name has already been declared\n");
      exit(0);
    }

    if (tokens[myIndex + 2]->tokenValue != 9)
    {
      printf("ERROR: Constants must be assigned with '='\n");
      exit(0);
    }
    if (tokens[myIndex + 3]->tokenValue != 3)
    {
      printf("ERROR: Constants must be assigned an integer value\n");
      exit(0);
    }

    symbolTable[symbolCount++] = makeSymbol(1, tokens[1]->identifier, tokens[3]->integer, -1);

    myIndex++;
    myIndex++;
    myIndex++;
    myIndex++;

    while (tokens[myIndex]->tokenValue != 18) // semicolon
    {
      if (tokens[myIndex + 1]->tokenValue != 2) // identifier
      {
        printf("ERROR: Const, var, and read must be followed by an identifier\n");
        exit(0);
      }
      if (tokens[myIndex + 2]->tokenValue != 9)
      {
        printf("ERROR: Constants must be assigned with '='\n");
        exit(0);
      }
      if (tokens[myIndex + 3]->tokenValue != 3)
      {
        printf("ERROR: Constants must be assigned an integer value\n");
        exit(0);
      }

      symbolTable[symbolCount++] = makeSymbol(2, tokens[myIndex + 1]->identifier, tokens[myIndex + 3]->integer, -1);
      myIndex = myIndex + 4;
    }
    if (tokens[myIndex]->tokenValue != 18)
    {
      printf("ERROR: Constant and variable declarations must be followed by a semicolon\n");
    }
    myIndex++;
    //printf("myindex bottom of constDec:   %d   token value %d\n", myIndex, tokens[myIndex]->tokenValue);
  }
}

void block(symbol **symbolTable ,token **tokens)
{
  int numVars;
  //printf("in constant\n");
  constantDeclaration(symbolTable, tokens);
  //printf("in variable\n");
  numVars = variableDeclaration(symbolTable, tokens);
  //printf("in emit\n");
  emit(6, (3 + numVars));
  //printf("in statement\n");
  statement(tokens, symbolTable);
}

void program (symbol **symbolTable, token **tokens)
{

  block(symbolTable, tokens);

  //printf("\n\ntoken is %d\n\n", tokens[myIndex]->tokenValue);
  if (tokens[myIndex]->tokenValue != 19) // period
  {
    //printf("token value: %d", tokens[myIndex]->tokenValue);
    //printf("my index is %d token count is\n", myIndex);
    exit (0);
  }
  emit(9, 3);
}

void checkDuplicates (token **tokens, int tokenCount, char *identifier)
{
  int count = 0;

  for (int i = 0; i < tokenCount; i++)
  {
    if (tokens[i]->tokenValue != 2)
    {
      continue;
    }

    if (strcmp(identifier, tokens[i]->identifier) == 0)
    {
      count++;
    }

    if (count > 1)
    {
      printf("ERROR: Symbol name has already been declared\n");
      exit(0);
    }
  }
}

void emit(int OP, int M)
{
  //printf("in emit, OP: %d M: %d\n", OP, M);
  assemblyTable[assemblyCount].OP = OP;
  assemblyTable[assemblyCount].L = 0;
  assemblyTable[assemblyCount].M = M;
  assemblyCount++;
  //printf("assemblyCount now = %d\n", assemblyCount);
}

char *printAssembly(int i)
{
  int halt = 0;

  switch (assemblyTable[i].OP)
  {
  case 1: // push M onto stack
    return "LIT";

    break;

  case 2: // returns from a subroutine and restores the caller’s AR

    switch (assemblyTable[i].M)
    {
    case 0: // return
      return "RTN";

      break;

    case 1: // add
      return "ADD";

      break;

    case 2: // subtract
      return "SUB";

      break;

    case 3: // multiply
      return "MUL";

      break;

    case 4: // divide
      return "DIV";
      break;

    case 5: // equal
      return "EQL";

      break;

    case 6: // not equal
      return "NEQ";

      break;

    case 7: // less than
      return "LSS";

      break;

    case 8: // less than or equal to
      return "LEQ";

      break;

    case 9: // greater than
      return "GTR";

      break;

    case 10: // greater than or equal to
      return "GEQ";

      break;
    }
    break;

  // load value to top of stack from the stack location at offset o from n lexicographical levels down
  case 3:
    return "LOD";

    break;

  // Store value at top of stack in the stack location at offset o from n lexicographical levels down
  case 4:
    return "STO";

    break;

  // call the procedure at code myIndex p, generating a new activation record and setting PC to p
  case 5:
    return "CAL";

    break;

  // allocate m locals on the stack
  case 6:
    return "INC";

    break;

  // jump to address in stack and pop
  case 7:
    return "JMP";

    break;

  // jump conditionally: if the value in stack[sp] is 0, then jump to a and pop the stack
  case 8:
    return "JPC";

    break;

  // print value, read value, or end program
  case 9:

    switch (assemblyTable[i].M)
    {
    // Output of the value in stack[SP] to standard output as a character and pop
    case 1:
      return "SYS";

      break;

    // Read an integer, as character value, from standard input (stdin) and store it on the top of the stack
    case 2:
      //int userInt;
      printf("Please Enter an Integer: \n");
      //scanf("%d", &userInt);
      printf("\tSIN\t");

      break;

    // halt program
    case 3:
      return "SYS";
      halt = 1;
      break;
    }
    }
    assemblyCount++;

    return "Error";
}

// Make character token
token *makeToken(char *buffer)
{
  token *workingToken = malloc(sizeof(token));

  strcpy(workingToken->identifier, buffer);

  workingToken->tokenValue = getToken(buffer);

  return workingToken;
}

// Make special tokens
token *getSpecial(char current, FILE *source_file)
{
  int skipsym = 1, identsym = 2, numbersym = 3, plussym = 4, minussym = 5,
  multsym = 6,  slashsym = 7, oddsym = 8,  eqlsym = 9, neqsym = 10, lessym = 11,
  leqsym = 12, gtrsym = 13, geqsym = 14, lparentsym = 15, rparentsym = 16,
  commasym = 17, semicolonsym = 18, periodsym = 19, becomessym = 20,
  beginsym = 21, endsym = 22, ifsym = 23, thensym = 24, whilesym = 25, dosym = 26,
  callsym9 = 27, constsym = 28, varsym = 29, procsym9 = 30, writesym = 31,
  readsym = 32, elsesym9 = 33;

  token *workingToken = malloc(sizeof(token));

  workingToken->identifier[0] = current;

  if (current == '+')
  {
    workingToken->tokenValue = plussym; // 4
  }
  else if (current == '-')
  {
    workingToken->tokenValue = minussym; // 5
  }
  else if (current == '*')
  {
    workingToken->tokenValue = multsym; // 6
  }
  else if (current == '/')
  {
    workingToken->tokenValue = slashsym; // 7
  }
  else if (current == '(')
  {
    workingToken->tokenValue = lparentsym; // 15
  }
  else if (current == ')')
  {
    workingToken->tokenValue = rparentsym; // 16
  }
  else if (current == ',')
  {
    workingToken->tokenValue = commasym; // 17
  }
  else if (current == '=')
  {
    workingToken->tokenValue = eqlsym; // 9
  }
  else if (current == '.')
  {
    workingToken->tokenValue = periodsym; // 19
  }
  else if (current == ';')
  {
    workingToken->tokenValue = semicolonsym; // 18
  }
  else if (current == ':')
  {
    char prev = current;

    current = getc(source_file);

    if (current == '=')
    {
      workingToken->tokenValue = becomessym; // 20
    }
    else
    {
      printf("ERROR: Invalid symbol\n");
      exit (0);
    }
  }
  else if (current == '<')
  {
    printf("1current = %c\n", current);
    char prev = current;

    current = getc(source_file);
    printf("2current = %c\n", current);

    if (current == '=')
    {
      workingToken->tokenValue = leqsym; // 12
    }
    else if (isspace(current) || iscntrl(current))
    {
      workingToken->tokenValue = lessym; // 11
    }
    else if (current == '>')
    {
      workingToken->tokenValue = neqsym; // 10
    }
    else
    {
      printf("ERROR: Invalid symbol\n");
      exit(0);
    }
  }
  else if (current == '>')
  {
    char prev = current;

    current = getc(source_file);

    if (current == '=')
    {
      workingToken->tokenValue = geqsym; // 14
    }
    else if (isspace(current) || iscntrl(current))
    {
      workingToken->tokenValue = gtrsym; // 13
    }
    else
    {
      printf("ERROR: Invalid symbol\n");
      exit(0);
    }
  }
  else if (current == '!')
  {
    char prev = current;

    current = getc(source_file);

    if (current == '=')
    {
      workingToken->tokenValue = neqsym; // 10
    }
    else
    {
      printf("ERROR: Invalid symbol\n");
      exit(0);
    }
  }

  return workingToken;
}

// Makes a token attached to number
token *getNumToken(int *buffer, int length)
{
  token *workingToken = malloc(sizeof(token));

  int answer = 0;
  int power = 1;

  for (int i = length - 2; i >= 0; i--)
  {
    answer = answer + (buffer[i] * power);
    power = power * 10;
  }

  workingToken->tokenValue = 3;
  workingToken->integer = answer;

  return workingToken;
}

int getToken(char *buffer)
{
  int skipsym = 1, identsym = 2, numbersym = 3, plussym = 4, minussym = 5,
  multsym = 6,  slashsym = 7, oddsym = 8,  eqlsym = 9, neqsym = 10, lessym = 11,
  leqsym = 12, gtrsym = 13, geqsym = 14, lparentsym = 15, rparentsym = 16,
  commasym = 17, semicolonsym = 18, periodsym = 19, becomessym = 20,
  beginsym = 21, endsym = 22, ifsym = 23, thensym = 24, whilesym = 25, dosym = 26,
  callsym9 = 27, constsym = 28, varsym = 29, procsym9 = 30, writesym = 31,
  readsym = 32, elsesym9 = 33;

  if (strcmp(buffer, "else") == 0) // 33
      return identsym;
  if (strcmp(buffer, "odd") == 0) // 8
    return oddsym;
  if (strcmp(buffer, "begin") == 0) // 21
      return beginsym;
  if (strcmp(buffer, "end") == 0) // 22
      return endsym;
  if (strcmp(buffer, "if") == 0) // 23
      return ifsym;
  if (strcmp(buffer, "then") == 0) // 24
      return thensym;
  if (strcmp(buffer, "while") == 0) // 25
      return whilesym;
  if (strcmp(buffer, "do") == 0) // 26
      return dosym;
  if (strcmp(buffer, "call") == 0) // 27
      return identsym;
  if (strcmp(buffer, "const") == 0) // 28
      return constsym;
  if (strcmp(buffer, "var") == 0) // 29
      return varsym;
  if (strcmp(buffer, "procedure") == 0) // 30 TO DO!!!!!!!!!!!!!!!!!!!!
      return identsym;
  if (strcmp(buffer, "read") == 0) // 32
      return readsym;
  if (strcmp(buffer, "write") == 0) // 31
      return writesym;
  else
      return identsym; // 2

}

int main (int argc, char **argv)
{
  //printf ("top of main \n");
  char current;

  int nums[515]; // we have nums[515] as well as buffer[515] ???
  int flag = 0;
  int tokenCount = 0;

  token **tokens = malloc(515 * sizeof(token));
  symbol *SymbolTable = malloc (515 * sizeof(symbol));

  char *file_name = argv[1];
  FILE *source_file = fopen(file_name, "r");

  // printf("Source Program: \n");
  //
  // // Print source program
  // while (!feof(source_file))
  // {
  //   current = getc(source_file);
  //   if (current == EOF)
  //     break;
  //   printf("%c", current);
  // }

  //source_file = fopen(file_name, "r");

  while (!feof(source_file))
  {
    //printf("in while\n");
    char buffer[515];
    if (flag == 1)
      break;

    current = getc(source_file);

    if (current == EOF)
      break;

    // If we have a whitespace character, skip it
    if (isspace(current) || iscntrl(current))
    {
        continue;
    }

    // If comment skip it
    if (current == '/' && !feof(source_file))
    {
      // Check to see if next char is *
      current = getc(source_file);


      if (current == '*')
      {
        current = getc(source_file);

        if (current == EOF)
        {
          printf("ERROR: Input ends during comment\n");
          exit(0);
          flag = 1;
          continue;
        }

        // loop through comment
        while (current != '*')
        {
          current = getc(source_file);
          if (current == EOF)
          {
            printf("ERROR: Input ends during comment\n");
            exit(0);
            flag = 1;
            break;
          }

          if (flag == 1)
            break;

          // Check if at end of comment
          if (current == '*')
          {
            current = getc(source_file);

            // at end of comment
            if (current == '/')
            {
              break;
            }
          }
        }

        continue;
      }

      // If its not, place back in file and reset state
      else
      {
          ungetc(current, source_file);
          current = '/';
      }
    }

    // If we have a char find token
    if (isalpha(current))
    {
      int i = 0;
      int lengthCheck = 0;

      // Scan in all valid characters
      while (isalpha(current) || isdigit(current))
      {
        buffer[i] = current;
        i++;
        if (i > 10)
        {
          printf("ERROR: Identifier too long\n");
          exit(0);
          lengthCheck = 1;

          break;
        }
        current = getc(source_file);
      }

      // If Identifier too long skip making token
      if (lengthCheck == 1)
      {
        continue;
      }

      // Put invalid char back into file
      ungetc(current, source_file);

      // Insert null so we can string compare
      buffer[i] = '\0';

      // create the token we need
      tokens[tokenCount++] = makeToken(buffer);
    }

    // Find number token
    else if(isdigit(current))
    {
      int i = 0;
      int length = 1;
      int numBuffer[5];
      int numFlag = 0;

      // Grab all valid numbers
      while (isdigit(current))
      {
        numBuffer[i] = current - '0';
        i++;
        length++;
        current = getc(source_file);

        // if number is too long
        if (i > 4)
        {
          numFlag = 1;
          printf("ERROR: Number too long\n");
          exit(0);

        }

        if (isalpha(current))
        {
          numFlag = 1;
          printf("ERROR: Identifier cannot start with digit\n");
          exit(0);
        }
      }

      // If number is too long skip it and continue
      if (numFlag == 1)
      {
        continue;
      }

      ungetc(current, source_file);

      // Make number token
      tokens[tokenCount++] = getNumToken(numBuffer, length);
    }
    // Is a special character
    else
    {
      tokens[tokenCount++] = getSpecial(current, source_file);

      if (tokens[tokenCount - 1] == NULL)
      {
        tokenCount--;
      }
    }
  }
  //printf("end of program 2\n");

  if (tokens[tokenCount - 1]->tokenValue != 19)
  {
    printf("ERROR: Program must end with a period\n");
    exit(0);
  }

  int symbolCount = 0;
  int address = 3;

  symbol **symbolTable = malloc(500 * sizeof(symbol));
  symbolTable[0] = malloc(sizeof(symbol));

  //printf ("making table\n");
  // always going to be the same
  symbolTable[0]->kind = 3;
  //printf ("doing name\n");
  strcpy(symbolTable[0]->name, "main");
  //printf("done with name\n");
  symbolTable[0]->val = 0;
  symbolTable[0]->level = 0;
  symbolTable[0]->addr = 3;
  symbolTable[0]->mark = 1;

  //printf("symbolTable made");

  // for (int i = 0; i < tokenCount; i++)
  // {
  //   if (tokens[i]->tokenValue == 2)
  //   {
  //     checkDuplicates(tokens, tokenCount, tokens[i]->identifier);
  //   }
  // }

  program(symbolTable, tokens);

  assemblyTable[0].OP = 7;
  assemblyTable[0].L = 0;
  assemblyTable[0].M = 3;


  printf("Line\t\tOP\t\t\tL\t\tM\n");

  for (int i = 0; i < assemblyCount; i++)
  {
    //printf("in for loop\n");
    printf("%d\t\t%s\t\t\t0\t\t%d\n", i, printAssembly(i), assemblyTable[i].M);
  }
  
  printTable(symbolTable);
}
