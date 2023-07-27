#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DIGIT_LEN_MAX 5
#define IDENT_LENMAX 11
#define TABLE_LEN_MAX 512

int curIndex = 0;
int jpcIndex = 0;
int loopIndex = 0;
int symIndex = 0;
int symCnt = 1;
int assemblyCnt = 1;

//ex: jmp, level, m-depends on thing
typedef struct
{
  int OP;
  int L;
  int M;
} InstructionRegister;
InstructionRegister assemblyTable[TABLE_LEN_MAX];

typedef struct {
  char identifier[IDENT_LENMAX]; //(identsym i think)
  int tokenValue; //(number associated)
  int integer; //IDK
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

//function which takes in symbol table and current symbol, then iterates through entire symbol table to see if it has been declared
int symbolTableCheck(char *string, symbol **symbolTable)
{
  //linear search through symbol table looking at name
  for (int i = 0; i < symCnt; i++)
  {
    //if yes it returns index
    if (strcmp(string, symbolTable[i]->name) == 0)
    {
      return i;
    }
  }
  //else returns -1
  return -1;
}

int variableDeclaration(symbol **symbolTable, token **tokenz);
void constantDeclaration(symbol **symbolTable, token **tokenz);
void expression(token **tokenz, symbol **symbolTable);
void emit(int OP, int M);
int getToken(char *buffer);

//mallocs and creates table as needed
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
	printf("----------------------------------------------------\n");

	for (int i = 0; i < symCnt; i++)
		printf("%4d | %11s | %5d | %5d | %7d | %5d\n", symbolTable[i]->kind, symbolTable[i]->name, symbolTable[i]->val, symbolTable[i]->level, symbolTable[i]->addr, 1);
	printf("\n");
}

void factor(token **tokenz, symbol **symbolTable)
{
  if (tokenz[curIndex]->tokenValue == 2) 
  {
    symIndex = symbolTableCheck(tokenz[curIndex]->identifier, symbolTable);
    if (symIndex == -1)
    {
      printf("ERROR: Undeclared identifier %s\n",tokenz[curIndex]->identifier);
      exit(0);
    }

    if (symbolTable[symIndex]->kind == 1)
    {
      emit(1, tokenz[curIndex]->integer); 
    }
    else
    {
      emit(3, symbolTable[symIndex]->addr); 
    }
    curIndex++;
  }

  else if (tokenz[curIndex]->tokenValue == 3) 
  {
    emit(1, tokenz[curIndex]->integer);
    curIndex++;
  }
  else if (tokenz[curIndex]->tokenValue == 15) //parenthesis(left side)
  {
    curIndex++;
    expression(tokenz, symbolTable);

    if (tokenz[curIndex]->tokenValue != 16) //parenthesis (right)
    {
      printf("ERROR: Left parenthesis must be followed by right parenthesis\n");
      exit(0);
    }
    curIndex++;
  }
  else
  {
    //printf("tokenVal: %d\n", tokenz[curIndex]->tokenValue);
    printf("ERROR: Arithmetic equations must contain operands, parenthesis, numbers, or symbols\n");
    exit(0);
  }
  //printf("End of factor curInd: %d   token val: %d\n", curIndex, tokenz[curIndex]->tokenValue);
}

void term (token **tokenz, symbol **symbolTable)
{
  //printf ("\n token inside term is %s\n\n" , tokenz[curIndex]->identifier);

  //printf("Top of term Curindex: %d  token value: %d\n", curIndex, tokenz[curIndex]->tokenValue);
  factor(tokenz, symbolTable);
  // mult, slash, mod
  while (tokenz[curIndex]->tokenValue == 6 || tokenz[curIndex]->tokenValue == 7)
  {
    if (tokenz[curIndex]->tokenValue == 6) // mult
    {
      curIndex++;
      factor(tokenz, symbolTable);
      emit(2, 3);
    }
    else if (tokenz[curIndex]->tokenValue == 7) // slash
    {
      curIndex++;
      factor(tokenz, symbolTable);
      emit(2, 4);
    }
    else
    {
      printf("ERROR: Arithmetic operations must contain operands, parenthesis, numbers, or symbols\n");
      exit (0);
    }
  }
  //printf("End of term CurInd: %d  token val: %d\n", curIndex, tokenz[curIndex]->tokenValue);
}

void expression (token **tokenz, symbol **symbolTable)
{
  if (tokenz[curIndex]->tokenValue == 5) // minus
  {
    curIndex++;
    term(tokenz, symbolTable);
    while (tokenz[curIndex]->tokenValue == 4 || tokenz[curIndex]->tokenValue == 5) // plus or minus
    {
      if (tokenz[curIndex]->tokenValue == 4) // plus
      {
        curIndex++;
        term(tokenz, symbolTable);
        emit(1, 0);
      }
      else
      {
        curIndex++;
        term(tokenz, symbolTable);
        emit(2, 0);
      }
    }
  }

  else
  {
    if (tokenz[curIndex]->tokenValue == 4) // plus
    {
      curIndex++;
    }

    term(tokenz, symbolTable);
    while (tokenz[curIndex]->tokenValue == 4 || tokenz[curIndex]->tokenValue == 5) // plus or minus
    {
      if (tokenz[curIndex]->tokenValue == 4) // plus
      {
        curIndex++;
        term(tokenz, symbolTable);
        emit(2, 1); // ADD
      }
      else
      {
        curIndex++;
        term(tokenz, symbolTable);
        emit(2, 2); // SUB
      }
    }
  }
  //printf("expression curInd:   %d   token val:  %d\n", curIndex, tokenz[curIndex]->tokenValue);
}

void condition(token **tokenz, symbol **symbolTable)
{
  //printf("Top of condition curInd:   %d   token val: %d\n", curIndex, tokenz[curIndex]->tokenValue);
  if (tokenz[curIndex]->tokenValue == 8) // odd
  {
    curIndex++;
    expression(tokenz, symbolTable);
    emit(2, 11); // ODD
  }
  else
  {
    expression(tokenz, symbolTable);
    if (tokenz[curIndex]->tokenValue == 9) // equal
    {
      curIndex++;
      expression(tokenz, symbolTable);
      emit(2, 5);
    }
    else if (tokenz[curIndex]->tokenValue == 10) // not equal
    {
      curIndex++;
      expression(tokenz, symbolTable);
      emit(2, 6);
    }
    else if (tokenz[curIndex]->tokenValue == 11) // less
    {
      curIndex++;
      expression(tokenz, symbolTable);
      emit(2, 7);
    }
    else if (tokenz[curIndex]->tokenValue == 12) // less equal
    {
      curIndex++;
      expression(tokenz, symbolTable);
      emit(2, 8);
    }
    else if(tokenz[curIndex]->tokenValue == 13) // greater
    {
      curIndex++;
      expression(tokenz, symbolTable);
      emit(2, 9);
    }
    else if (tokenz[curIndex]->tokenValue == 14) // greater equal
    {
      curIndex++;
      expression(tokenz, symbolTable);
      emit(2, 10);
    }
    else
    {
      printf("ERROR: Condition must contain comparison operators\n");
      exit (0);
    }
    //printf("End of cond curIndex:   %d   token val:  %d\n", curIndex, tokenz[curIndex]->tokenValue);
  }
}

void statement(token **tokenz, symbol **symbolTable)
{
  //printf("Top of statement curIndex: %d token val: %d\n", curIndex, tokenz[curIndex]->tokenValue);
  if (tokenz[curIndex]->tokenValue == 2) // identsym
  {
    //printf("stmnt identsym = %s\n" , tokenz[curIndex]->identifier);
    symIndex = symbolTableCheck(tokenz[curIndex]->identifier, symbolTable);
    if (symIndex == -1)
    {
      printf("ERROR: Undeclared identifier %s\n", tokenz[curIndex]->identifier);
      exit(0);
    }

    if (symbolTable[symIndex]->kind != 2)
    {
      printf("ERROR: Only variable values may be altered\n");
      exit(0);
    }
    curIndex++;
    if (tokenz[curIndex]->tokenValue != 20) // becomsym
    {
      printf("ERROR: Assignment statements must use ':='\n");
      exit (0);
    }
    curIndex++;


    expression(tokenz, symbolTable);
    emit(4, symbolTable[symIndex]->addr);
    return;
  }

  if (tokenz[curIndex]->tokenValue == 21) // beginsym
  {
    do {
      curIndex++;

      if (tokenz[curIndex]->tokenValue == 19)
      {
        printf("ERROR: Begin must be followed by end\n");
        exit (0);
      }

      statement(tokenz, symbolTable);
    } while(tokenz[curIndex]->tokenValue != 18);


    curIndex++;

    if (tokenz[curIndex]->tokenValue != 22) // endsym
    {
      printf("ERROR: Begin must be followed by end\n");
      exit (0);
    }
    curIndex++;
    return;
  }

  if (tokenz[curIndex]->tokenValue == 23) // ifsym
  {
    curIndex++;
    condition(tokenz, symbolTable);
    jpcIndex = assemblyCnt;
    emit(8, 0);
    if (tokenz[curIndex]->tokenValue != 24) // thensym
    {
      printf("ERROR: If must be followed by then\n");
      exit (0);
    }
    curIndex++;
    statement(tokenz, symbolTable);
    assemblyTable[jpcIndex].M = assemblyCnt;
    return;
  }

  if (tokenz[curIndex]->tokenValue == 25) // whilesym
  {
    curIndex++;
    loopIndex = assemblyCnt;
    condition(tokenz, symbolTable);
    if (tokenz[curIndex]->tokenValue != 26) // dosym
    {
      printf("ERROR: While must be followed by do\n");
      exit (0);
    }
    curIndex++;
    jpcIndex = assemblyCnt;
    emit(8, 0);
    statement(tokenz, symbolTable);
    emit(7, loopIndex);
    assemblyTable[jpcIndex].M = assemblyCnt;
    return;
  }

  if (tokenz[curIndex]->tokenValue == 32) // readsym
  {
    curIndex++;
    if (tokenz[curIndex]->tokenValue != 2) // identsym
    {
      printf("ERROR: Const, var, and read must be followed by an identifier\n");
      exit (0);
    }

    symIndex = symbolTableCheck(tokenz[curIndex]->identifier, symbolTable);
    if (symIndex == -1)
    {
      printf("ERROR: Undeclared identifier %s\n", tokenz[curIndex]->identifier);
      exit(0);
    }

    if (symbolTable[symIndex]->kind != 2)
    {
      printf("ERROR: Only variable values may be altered\n");
      exit(0);
    }
    curIndex++;
    emit(9, 2);
    emit(4, symbolTable[symIndex]->addr);
    return;
  }

  //printf("curIndex in stmnt: %d token val: %d\n", curIndex, tokenz[curIndex]->tokenValue);

  if (tokenz[curIndex]->tokenValue == 31) // writesym
  {
    curIndex++;
    expression(tokenz, symbolTable);
    emit(9, 1);
    return;
  }

  //printf("Bottom of statement curInd : %d token val: %d\n", curIndex, tokenz[curIndex]->tokenValue);
}

int variableDeclaration (symbol **symbolTable, token **tokenz)
{
  int numVars = 0;

  //printf("Top of varDec curInd: %d   token val %d\n", curIndex, tokenz[curIndex]->tokenValue);

  if (tokenz[curIndex]->tokenValue == 29) //varsym
  {
    do {
      numVars++;
      curIndex++;
      if (tokenz[curIndex]->tokenValue != 2) // identsym
      {
        printf("ERROR: Const, var, and read must be followed by an identifier\n");
        exit (0);
      }
      if (symbolTableCheck(tokenz[curIndex]->identifier, symbolTable) != -1)
      {
          printf("ERROR: Symbol name has already been declared\n");
          exit (0);
      }
      //printf("making symbol %s\n", tokenz[curIndex]->identifier);
      symbolTable[symCnt++] = makeSymbol(2, tokenz[curIndex]->identifier, 0, (numVars + 2));
      curIndex++;
    } while(tokenz[curIndex]->tokenValue == 17);

    if (tokenz[curIndex]->tokenValue != 18)
    {
      printf("ERROR: Constant and variable declarations must be followed by a semicolon\n");
      exit (0);
    }
    curIndex++;
  }
  //printf("Bottom of varDec curInd : %d token val: %d\n", curIndex, tokenz[curIndex]->tokenValue);
  return numVars;
}

void constantDeclaration (symbol **symbolTable, token **tokenz)
{
  //printf("Top of constDec curIndex : %d   token val: %d\n", curIndex, tokenz[curIndex]->tokenValue);

  if (tokenz[curIndex]->tokenValue == 28)
  {
    if (tokenz[curIndex + 1]->tokenValue != 2)
    {
      //printf("token value is %d\n ", tokenz[curIndex + 1]->tokenValue);
      printf("ERROR: Const, var, and read must be followed by an identifier\n");
      exit(0);
    }

    if (symbolTableCheck(tokenz[curIndex + 1]->identifier, symbolTable) != -1)
    {
      printf("ERROR: Symbol name has already been declared\n");
      exit(0);
    }

    if (tokenz[curIndex + 2]->tokenValue != 9)
    {
      printf("ERROR: Constants must be assigned with '='\n");
      exit(0);
    }
    if (tokenz[curIndex + 3]->tokenValue != 3)
    {
      printf("ERROR: Constants must be assigned an integer value\n");
      exit(0);
    }

    symbolTable[symCnt++] = makeSymbol(1, tokenz[1]->identifier, tokenz[3]->integer, -1);

    curIndex++;
    curIndex++;
    curIndex++;
    curIndex++;

    while (tokenz[curIndex]->tokenValue != 18) // semicolonsym
    {
      if (tokenz[curIndex + 1]->tokenValue != 2) // identsym
      {
        printf("ERROR: Const, var, and read must be followed by an identifier\n");
        exit(0);
      }
      if (tokenz[curIndex + 2]->tokenValue != 9)
      {
        printf("ERROR: Constants must be assigned with '='\n");
        exit(0);
      }
      if (tokenz[curIndex + 3]->tokenValue != 3)
      {
        printf("ERROR: Constants must be assigned an integer value\n");
        exit(0);
      }

      symbolTable[symCnt++] = makeSymbol(2, tokenz[curIndex + 1]->identifier, tokenz[curIndex + 3]->integer, -1);
      curIndex = curIndex + 4;
    }
    if (tokenz[curIndex]->tokenValue != 18)
    {
      printf("ERROR: Constant and variable declarations must be followed by a semicolon\n");
    }
    curIndex++;
    //printf("End of constDec curIndex: %d token val: %d\n", curIndex, tokenz[curIndex]->tokenValue);
  }
}

void block(symbol **symbolTable ,token **tokenz)
{
  int numVars;
  //printf("in constant\n");
  constantDeclaration(symbolTable, tokenz);
  //printf("in variable\n");
  numVars = variableDeclaration(symbolTable, tokenz);
  //printf("in emit\n");
  emit(6, (3 + numVars));
  //printf("in statement\n");
  statement(tokenz, symbolTable);
}

void program (symbol **symbolTable, token **tokenz)
{

  block(symbolTable, tokenz);

  //printf("\n\ntoken: %d\n\n", tokenz[curIndex]->tokenValue);
  if (tokenz[curIndex]->tokenValue != 19) // period
  {
    //printf("token val: %d", tokenz[curIndex]->tokenValue);
    //printf("index: %d token count: \n", curIndex);
    exit (0);
  }
  emit(9, 3);
}

void checkDuplicates (token **tokenz, int tokenCnt, char *identifier)
{
  int count = 0;

  for (int i = 0; i < tokenCnt; i++)
  {
    if (tokenz[i]->tokenValue != 2)
    {
      continue;
    }

    if (strcmp(identifier, tokenz[i]->identifier) == 0)
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
  assemblyTable[assemblyCnt].OP = OP;
  assemblyTable[assemblyCnt].L = 0;
  assemblyTable[assemblyCnt].M = M;
  assemblyCnt++;
  //printf("assemblyCnt now = %d\n", assemblyCnt);
}

char *printAssembly(int i)
{
  int halt = 0;

  switch (assemblyTable[i].OP)
  {
  case 1: // pushes M onto stack
    return "LIT";

    break;

  case 2: // returns from a subroutine and restores caller’s AR

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

  // loads val to top of stack from stack location at offset o from n lexicographical levels down
  case 3:
    return "LOD";

    break;

  // Stores val at top of stack in stack location at offset o from n lexicographical levels down
  case 4:
    return "STO";

    break;

  // calls procedure at code curIndex p, generates new activation record and setting PC to p
  case 5:
    return "CAL";

    break;

  // allocates m locals on stack
  case 6:
    return "INC";

    break;

  // jump to addr in stack and pop
  case 7:
    return "JMP";

    break;

  // jump conditionally, if val in stack[sp] is 0, then jumps to a and pop stack
  case 8:
    return "JPC";

    break;

  // prints val, reads val, or ends program
  case 9:

    switch (assemblyTable[i].M)
    {
    // Output of the value in stack[SP] to standard output as a character and pops
    case 1:
      return "SYS";

      break;

    // Reads an int as char value, from stdin and stores it on the top of the stack
    case 2:
      //int userInt;
      printf("Please Enter an Integer: \n");
      //scanf("%d", &userInt);
      printf("\tSIN\t");

      break;

    // stop program
    case 3:
      return "SYS";
      halt = 1;
      break;
    }
    }
    assemblyCnt++;

    return "Error";
}

// Create character token
token *makeToken(char *buffer)
{
  token *propToken = malloc(sizeof(token));

  strcpy(propToken->identifier, buffer);

  propToken->tokenValue = getToken(buffer);

  return propToken;
}

// Make special tokenz
token *getSpecial(char current, FILE *srcFile)
{
  int skipsym = 1, identsym = 2, numbersym = 3, plussym = 4, minussym = 5,
  multsym = 6,  slashsym = 7, oddsym = 8,  eqlsym = 9, neqsym = 10, lessym = 11,
  leqsym = 12, gtrsym = 13, geqsym = 14, lparentsym = 15, rparentsym = 16,
  commasym = 17, semicolonsym = 18, periodsym = 19, becomessym = 20,
  beginsym = 21, endsym = 22, ifsym = 23, thensym = 24, whilesym = 25, dosym = 26,
  callsym9 = 27, constsym = 28, varsym = 29, procsym9 = 30, writesym = 31,
  readsym = 32, elsesym9 = 33;

  token *propToken = malloc(sizeof(token));

  propToken->identifier[0] = current;

  if (current == '+')
  {
    propToken->tokenValue = plussym; // 4
  }
  else if (current == '-')
  {
    propToken->tokenValue = minussym; // 5
  }
  else if (current == '*')
  {
    propToken->tokenValue = multsym; // 6
  }
  else if (current == '/')
  {
    propToken->tokenValue = slashsym; // 7
  }
  else if (current == '(')
  {
    propToken->tokenValue = lparentsym; // 15
  }
  else if (current == ')')
  {
    propToken->tokenValue = rparentsym; // 16
  }
  else if (current == ',')
  {
    propToken->tokenValue = commasym; // 17
  }
  else if (current == '=')
  {
    propToken->tokenValue = eqlsym; // 9
  }
  else if (current == '.')
  {
    propToken->tokenValue = periodsym; // 19
  }
  else if (current == ';')
  {
    propToken->tokenValue = semicolonsym; // 18
  }
  else if (current == ':')
  {
    char prev = current;

    current = getc(srcFile);

    if (current == '=')
    {
      propToken->tokenValue = becomessym; // 20
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

    current = getc(srcFile);
    printf("2current = %c\n", current);

    if (current == '=')
    {
      propToken->tokenValue = leqsym; // 12
    }
    else if (isspace(current) || iscntrl(current))
    {
      propToken->tokenValue = lessym; // 11
    }
    else if (current == '>')
    {
      propToken->tokenValue = neqsym; // 10
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

    current = getc(srcFile);

    if (current == '=')
    {
      propToken->tokenValue = geqsym; // 14
    }
    else if (isspace(current) || iscntrl(current))
    {
      propToken->tokenValue = gtrsym; // 13
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

    current = getc(srcFile);

    if (current == '=')
    {
      propToken->tokenValue = neqsym; // 10
    }
    else
    {
      printf("ERROR: Invalid symbol\n");
      exit(0);
    }
  }
  return propToken;
}

// Makes a token attached to num
token *getNumToken(int *buffer, int length)
{
  token *propToken = malloc(sizeof(token));

  int ans= 0;
  int power = 1;

  for (int i = length - 2; i >= 0; i--)
  {
    ans= ans+ (buffer[i] * power);
    power = power * 10;
  }

  propToken->tokenValue = 3;
  propToken->integer = ans;

  return propToken;
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

  if (strcmp(buffer, "else99") == 0) // 33
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
  if (strcmp(buffer, "procedure99") == 0) // 30
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
  int flag = 0;
  int tokenCnt = 0;

  token **tokenz = malloc(515 * sizeof(token));
  symbol *SymbolTable = malloc (515 * sizeof(symbol));

  char *file_name = argv[1];
  FILE *srcFile = fopen(file_name, "r");

   /*printf("Source Program: \n");
   // Print source program
   while (!feof(srcFile))
   {
     current = getc(srcFile);
     if (current == EOF)
      break;
     printf("%c", current);
   }*/

  //srcFile = fopen(file_name, "r");

  while (!feof(srcFile))
  {
    //printf("in while\n");
    char buffer[515];
    if (flag == 1)
      break;

    current = getc(srcFile);

    if (current == EOF)
      break;

    // Skip whitespace
    if (isspace(current) || iscntrl(current))
    {
        continue;
    }

    // Skip comment
    if (current == '/' && !feof(srcFile))
    {
      // Checks if next char is *
      current = getc(srcFile);


      if (current == '*')
      {
        current = getc(srcFile);

        if (current == EOF)
        {
          printf("ERROR: Input ends during comment\n");
          exit(0);
          flag = 1;
          continue;
        }

        // loops through the comment
        while (current != '*')
        {
          current = getc(srcFile);
          if (current == EOF)
          {
            printf("ERROR: Input ends during comment\n");
            exit(0);
            flag = 1;
            break;
          }

          if (flag == 1)
            break;

          // Checks if end of comment
          if (current == '*')
          {
            current = getc(srcFile);

            // found end of comment
            if (current == '/')
            {
              break;
            }
          }
        }

        continue;
      }

      // If not, places back in file and resets state
      else
      {
          ungetc(current, srcFile);
          current = '/';
      }
    }

    // If a char finds token
    if (isalpha(current))
    {
      int i = 0;
      int lengthCheck = 0;

      // Scans in all valid chars
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
        current = getc(srcFile);
      }

      // If the Identifier too long skips making the token
      if (lengthCheck == 1)
      {
        continue;
      }

      // Puts invalid char back into file
      ungetc(current, srcFile);

      // Inserts null for strcmp 
      buffer[i] = '\0';

      // creates needed token 
      tokenz[tokenCnt++] = makeToken(buffer);
    }

    // Finds num token
    else if(isdigit(current))
    {
      int i = 0;
      int length = 1;
      int numBuffer[5];
      int numFlag = 0;

      // Grabs every valid number
      while (isdigit(current))
      {
        numBuffer[i] = current - '0';
        i++;
        length++;
        current = getc(srcFile);

        //num is too long
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

      // If number is too long continue on
      if (numFlag == 1)
      {
        continue;
      }

      ungetc(current, srcFile);

      // Makes  number token
      tokenz[tokenCnt++] = getNumToken(numBuffer, length);
    }
    // Special character
    else
    {
      tokenz[tokenCnt++] = getSpecial(current, srcFile);

      if (tokenz[tokenCnt - 1] == NULL)
      {
        tokenCnt--;
      }
    }
  }
  //printf("EO Program 2\n");

  if (tokenz[tokenCnt - 1]->tokenValue != 19)
  {
    printf("ERROR: Program must end with a period\n");
    exit(0);
  }

  int symCnt = 0;
  int address = 3;

  symbol **symbolTable = malloc(500 * sizeof(symbol));
  symbolTable[0] = malloc(sizeof(symbol));

  //printf ("creating table\n");
  // should be same
  symbolTable[0]->kind = 3;
  //printf ("currentname\n");
  strcpy(symbolTable[0]->name, "main");
  //printf("finished name\n");
  symbolTable[0]->val = 0;
  symbolTable[0]->level = 0;
  symbolTable[0]->addr = 3;
  symbolTable[0]->mark = 1;

  //printf("symbolTable made");
//trial for later
  // for (int i = 0; i < tokenCnt; i++)
  // {
  //   if (tokenz[i]->tokenValue == 2)
  //   {
  //     checkDuplicates(tokenz, tokenCnt, tokenz[i]->identifier);
  //   }
  // }

  program(symbolTable, tokenz);

  assemblyTable[0].OP = 7;
  assemblyTable[0].L = 0;
  assemblyTable[0].M = 3;


  printf("\nLine\t\tOP\t\t\tL\t\tM\n");

  for (int i = 0; i < assemblyCnt; i++)
  {
    //printf("in for loop\n");
    printf("%d\t\t%s\t\t\t0\t\t%d\n", i, printAssembly(i), assemblyTable[i].M);
  }
  
  printTable(symbolTable);
}
