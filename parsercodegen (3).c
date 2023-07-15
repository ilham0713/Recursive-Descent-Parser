// Isaac Tam & Ilham Mukati
// 3/1/23

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE 500
#define MAX_SYMBOL_TABLE_SIZE 500

typedef enum {   
    skipsym = 1, identsym, numbersym, plussym, minussym, 
    multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym, 
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym, 
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,  
    whilesym, dosym, callsym9, constsym, varsym, procsym9, writesym, 
    readsym , elsesym9 
} token_type; 

typedef enum opcode_type {
	LIT = 1, OPR = 2, LOD = 3, STO = 4, CAL = 5, INC = 6, JMP = 7, JPC = 8, 
	SYS = 9, WRT = 1, RED = 2, HLT = 3, 
	RTN = 0, ADD = 1, SUB = 2, MUL = 3, DIV = 4, EQL = 5, NEQ = 6,
	LSS = 7, LEQ = 8, GTR = 9, GEQ = 10, ODD = 11
} opcode_type;

typedef struct lexeme {
	token_type type;
	char identifier[12];
	int number;
	int error_name;
} lexeme;

typedef struct symbol {
	int kind;
	char name[10];
	int value;
	int level;
	int address;
	int mark;
} symbol;

typedef struct instruction {
	int op;
	int l;
	int m;
} instruction;

symbol *table;
instruction *code;
lexeme *tokens;

int t_idx = 0;
int table_idx = 0;
int c_idx = 0;
int error = 0;
int level;
char s_hold[11];
int numVars = 0;

int main(int argc, char *argv[])
{
  // Initialize
  int i;
  FILE *inFile;
	int buff;
  
  // DMA To Start
	table = calloc(MAX_SYMBOL_TABLE_SIZE, sizeof(symbol));
	code = calloc(ARRAY_SIZE, sizeof(instruction));
  tokens = calloc(ARRAY_SIZE, sizeof(lexeme));
	
	// Error For File
	if (argc < 2)
	{
		printf("ERROR: Include The Name of The File.\n");
		return 0;
	}
	
  // Scan the file
	inFile = fopen(argv[1], "r");
	while(fscanf(inFile, "%d", &buff) != EOF)
	{
		tokens[t_idx].type = buff;
		if (buff == identsym)
			fscanf(inFile, "%s", tokens[t_idx].identifier);
		else if (buff == numbersym)
			fscanf(inFile, "%d", &(tokens[t_idx].number));
		t_idx++;
	}

  // Close the file
	fclose(inFile);
	t_idx = 0;
	
  // Run PROGRAM
  PROGRAM();
	
  // Free
	free(table);
	free(code);
  free(tokens);

	return 0;
}

int SYMBOL_TABLE_CHECK(char name[])
{
	int i;
	for (i = 0; i < table_idx; i++)
		if (table[i].MARK == 0 && table[i].level == level && strcmp(name, table[i].name) == 0)
			return i;
	return -1;
}

void PROGRAM(){
  
  BLOCK();
  if(tokens[t_idx].type != periodsym){

    printf("ERROR: Program must end with period");
    error = -1;
    return;
  }

  EMIT(SYS,0,HLT);
    
  PRINT_ASB_CODE();
  PRINT_SYM_TAB();  
}

void BLOCK(){

    CONST_DECLARATION();
    numVars = VAR_DECLARATION();
    EMIT(INC, 0, 3 + numVars);
    STATEMENT();
}

void CONST_DECLARATION(){
  
  if(tokens[t_idx].type == constsym){
  
  do{
    
    t_idx++; 
    if(tokens[t_idx].type != identsym){
      printf("ERROR: CONST, VAR, READ Must be followed by identifier");
      error = -1;
      return;
    }
    
    if(SYMBOL_TABLE_CHECK(tokens[t_idx].type) != -1){
      printf("ERROR: Symbol name already declared");
      error = -1;
      return;
    }
    
    strcpy(s_hold, tokens[t_idx].identifier);
    t_idx++;
    if(tokens[t_idx].type != eqsym){
      printf("ERROR: Constant has to be assigned with =>");
      error = -1;
      return;
    }

    t_idx++;
    if(tokens[t_idx].type != numbersym){
      printf("ERROR: Constants has to be assigned integer value");
      error = -1;
      return;
    }
    
    SYM_ADD(1, s_hold, 0, level, 0);
    t_idx++;
  
  }

  while(tokens[t_idx].type == commasym);
    if(tokens[t_idx].type != semicolonsym){
      
      printf("ERROR: CONST and VAR dec must be followed by a semicolon");
      error = -1;
      return;
      }
    t_idx++;
     
  } 
} 

int VAR_DECLARATION(){

  numVars = 0;
  if(tokens[t_idx].type == varsym){

  do{

    t_idx++;
    numVars++;
    if(tokens[t_idx].type != identsym){
      
      printf("ERROR: CONST, VAR, READ Must be followed by identifier");
      error = -1;
      return 0;
    }

    if(SYMBOL_TABLE_CHECK(tokens[t_idx].type) != -1){
      printf("ERROR: Symbol name already declared");
      error = -1;
      return 0;
    }
    
    SYM_ADD(2, s_hold, 0, level, numVars + 3);
    t_idx++;
  }
  
  while(tokens[t_idx].type == commasym);
  if(tokens[t_idx].type != semicolonsym){
      printf("ERROR: CONST and VAR dec must be followed by a semicolon");
      error = -1;
      return 0;
    }
  t_idx++; 
  
  return numVars;
  }
}

void STATEMENT(){
  
  int symIdx;
  int jpcIdx;

  if(tokens[t_idx].type == identsym){
    symIdx = SYMBOL_TABLE_CHECK(tokens[t_idx].type);
    if(symIdx == -1){
      printf("ERROR: Symbol name already declared");
      error = -1;
      return;
  }

  if(table[symIdx].kind != 2){
    error = -1;
    printf("ERROR");
    
  }

  table_idx++;
  if(tokens[t_idx].type != becomessym){
    error = -1;
    printf("ERROR");
  }

  t_idx++;
  EXPRESSION();
  EMIT(STO,0,table[symIdx].address);
  return;
  }

  if(tokens[t_idx].type == beginsym){
    do{
      t_idx++;
      STATEMENT();
    }
    while(tokens[t_idx].type == semicolonsym);
      if(tokens[t_idx].type != endsym){
        error = -1;
        printf("ERROR");
      } 
    t_idx++;
    return;
  }

  if(tokens[t_idx].type == ifsym){
    
    t_idx++;
    CONDITION();
    jpcIdx = c_idx;
    EMIT(JPC, 0, jpcIdx);

    if(tokens[t_idx].type != thensym){
      error = -1;
      printf("ERROR");
    }

    t_idx++;
    STATEMENT();
    code[jpcIdx].m = c_idx;
    return;
  }

  if(tokens[t_idx].type == whilesym){
    
    t_idx++;
    int loopIdx = c_idx;
    CONDITION();
    if(tokens[t_idx].type != dosym){
      error = -1;
      printf("ERROR");
    }

    t_idx++;
    jpcIdx = c_idx;
    EMIT(JPC, 0, jpcIdx);
    STATEMENT();
    EMIT(JMP, 0, loopIdx);
    code[jpcIdx].m = c_idx;

    return;
  }

  if(tokens[t_idx].type == readsym){

    t_idx++;
    if(tokens[t_idx].type != identsym){
      error = -1;
      printf("ERROR");
    }

    symIdx = SYMBOL_TABLE_CHECK(tokens[t_idx].type);
    if(symIdx == -1){
      error = -1;
      printf("ERROR");
    }

    if(table[symIdx].kind != 2){
      error = -1;
      printf("ERROR");
    }

    t_idx++;
    EMIT(RED, 0, 0);
    EMIT(STO, 0, table[symIdx].address);

    return;
  }

  if(tokens[t_idx].type == writesym){
    t_idx++;
    EXPRESSION();
    EMIT(WRT, 0, 0);
    return;
  }
}

void CONDITION(){

  if(tokens[t_idx].type == oddsym){
    t_idx++;
    EXPRESSION();
    EMIT(ODD, 0, 11);
  }

  else{

    EXPRESSION();
    if(tokens[t_idx].type ==eqsym){
      t_idx++;
      EXPRESSION();
      EMIT(EQL, 0, 5);
    }
    else if(tokens[t_idx].type == neqsym){
      t_idx++;
      EXPRESSION();
      EMIT(NEQ, 0, 6);
    }
    else if(tokens[t_idx].type == lessym){
      t_idx++;
      EXPRESSION();
      EMIT(LSS, 0, 7);
    }
    else if(tokens[t_idx].type == leqsym){
    t_idx++;
    EXPRESSION();
    EMIT(LEQ, 0, 8);
    }
    else if(tokens[t_idx].type == gtrsym){
      t_idx++;
      EXPRESSION();
      EMIT(GTR, 0, 9);
    }
    else if(tokens[t_idx].type == geqsym){
      t_idx++;
      EXPRESSION();
      EMIT(GEQ, 0, 10);
    }
    else{
      printf("ERROR: Must contain comparison op");
      error = -1;
      return;
    }
  }
}

void EXPRESSION(){

  if(tokens[t_idx].type == minussym){

    t_idx++;
    TERM();
    //EMIT(NEG, 0, 0);
    while(tokens[t_idx].type == plussym || tokens[t_idx].type == minussym){
      
      if(tokens[t_idx].type == plussym){
        t_idx++;
        TERM();
        EMIT(ADD, 0, 1);
      }
      else{
        t_idx++;
        TERM();
        EMIT(SUB, 0, 2);
      }

    }
  }
}

void TERM(){
  
  FACTOR();
  while(tokens[t_idx].type == multsym){
    if(tokens[t_idx].type ==multsym){
      t_idx++;
      FACTOR();
      EMIT(MUL, 0, 3);
      }
      else if(tokens[t_idx].type == slashsym){
        t_idx++;
        FACTOR();
        EMIT(DIV, 0, 4);
      }
  }
}

void FACTOR(){

  if(tokens[t_idx].type == identsym){

    int symIdx = SYMBOL_TABLE_CHECK(tokens[t_idx].type);
    if(symIdx == -1){
      printf("ERROR: Symbol name already declared");
      error = -1;
      return;
    }
    if(table[symIdx].kind == 1){
    EMIT(LIT,0,table[symIdx].value);
    }
    else{
      EMIT(LOD,0,table[symIdx].address);
    }
    t_idx++;
  }
  else if(tokens[t_idx].type == numbersym){
      EMIT(LIT, 0, 1);
      t_idx++;
  }
  else if(tokens[t_idx].type == lparentsym){
      t_idx++;
      EXPRESSION();

    if(tokens[t_idx].type != rparentsym){
      printf("ERROR: Right par must follow left par");
      error = -1;
      return;
    }
    t_idx++;
  }
  else{
    printf("ERROR: Arithmetic equations must contain OP, PAR, NUM, or SYM");
    error = -1;
    return;
  }
}

void EMIT(int op, int l, int m)
{
	code[c_idx].op = op;
	code[c_idx].l = l;
	code[c_idx].m = m;
	c_idx++;
}

void SYM_ADD(int kind, char name[], int value, int level, int address)
{
	table[table_idx].kind = kind;
	strcpy(table[table_idx].name, name);
	table[table_idx].value = value;
	table[table_idx].level = level;
	table[table_idx].address = address;
	table[table_idx].MARK = 0;
	table_idx++;
}

void MARK()
{
	int i;
	for (i = table_idx - 1; i >= 0; i--)
	{
		if (table[i].MARK == 1)
			continue;
		if (table[i].level < level)
			return;
		table[i].MARK = 1;
	}
}

void PRINT_SYM_TAB()
{
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Value | Level | Address | Mark\n");
	printf("---------------------------------------------------\n");
	for (i = 0; i < table_idx; i++)
		printf("%4d | %11s | %5d | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].value, table[i].level, table[i].address, table[i].MARK); 
	printf("\n");
}

void PRINT_ASB_CODE()
{
	int i;
	printf("Assembly Code:\n");
	printf("Line\tOP Code\tOP Name\tL\tM\n");
	for (i = 0; i < c_idx; i++)
	{
		printf("%d\t%d\t", i, code[i].op);
		switch(code[i].op)
		{
			case LIT :
				printf("LIT\t");
				break;
			case OPR :
				switch (code[i].m)
				{
					case RTN :
						printf("RTN\t");
						break;
					case ADD : 
						printf("ADD\t");
						break;
					case SUB : 
						printf("SUB\t");
						break;
					case MUL : 
						printf("MUL\t");
						break;
					case DIV : 
						printf("DIV\t");
						break;
					case EQL : 
						printf("EQL\t");
						break;
					case NEQ : 
						printf("NEQ\t");
						break;
					case LSS : 
						printf("LSS\t");
						break;
					case LEQ : 
						printf("LEQ\t");
						break;
					case GTR : 
						printf("GTR\t");
						break;
					case GEQ : 
						printf("GEQ\t");
						break;
					default :
						printf("err\t");
						break;
				}
				break;
			case LOD :
				printf("LOD\t");
				break;
			case STO :
				printf("STO\t");
				break;
			case CAL :
				printf("CAL\t");
				break;
			case INC :
				printf("INC\t");
				break;
			case JMP :
				printf("JMP\t");
				break;
			case JPC : 
				printf("JPC\t");
				break;
			case SYS :
				switch (code[i].m)
				{
					case WRT : 
						printf("WRT\t");
						break;
					case RED :
						printf("RED\t");
						break;
					case HLT :
						printf("HLT\t");
						break;
					default :
						printf("err\t");
						break;
				}
				break;
			default :
				printf("err\t");
				break;
		}
		printf("%d\t%d\n", code[i].l, code[i].m);
	}
	printf("\n");
}