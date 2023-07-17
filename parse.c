#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int SymbolTableCheck();
void Program();
void Block();
void ConstDeclaration();
int VarDeclaration();
void Statement();
void Condition();
void Expression();
void Term();
void Factor();



typedef enum {   
skipsym = 1, identsym, numbersym, plussym, minussym, 
multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym, 
gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym, 
periodsym, becomessym, beginsym, endsym, ifsym, thensym,  
whilesym, dosym, callsym, constsym, varsym, procsym, writesym, 
readsym , elsesym } token_type; 

typedef enum opcode_name {
	LIT = 1, OPR = 2, LOD = 3, STO = 4, CAL = 5, INC = 6, JMP = 7, JPC = 8, 
	SYS = 9, WRT = 1, RED = 2, HLT = 3, 
	RTN = 0, ADD = 1, SUB = 2, MUL = 3, DIV = 4, EQL = 5, NEQ = 6,
	LSS = 7, LEQ = 8, GTR = 9, GEQ = 10
} opcode_name;

typedef struct lexeme {
	token_type type;
	char identifier_name[100];
	int number_value;
	int error_type;
} lexeme;

typedef struct instruction {
	int op;
	int l;
	int m;
} instruction;

typedef struct symbol {
	int kind;
	char name[10];
	int value;
	int level;
	int address;
	int mark;
} symbol;

#define MAX_SYMBOL_TABLE_SIZE 500

lexeme *tokens;
int symIdx = 0;
symbol *table;
int table_index = 0;
instruction *code;
int code_index = 0;
char saved_name[11];
int token_index = 0;

int error = 0;
int level;
int numVars = 0;

void emit(int op, int l, int m);
void add_symbol(int kind, char name[], int value, int level, int address);
void mark();
int multiple_declaration_check(char name[]);
int find_symbol(char name[], int kind);

void print_parser_error(int error_code, int case_code);
void print_assembly_code();
void print_symbol_table();

int main(int argc, char *argv[])
{
	// variable setup
	int i;
	tokens = calloc(MAX_SYMBOL_TABLE_SIZE, sizeof(lexeme));
	table = calloc(MAX_SYMBOL_TABLE_SIZE, sizeof(symbol));
	code = calloc(MAX_SYMBOL_TABLE_SIZE, sizeof(instruction));
	FILE *ifp;
	int buffer;
	
	// read in input
	if (argc < 2)
	{
		printf("Error : please include the file name\n");
		return 0;
	}
	
	ifp = fopen(argv[1], "r");
	while(fscanf(ifp, "%d", &buffer) != EOF)
	{
     
		tokens[token_index].type = buffer;
     
		if(buffer == identsym){
      
			fscanf(ifp, "%s", tokens[token_index].identifier_name);
      
      }
		else if (buffer == numbersym){
     
			fscanf(ifp, "%d", &(tokens[token_index].number_value));
      
      }
      
		token_index++;
	}
	fclose(ifp);
	symIdx = 0;
	
	
  Program();
	
	free(tokens);
	free(table);
	free(code);
	return 0;
}



void Program(){
  
   
   add_symbol(3,"main", 0, 0, 0);
    level = -1;
    
    Block();
    if(tokens[token_index].type!=periodsym){
        
         printf("Error:<program must end with period>");
        error = -1;
        return;
    }

    emit(SYS,0,HLT);
    
    print_assembly_code();
    print_symbol_table(); 
    


    
}

void Block(){
  
   level++;
    ConstDeclaration();
    numVars = VarDeclaration();
    int  M = 3 + numVars;
    emit(INC, 0, M);
    Statement();
    mark();
    level--;

    
}

void ConstDeclaration(){
 
  if(tokens[token_index].type == constsym){
  do{
    token_index++; 
    if(tokens[token_index].type != identsym){
      printf("Error:<Const, var, and read keywords must be followed by identifier>");
      error = -1;
      return;
    }
    
    if(SymbolTableCheck(tokens[token_index].type) != -1){
      printf("Error:<symbol name has already been declared>");
      error = -1;
      return;
    }
    
    strcpy(saved_name, tokens[token_index].identifier_name);
    token_index++;
    if(tokens[token_index].type != eqsym){
      printf("Error:<constants must be assigned with =>");
      error = -1;
      return;
    }
    token_index++;
    if(tokens[token_index].type != numbersym){
      printf("Error:<constants must be assigned an integer value>");
      error = -1;
      return;
    }
    //add to symbol table
    add_symbol(1, saved_name, 0, level, 0);
    //symIdx++;
  
  }
  while(tokens[token_index].type == commasym);
    if(tokens[token_index].type != semicolonsym){
      
      printf("Error:<constant and variable declarations must be followed by a semicolon>");
      error = -1;
      return;
      }
    token_index++;
     
  }
  
  
    

} 

int VarDeclaration(){
 
  numVars = 0;
  if(tokens[token_index].type == varsym){
  do{
    numVars++;
    token_index++;
    if(tokens[token_index].type != identsym){
      printf("Error:<const, var, and read keywords must be followed by identifier>");
      error = -1;
      return 0;
    }
    if(SymbolTableCheck(tokens[token_index].type) != -1){
      printf("Error:<symbol name has already been declared>");
      error = -1;
      return 0;
    }
    //add to symbol table
    add_symbol(2, saved_name, 0, level, numVars + 3);
    symIdx++;
  }
  
  while(tokens[token_index].type == commasym);
  if(tokens[token_index].type != semicolonsym){
      printf("Error:<constant and variable declarations must be followed by a semicolon>");
      error = -1;
      return 0;
    }
  token_index++; 
  
  return numVars;
}
}

void Statement(){
 
 //printf("%d",symIdx);
int jpcIdx;
int check;
  if(tokens[token_index].type == identsym){
    check =SymbolTableCheck(tokens[token_index].type);
    if(check == -1){
      printf("Error:<symbol name has already been declared>");
      error = -1;
      return;
  }
  if(table[symIdx].kind != 2){
    printf("Error:<only variable values may be altered>");
    error = -1;
    return;
  }
  token_index++;
  //printf("%d",table_index);
  if(tokens[token_index].type != becomessym){
    printf("Error:<assignment statements must use :=>");
    error = -1;
    return;
  }
  token_index++;
  Expression();
  emit(STO,0,table[symIdx].address);
  return;
}
  if(tokens[token_index].type == beginsym){
    do{
      token_index++;
      Statement();
      }
    while(tokens[token_index].type == semicolonsym);
     if(tokens[token_index].type != endsym){
       printf("Error:<begin must be followed by end>");
       error = -1;
       return;
     } 
    token_index++;
    return;
  }
  if(tokens[token_index].type == ifsym){
    token_index++;
    Condition();
    jpcIdx = code_index;
    //emit(JPC,0,);
    if(tokens[token_index].type != thensym){
      printf("Error:<if must be followed by then>");
      error = -1;
      return;
    }
    token_index++;
    Statement();
    code[jpcIdx].m = code_index;
    return;
    
    }
  if(tokens[token_index].type == whilesym){
    token_index++;
    int loopIdx = code_index;
    Condition();
    if(tokens[token_index].type != dosym){
      printf("Error:<while must be followed by do>");
      error = -1;
      return;
    }
    token_index++;
    jpcIdx = code_index;
    //emit(JPC,,);
    Statement();
    emit(JMP,0,loopIdx);
    code[jpcIdx].m = code_index;
    return;
  }
  if(tokens[token_index].type == readsym){
    token_index++;
    if(tokens[token_index].type != identsym){
      printf("Error:<const,var,and read keywords must be followed by identifier>");
      error = -1;
      return;
    }
    check = SymbolTableCheck(tokens[symIdx].type);
    if(check == -1){
      printf("Error:<symbol name has already been declared>");
        error = -1;
        return;
    }
    if(table[symIdx].kind != 2){
      printf("Error:<only variable values may be altered>");
        error = -1;
        return;
    }
    token_index++;
    //emit(RED,0,);
    emit(STO,0,table[symIdx].address);
    return;
  }
  if(tokens[token_index].type == writesym){
    token_index++;
    Expression();
    //emit(WRT,0,);
    return;
  }
}
void Condition(){
 
  if(tokens[token_index].type == oddsym){
    token_index++;
    Expression();
    //emit(ODD,0,);
  }
  else{
    Expression();
    if(tokens[token_index].type ==eqsym){
      token_index++;
      Expression();
      //emit(EQL,0,);
    }
    else if(tokens[token_index].type == neqsym){
      token_index++;
      Expression();
      //emit(NEQ,0,);
    }
    else if(tokens[token_index].type == lessym){
      token_index++;
      Expression();
      //emit(LSS,0,);
  }
  else if(tokens[token_index].type == leqsym){
    token_index++;
    Expression();
    //emit(LEQ,0,);
  }
  else if(tokens[token_index].type == gtrsym){
    token_index++;
    Expression();
    //emit(GTR,0,);
  }
  else if(tokens[token_index].type == geqsym){
    token_index++;
    Expression();
    //emit(GEQ,0,);
  }
  else{
    printf("Error:<condition must contain comparison operator>");
    error = -1;
    return;
  }
}
}
void Expression(){
 
  if(tokens[token_index].type == minussym){
    token_index++;
    Term();
    //emit(NEG,0,);
    while(tokens[token_index].type == plussym || tokens[token_index].type == minussym){
      if(tokens[token_index].type == plussym){
      token_index++;
      Term();
      //emit(ADD,0,);
      }
      else{
        token_index++;
        Term();
        //emit(SUB,0,);
      }
}
}
else{
  if(tokens[token_index].type == plussym){
    token_index++;
  }
  Term();
  while(tokens[token_index].type == plussym || tokens[token_index].type == minussym){
    if(tokens[token_index].type == plussym){
      token_index++;
      Term();
      //emit(ADD,0,);
          }
    else{
      token_index++;
      Term();
      //emit(SUB,0,);
    }
  }
}
}
void Term(){

  Factor();
  while(tokens[token_index].type == multsym || tokens[token_index].type == slashsym){
    if(tokens[token_index].type ==multsym){
      token_index++;
      Factor();
      //emit(MUL,0,);
      }
      else if(tokens[token_index].type == slashsym){
        token_index++;
        Factor();
        //emit(DIV,0,);
      }
      
}
}
void Factor(){
int check;
 
 if(tokens[token_index].type == identsym){
   int check = SymbolTableCheck(tokens[symIdx].type);
   if(check == -1){
     printf("Error:<symbol name has already been declared>");
     error = -1;
     return;
   }
 if(table[symIdx].kind == 1){
   emit(LIT,0,table[symIdx].value);
 }
 else{
   emit(LOD,0,table[symIdx].address);
 }
 token_index++;
}
else if(tokens[token_index].type == numbersym){
  //emit(LIT,0,);
  token_index++;
}
else if(tokens[token_index].type == lparentsym){
    token_index++;
    Expression();
    if(tokens[token_index].type != rparentsym){
      printf("Error:<right parenthesis must follow left parentheses>");
      error = -1;
      return;
    }
    token_index++;
}
else{
  printf("Error:<arithmetic equations must contain operands, parentheses, numbers, or symbols>");
  error = -1;
  return;
  
}
}



// adds a new instruction to the end of the code
void emit(int op, int l, int m)
{
 
	code[code_index].op = op;
	code[code_index].l = l;
	code[code_index].m = m;
	code_index++;
}

// adds a new symbol to the end of the table
void add_symbol(int kind, char name[], int value, int level, int address)
{
 
	table[symIdx].kind = kind;
	strcpy(table[symIdx].name, name);
	table[symIdx].value = value;
	table[symIdx].level = level;
	table[symIdx].address = address;
	table[symIdx].mark = 0;
	symIdx++;
}

// marks all of the current procedure's symbols
void mark()
{

	int i;
	for (i = table_index - 1; i >= 0; i--)
	{
		if (table[i].mark == 1)
			continue;
		if (table[i].level < level)
			return;
		table[i].mark = 1;
	}
}

// returns -1 if there are no other symbols with the same name within this procedure
int SymbolTableCheck(char name[])
{
 
	int i;
	for (i = 0; i < table_index; i++)
		if (table[i].mark == 0 && table[i].level == level && strcmp(name, table[i].name) == 0)
			return i;
	return -1;
}


void print_assembly_code()
{
	int i;
	printf("Assembly Code:\n");
	printf("Line\tOP Code\tOP Name\tL\tM\n");
	for (i = 0; i < code_index; i++)
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
					case ADD : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("ADD\t");
						break;
					case SUB : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("SUB\t");
						break;
					case MUL : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("MUL\t");
						break;
					case DIV : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("DIV\t");
						break;
					case EQL : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("EQL\t");
						break;
					case NEQ : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("NEQ\t");
						break;
					case LSS : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("LSS\t");
						break;
					case LEQ : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("LEQ\t");
						break;
					case GTR : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("GTR\t");
						break;
					case GEQ : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
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
			case JPC : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
				printf("JPC\t");
				break;
			case SYS :
				switch (code[i].m)
				{
					case WRT : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
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

void print_symbol_table()
{
//printf("%d", table_index);
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Value | Level | Address | Mark\n");
	printf("---------------------------------------------------\n");
	for (i = 0; i <symIdx ; i++)
		printf("%4d | %11s | %5d | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].value, table[i].level, table[i].address, table[i].mark); 
	printf("\n");
}
