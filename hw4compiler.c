#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>


#define MAX_SOURCE_FILE_SIZE 1000 // chars
#define MAX_BYTECODE_SIZE 1000 // bytes
#define MAX_SYMBOL_TABLE_SIZE 120 // symbols
#define NOT_IMPL { printf("Not implemented [line: %d]\n", __LINE__); exit(2); }


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
void lex_analyze();


typedef enum {  
    skipsym = 1, identsym, numbersym, plussym, minussym, 
    multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym, 
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym, 
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,  
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym, 
    readsym, elsesym } token_type; 


typedef struct {
    int kind; // const = 1, var = 2, proc = 3
    char* name;
    int val;
    int level;
    int addr;
    int mark;
} symbol;


token_type* token_types;
int chrs = 0;
int cnt = 0;
int cnt1 = 0;
int cnt2 = 0;
int numbers = 0;
char characters[100][12];
int arr[100];
int line = 1;
char* input;
int input_len = 0;
char* filename;
// this index is used to store instructions while generating them
int gen_index = 0;
// this index is used to read tokens
int token_index = 0;
// contains all declared symbol names and info about them
symbol symbols_table[15][MAX_SYMBOL_TABLE_SIZE];
int symbols_index[15];
int tables_len = 0;

int numbers_index = 0;
int idents_index = 0;

int sp = 3;

//holds our op, l and the m
typedef struct {
    int OP;
    int L;
    int M;
} isa;

//implicit declaration of functions
int parse_block();
symbol mark(char* ident);
void error(int code);
void stack_out(int PC, int BP, int SP, int GP, int *PAS, int *cols);
void header_print(isa IR);
int base(int BP, int L);
void error_no_code(const char* message);

//faster way to get index, and it makes the code cleaners
int tables_index() {
    return tables_len - 1;
}

char* concat(const char* l, const char* r) {
    int llen = strlen(l);
    int rlen = strlen(r);
    int len = llen + rlen;
    char* ptr = malloc(len + 1);
    ptr[len] = '\0';

    strcpy(ptr, l);
    strcpy(ptr + llen, r);

    return ptr;
}

int* PAS;

void header_print(isa IR){
    switch (IR.OP)
    {
    case 1:
        printf("%s \t| %d \t| %d\n", "LIT", IR.L, IR.M);
        break;
    case 2:
        switch (IR.M)
        {
        case 0:
            printf("%s \t| %d \t| %d\n", "RTN", IR.L, IR.M);
            break;
        case 1:
            printf("%s \t| %d \t| %d\n", "ADD", IR.L, IR.M);
            break;
        case 2:
            printf("%s \t| %d \t| %d\n", "SUB", IR.L, IR.M);
            break;
        case 3:
            printf("%s \t| %d \t| %d\n", "MUL", IR.L, IR.M);
            break;
        case 4:
            printf("%s \t| %d \t| %d\n", "DIV", IR.L, IR.M);
            break;
        case 5:
            printf("%s \t| %d \t| %d\n", "EQL", IR.L, IR.M);
            break;
        case 6:
            printf("%s \t| %d \t| %d\n", "NEQ", IR.L, IR.M);
            break;
        case 7:
            printf("%s \t| %d \t| %d\n", "LSS", IR.L, IR.M);
            break;
        case 8:
            printf("%s \t| %d \t| %d\n", "LEQ", IR.L, IR.M);
            break;
        case 9:
            printf("%s \t| %d \t| %d\n", "GTR", IR.L, IR.M);
            break;
        case 10:
            printf("%s \t| %d \t| %d\n", "GEQ", IR.L, IR.M);
            break;
        default:
            printf("%s \t| %d \t| %d\n", "ERR", IR.L, IR.M);
            break;
        }
        break;
    case 3:
        printf("%s \t| %d \t| %d\n", "LOD", IR.L, IR.M);
        break;
    case 4:
        printf("%s \t| %d \t| %d\n", "STO", IR.L, IR.M);
        break;
    case 5:
        printf("%s \t| %d \t| %d\n", "CAL", IR.L, IR.M);
        break;
    case 6:
        printf("%s \t| %d \t| %d\n", "INC", IR.L, IR.M);
        break;
    case 7:
        printf("%s \t| %d \t| %d\n", "JMP", IR.L, IR.M);
        break;
    case 8:
        printf("%s \t| %d \t| %d\n", "JPC", IR.L, IR.M);
        break;
    case 9:
        switch (IR.M)
        {
        case 1:
            printf("%s \t| %d \t| %d\n", "SOU", IR.L, IR.M);
            break;
        case 2:
            printf("%s \t| %d \t| %d\n", "SIN", IR.L, IR.M);
            break;
        case 3:
            printf("%s \t| %d \t| %d\n", "EOP", IR.L, IR.M);
            break;
        default:
            printf("%s \t| %d \t| %d\n", "ERR", IR.L, IR.M);
            break;
        }
        break;

    default:
        printf("%s \t| %d \t| %d\n", "ERR", IR.L, IR.M);
        break;
    }
}

// prints key words and identifiers/tokens
void token_printer(char* c){
    if(strcmp(c, "main") == 0 || strcmp(c, "null") == 0){
        error_no_code("identifiers cannot be named 'null' or 'main'");
    }
    else if(strcmp(c,"const") == 0){
        token_types[cnt] = constsym;
        cnt++;
    }
    else if(strcmp(c,"var") == 0){
        token_types[cnt] = varsym;
        cnt++;
    }
    else if(strcmp(c,"odd") == 0){
        token_types[cnt] = oddsym;
        cnt++;
    }
    else if(strcmp(c,"procedure") == 0){
        token_types[cnt] = procsym;
        cnt++;
    }
    else if(strcmp(c,"begin") == 0){
        token_types[cnt] = beginsym;
        cnt++;
    }
    else if(strcmp(c,"end") == 0){
        token_types[cnt] = endsym;
        cnt++;
    }
    else if(strcmp(c,"call") == 0){
        token_types[cnt] = callsym;
        cnt++;
    }
    else if(strcmp(c,"if") == 0){
        token_types[cnt] = ifsym;
        cnt++;
    }
    else if(strcmp(c,"then") == 0){
        token_types[cnt] = thensym;
        cnt++;
    }
    else if(strcmp(c,"else") == 0){
        token_types[cnt] = elsesym;
        cnt++;
    }
    else if(strcmp(c,"while") == 0){
        token_types[cnt] = whilesym;
        cnt++;
    }
    else if(strcmp(c,"do") == 0){
        token_types[cnt] = dosym;
        cnt++;
    }
    else if(strcmp(c,"read") == 0){
        token_types[cnt] = readsym;
        cnt++;
    }
    else if(strcmp(c,"write") == 0){
        token_types[cnt] = writesym;
        cnt++;
    } // if not a reserved word it is an identifier
    else{
        token_types[cnt] = identsym;
        cnt++;
        strcpy(characters[cnt2],c);
        cnt2+=1;
    }
}

// prints symbols and their tokens
int print_symb(const char* source_code, int index){
    if(source_code[index] == '.'){
        token_types[cnt] = periodsym;
        cnt++;
        index++;
    }
    // checks for becomessym
    else if(source_code[index] == ':'){
        index++;
        if(source_code[index] == '='){
            token_types[cnt] = becomessym;
            cnt++;
            index++;
        }
        else{
            error(22);
        }    
    }
    else if(source_code[index] =='-'){
        token_types[cnt] = minussym;
        cnt++;
        index++;
    }
    else if(source_code[index] ==';'){
        token_types[cnt] = semicolonsym;
        cnt++;
        index++;
    }
    else if(source_code[index] ==','){
        token_types[cnt] = commasym;
        cnt++;
        index++;
    }
    else if(source_code[index]=='>'){
        index++;
        if(source_code[index] == '='){
            token_types[cnt] = geqsym;
            cnt++;
            index++;
        }
        else{
            if(source_code[index - 1] == '>' && source_code[index-2] != '<'){
                token_types[cnt] = gtrsym;
                cnt++;
            }
        }
    }
    else if(source_code[index] == '='){
        index++;
        if(source_code[index] != ':') {
            token_types[cnt] = eqsym;
            cnt++;
        }
    }
    // checks for <, <=, and <>
    else if(source_code[index] =='<'){
        index++;
        if(source_code[index] == '='){
            token_types[cnt] = leqsym;
            cnt++;
            index++;
        }
        else if(source_code[index] == '>'){
            token_types[cnt] = neqsym;
            cnt++;
            index++;
        }
        else{
            token_types[cnt] = lessym;
            cnt++;
        }
    }
    else if(source_code[index]=='+'){
        token_types[cnt] = plussym;
        cnt++;
        index++;
    }
    else if(source_code[index]=='/'){
        token_types[cnt] = slashsym;
        cnt++;
        index++;
    }
    else if(source_code[index]=='*'){
        token_types[cnt] = multsym;
        cnt++;
        index++;
    }
    else if(source_code[index]==')'){
        token_types[cnt] = rparentsym;
        cnt++;
        index++;
    }
    else if(source_code[index]=='('){
        token_types[cnt] = lparentsym;
        cnt++;
        index++;
    }
    else{
        error(22);
    }

    return index;
}

//takes care of error output
const char* error_code(int code) {
    switch (code) {
    case 1: return "Program must end with period";
    case 2: return "Const must be followed by identifier";
    case 3: return "Var must be followed by identifier";
    case 4: return "Read must be followed by identifier";
    case 5: return "Symbol name already declared";
    case 6: return "Constants must be assigned with =";
    case 7: return "Consts must be assigned integer values";
    case 8: return "Constant declarations must be followed by semicolon";
    case 9: return "Variable declarations must be followed by semicolon";
    case 10: return "Undeclared identifier";
    case 11: return "Only variable values may be altered";
    case 12: return "Assignment statements must use :=";
    case 13: return "Begin must be followed by end";
    case 14: return "If must be followed by then";
    case 15: return "While must be followed by do";
    case 16: return "Condition must contain comparison operator";
    case 17: return "Right parenthesis must follow left parenthesis";
    case 18: return "Arithmetic equations must contain operands";
    case 19: return "procedure must be followed by identifier";
    case 20: return "call must be followed by identifier";
    case 21: return "Cannot use assignment with procedure";
    case 22: return "invalid symbol";
    case 23: return "Only procedures can be called";
    case 24: return "Expression cannot contain procedure";
    case 25: return "Identifier too long";
    case 26: return "Number too large";
    case 27: return "Expression can't begin with or use this symbol";
    case 28: return "factor can't use relational operators";
    case 29: return "invalid operation for statement";
    case 30: return "number expected";
    case 31: return "Only procedures can be called";
    default:
        NOT_IMPL;
    }
}

void error_no_line(const char* message) {
    printf("Error %s \n", message);
    exit(1);
}

void error_no_code(const char* message) {
    printf("%s\n", message);
    exit(1);
}

char* int_to_string(int n) {
    int num_chars = n == 0 ? 1 : (n < 0 ? 2 : 1);
    int temp = n;
    while (temp != 0) {
        num_chars++;
        temp /= 10;
    }

    char* str = (char*) malloc(num_chars * sizeof(char) + 1);

    if (n == 0) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    int i = 0;
    if (n < 0) {
        str[i++] = '-';
        n = -n;
    }

    while (n != 0) {
        int digit = n % 10;
        str[i++] = digit + '0';
        n /= 10;
    }

    str[i] = '\0';

    int start = str[0] == '-' ? 1 : 0;
    int end = i - 1;
    while (start < end) {
        char tmp = str[start];
        str[start++] = str[end];
        str[end--] = tmp;
    }

    return str;
}

void error(int code) {
    char* prefix = concat(concat("Error ", int_to_string(code)), ", ");

    error_no_code(
        concat(prefix, error_code(code))
    );
}

void lex_analyze(const char* source_code) {
    char buffarr[12];
    int i = 0;
    int j = 0;

    while(source_code[i] != '\0'){
    //starting identifies the lexeme and respective tokens
    
    //first char being stored is letter
    if(isalpha(source_code[i])){
        j = 0;
        while (isalpha(source_code[i]) || isdigit(source_code[i])){
            buffarr[j++] = source_code[i++];
            if(j == 11){
                break;
            }

        }
        // checks for identifer len over 11 throws error
        if(isalpha(source_code[i]) || isdigit(source_code[i])){
            error(25);
            while(isalpha(source_code[i]) || isdigit(source_code[i])){
                i++;

            }
        
        }
        else{
            buffarr[j] = '\0';
            token_printer(buffarr);
        }
    }
    // first character being stored in buffarr is a digit
    else if(isdigit(source_code[i])){
        j = 0;
        while(isdigit(source_code[i])){
            buffarr[j++] = source_code[i++];
            if(j==5){
                break;
            }
        }
        if(isalpha(source_code[i])){
            error_no_code("identifiers cannot begin with digits");
            while(isalpha(source_code[i]) || isdigit(source_code[i])){
                i++;
            }
        }
        else if(isdigit(source_code[i])){
            error(26);
            while(isalpha(source_code[i]) || isdigit(source_code[i])){
                i++;
            }
        }
        else{
            buffarr[j] = '\0';
            arr[cnt1] = atoi(buffarr);
            token_types[cnt] = numbersym;
            cnt++;
            cnt1++;
            
        }
    }
    //if a blank space or cntrl character is hit
    else if(source_code[i] == ' ' || iscntrl(source_code[i])){
        if (source_code[i] == '\n')
            line++;

        i++;
    }
    
    //if comment is hit
    else if(source_code[i] == '/' && source_code[i+1] =='*'){
        i++;
        i++;

        while(true){
            if (source_code[i] == '*' && source_code[i+1] == '/') {
                i++;
                i++;
                break;
            }

            i++;
            if(source_code[i] == EOF){
                error_no_code("input ended during comment");
            }
        }
    }
    //if a symbol is hit
    else{
        i = print_symb(source_code, i);
    }
    }
}

/*
    Examples:
        // read
        emit(9, 0, 2);
        // load 5
        emit(1, 0, 5);
        // add
        emit(2, 0, 1);
        // write
        emit(9, 0, 1);
        // halt
        emit(9, 0, 3);
*/

void emit(int op, int l, int m) {
    PAS[gen_index++] = op;
    PAS[gen_index++] = l;
    PAS[gen_index++] = m;
}

/*
program ::= block "." .
block ::= const-declaration var-declaration procedure-declaration statement.
constdeclaration ::= ["const" ident "=" number {"," ident "=" number} ";"].
var-declaration ::= [ "var "ident {"," ident} “;"].
procedure-declaration ::= { "procedure" ident ";" block ";" }
statement ::= [ ident ":=" expression
    | "call" ident
    | "begin" statement { ";" statement } "end" 
    | "if" condition "then" statement
    | "while" condition "do" statement
    | "read" ident
    | "write" expression
    | empty ] .
condition ::= "odd" expression
    | expression rel-op expression.
rel-op ::= "="|“< >"|"<"|"<="|">"|">=“.
expression ::= [ "+"|"-"] term { ("+"|"-") term}.
term ::= factor {("*"|"/") factor}.
factor ::= ident | number | "(" expression ")“.
number ::= digit {digit}.
ident ::= letter {letter | digit}.
digit ;;= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9“.
letter ::= "a" | "b" | … | "y" | "z" | "A" | "B" | ... | "Y" | "Z".
*/

void declare_symbol(symbol sym) {
    for (int i = 0; i < symbols_index[tables_index()]; i++)
        if (strcmp(symbols_table[tables_index()][i].name, sym.name) == 0)
            error(5);

    sym.mark = false;
    symbols_table[tables_index()][symbols_index[tables_index()]++] = sym;
}

void Condition();
void parse_expression();

int match_token(token_type type) {
    return token_types[token_index] == type;
}

int eat() {
    return token_types[token_index++];
}

int expect_token(token_type type) {
    if (!match_token(type))
        error_no_code("unexpected token");
    
    return eat();
}

int expect_number() {
    expect_token(numbersym);
    return arr[numbers_index++];
}

char* expect_ident() {
    expect_token(identsym);
    return characters[idents_index++];
}

//match_token function makes the code much cleaner but still implements block correctly
void Block(){
    if (!match_token(constsym))
        return;

    eat();

    if (!match_token(identsym))
        error(2);
    
    char* ident = expect_ident();

    if (!match_token(eqsym))
        error(6);

    expect_token(eqsym);

    if (!match_token(numbersym))
        error(7);

    int num = expect_number();

    declare_symbol((symbol) {
        .kind = 1,
        .name = ident,
        .val = num,
    });

    while (match_token(commasym)) {
        eat();

        if (!match_token(identsym))
            error(2);

        char* ident = expect_ident();

        if (!match_token(eqsym))
            error(6);

        expect_token(eqsym);

        if (!match_token(numbersym))
            error(7);

        int num = expect_number();

        declare_symbol((symbol) {
            .kind = 1,
            .name = ident,
            .val = num,
        });
    }

    if (!match_token(semicolonsym))
        error(8);

    expect_token(semicolonsym);
}

void parse_var_declaration() {
    if (!match_token(varsym))
        return;

    eat();

    if (!match_token(identsym))
        error(3);
    
    char* ident = expect_ident();
    declare_symbol((symbol) {
        .kind = 2,
        .name = ident,
        .addr = sp++,
    });

    while (match_token(commasym)) {
        eat();

        if (!match_token(identsym))
            error(3);

        char* ident = expect_ident();
        declare_symbol((symbol) {
            .kind = 2,
            .name = ident,
            .addr = sp++,
        });
    }

    if (!match_token(semicolonsym))
        error(9);

    expect_token(semicolonsym);
}

void Statement() {
    int cur = eat();

    char* ident;
    int m_index;
    symbol sym;
    int head_addr;
    if(cur == identsym){
        ident = characters[idents_index++];
        if (!match_token(becomessym))
            error(12);
        expect_token(becomessym);
        parse_expression();
        // store
        sym = mark(ident);
        if (sym.kind == 1)
            error(11);
        if (sym.kind == 3)
            error(21);
        emit(4, sym.level, sym.addr);
        
    }else if(cur==beginsym){
        Statement();
        if (match_token(skipsym))
            error(13);
        // the semicolon is optional
        if (match_token(endsym)) {
            eat();
        }
        expect_token(semicolonsym);
        while (!match_token(endsym)) {
            Statement();
            // the semicolon is optional
            if (match_token(endsym))
                break;
                expect_token(semicolonsym);
            }
        // endsym
        eat();
        

    }else if(cur==callsym){
        if (!match_token(identsym))
                error(20);

            ident = expect_ident();
            sym = mark(ident);

            if (sym.kind != 3)
                error(23);

            emit(5, 0, sym.addr);
    }else if(cur==ifsym){
        Condition();

        if (!match_token(thensym))
            error(14);

        expect_token(thensym);

        emit(8, 0, -1);
        m_index = gen_index - 1;

        Statement();

        PAS[m_index] = gen_index;

    }else if(cur==whilesym){
        head_addr = gen_index;
            
        Condition();

        if (!match_token(dosym))
            error(15);

        expect_token(dosym);

        emit(8, 0, -1);
        m_index = gen_index - 1;

        Statement();
        emit(7, 0, head_addr);

        PAS[m_index] = gen_index;
    }else if(cur == readsym){
        if (!match_token(identsym))
            error(4);

        ident = expect_ident();
        sym = mark(ident);

        // read
        emit(9, 0, 2);
        // store
        emit(4, sym.level, sym.addr);
    }else if(cur==writesym){
        parse_expression();
        // write
        emit(9, 0, 1);
        
        // empty
    }else{
        switch (cur) {
            case eqsym:
            case neqsym:
            case lessym:
            case leqsym:
            case gtrsym:
            case geqsym:
                error(29);
            default:
                // Code
                break;
            }
        token_index--;


    }
        
            
}
//marks procedure's symbols
symbol mark(char* ident) {
    for (int j = tables_index(); j >= 0; j--) {
        for (int i = 0; i < symbols_index[j]; i++) {
            if (strcmp(symbols_table[j][i].name, ident) != 0)
                continue;
            
            if (symbols_table[j][i].mark)
                error_no_code(concat("unable to use deleted identifier ", ident));

            symbols_table[j][i].level = tables_index() - j;
            return symbols_table[j][i];
        }
    }
    
    error(10);
    return (symbol) {
        .kind = -1
    };
}

void Factor() {
    int cur = token_types[token_index];

    char* ident;
    int num;
    symbol sym;

    switch (cur) {
        case identsym:
            ident = expect_ident();
            sym = mark(ident);

            if (sym.kind == 1)
                emit(1, 0, sym.val);
            else if (sym.kind == 3)
                error(24);
            else
                emit(3, sym.level, sym.addr);
            break;
        
        case numbersym:
            num = expect_number();
            emit(1, 0, num);
            break;
        
        case lparentsym:
            eat();
            parse_expression();

            if (!match_token(rparentsym))
                error(17);
            
            expect_token(rparentsym);
            break;
        
        default:
            error(27);
    }

    switch (token_types[token_index]) {
    case eqsym:
    case neqsym:
    case lessym:
    case leqsym:
    case gtrsym:
    case geqsym:
        error(28);
    
    default:
        break;
    }
}

void parse_term() {
    Factor();

    while (match_token(multsym) || match_token(slashsym)) {
        int op = eat();

        if (match_token(semicolonsym))
            error(18);

        Factor();

        if (op == multsym)
            emit(2, 0, 3);
        else
            emit(2, 0, 4);
    }
}

void parse_expression() {
    int neg = false;
    if (match_token(plussym) || match_token(minussym)) {
        int e = eat();
        
        switch (e) {
        case minussym:
            // load 0
            emit(1, 0, 0);
            neg = true;
            break;
        case eqsym:
        case neqsym:
        case lessym:
        case leqsym:
        case gtrsym:
        case geqsym:
            error(27);

        }
    }
    
    parse_term();

    while (match_token(plussym) || match_token(minussym)) {
        int op = eat();

        if (match_token(semicolonsym))
            error(18);

        parse_term();

        if (op == plussym)
            emit(2, 0, 1);
        else
            emit(2, 0, 2);
    }

    // 0 - expr
    if (neg)
        emit(2, 0, 2);
}

int parse_rel_op() {
    int op = eat();

    switch (op) {
        case eqsym:
        case neqsym:
        case lessym:
        case leqsym:
        case gtrsym:
        case geqsym:
            return op;

        default:
            error(16);
            return -1;
    }
}

void Condition() {
    if (match_token(oddsym)) {
        NOT_IMPL;
        eat();
        parse_expression();
        return;
    }

    parse_expression();
    int op = parse_rel_op();
    parse_expression();

    switch (op) {
        case eqsym:
            emit(2, 0, 5);
            break;
        
        case neqsym:
            emit(2, 0, 6);
            break;
        
        case lessym:
            emit(2, 0, 7);
            break;

        case leqsym:
            emit(2, 0, 8);
            break;

        case gtrsym:
            emit(2, 0, 9);
            break;
        
        case geqsym:
            emit(2, 0, 10);
            break;
    }
}

void parse_procedure_declaration() {
    while (match_token(procsym)) {
        // procedure
        eat();

        if (!match_token(identsym))
            error(19);

        char* ident = expect_ident();
        expect_token(semicolonsym);

        declare_symbol((symbol) {
            .kind = 3,
            .addr = gen_index,
            .name = ident,
        });
        
        parse_block();
        Statement();
        emit(2, 0, 0);
        expect_token(semicolonsym);
    }
}

int parse_block() {
    sp = 4;
    tables_len++;
    Block();
    parse_var_declaration();
    int locals_sp = sp;
    parse_procedure_declaration();

    int procedure_index = gen_index;

    // allocating stack space for locals
    emit(6, 0, locals_sp);
    Statement();
    tables_len--;

    return procedure_index;
}
//
void Program() {
    emit(7, 0, -1);
    int m_index = gen_index - 1;
    int main_index = parse_block();
    PAS[m_index] = main_index;

    if (!match_token(periodsym))
        error(1);

    expect_token(periodsym);
}

void parse() {
    // int cur;
    // while (cur != skipsym && cur != 0) {
    //     cur = token_types[token_index++];
    //     printf("%d", cur);
    // 
    //     if (cur == identsym)
    //         printf(" '%s'", characters[idents_index++]);
    //     
    //     printf("\n");
    // }

    // initializig first scope
    tables_len++;

    // declaring main
    declare_symbol((symbol) {
        .kind = 3,
        .name = "main",
    });
    Program();
    // halt
    emit(9, 0, 3);

}

//main using jie lins driver file as inspiration
int main(int argc, char *argv[])
{
	FILE *ifp;
	char buffer;
	// read in the input file
	if (argc < 2)
		error_no_line("please include the file name :(");
    
    token_types = calloc(1000, sizeof(token_type));
    PAS = calloc(MAX_BYTECODE_SIZE, sizeof(int));
    input = calloc(MAX_SOURCE_FILE_SIZE, sizeof(char));
    
    filename = argv[1];
	ifp = fopen(argv[1], "r");

	input_len = 0;
	while (fscanf(ifp, "%c", &buffer) != EOF)
		input[input_len++] = buffer;
    
	input[input_len] = '\0';
	fclose(ifp);

    printf("Source Program:\n%s\n\n", input);
	
	lex_analyze(input);
	parse();

    printf("No errors, program is syntactically correct\n\n");

    printf("Assembly Output:\n\n");
    printf("Addr \t| Name \t| L \t| M\n\n");

    FILE* ofp = fopen("elf.txt", "w");

    for (int i = 0; i < gen_index; i += 3) {
        isa ir = (isa) { .OP = PAS[i + 0], .L = PAS[i + 1], .M = PAS[i + 2] };

        printf("%d: \t| ", i);
        header_print(ir);
        fprintf(ofp, "%d %d %d\n", ir.OP, ir.L, ir.M);
    }

    printf("\n\n");

    fclose(ofp);
	return 0;
}
