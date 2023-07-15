//Isaac Tam & Ilham Mukati
// 2/14/23

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

typedef enum {  
skipsym = 1, identsym, numbersym, plussym, minussym, 
    multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym, 
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym, 
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,  
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym, 
    readsym , elsesym } token_type; 

int output[500];
int chrs = 0;
int cnt = 0;
int cnt1 = 0;
int cnt2 = 0;
int numbers = 0;
char characters[30][12];
int arr[60];
#define MAX 500


//prints key words and identifiers/tokens
void token_printer(char* c){
    if(strcmp(c, "main") == 0 || strcmp(c, "null") == 0){
        printf("Lex Analyzer Error: identifiers cannot be named 'null' or 'main' :(\n");
    }
    else if(strcmp(c,"const") == 0){
        printf("%-11s%-11d", "const", constsym);
        output[cnt] = constsym;
        cnt++;
    }
    else if(strcmp(c,"var") == 0){
        printf("%-11s%-11d", "var", varsym);
        output[cnt] = varsym;
        cnt++;
    }
    else if(strcmp(c,"odd") == 0){
        printf("%-11s%-11d", "odd", oddsym);
        output[cnt] = oddsym;
        cnt++;
    }
    else if(strcmp(c,"procedure") == 0){
        printf("%-11s%-11d", "procedure", procsym);
        output[cnt] = procsym;
        cnt++;
    }
    else if(strcmp(c,"begin") == 0){
        printf("%-11s%-11d", "begin", beginsym);
        output[cnt] = beginsym;
        cnt++;
    }
    else if(strcmp(c,"end") == 0){
        printf("%-11s%-11d", "end", endsym);
        output[cnt] = endsym;
        cnt++;
    }
    else if(strcmp(c,"call") == 0){
        printf("%-11s%-11d", "call", callsym);
        output[cnt] = callsym;
        cnt++;
    }
    else if(strcmp(c,"if") == 0){
        printf("%-11s%-11d", "if", ifsym);
        output[cnt] = ifsym;
        cnt++;
    }
    else if(strcmp(c,"then") == 0){
        printf("%-11s%-11d", "then", thensym);
        output[cnt] = thensym;
        cnt++;
    }
    else if(strcmp(c,"else") == 0){
        printf("%-11s%-11d", "else", elsesym);
        output[cnt] = elsesym;
        cnt++;
    }
    else if(strcmp(c,"while") == 0){
        printf("%-11s%-11d", "while", whilesym);
        output[cnt] = whilesym;
        cnt++;
    }
    else if(strcmp(c,"do") == 0){
        printf("%-11s%-11d", "do", dosym);
        output[cnt] = dosym;
        cnt++;
    }
    else if(strcmp(c,"read") == 0){
        printf("%-11s%-11d", "read", readsym);
        output[cnt] = readsym;
        cnt++;
    }
    else if(strcmp(c,"write") == 0){
        printf("%-11s%-11d", "write", writesym);
        output[cnt] = writesym;
        cnt++;
    } // if not a reserved word it is an identifier
    else{
        printf("%-11s%-11d",c,identsym );
        output[cnt] = identsym;
        cnt++;
        strcpy(characters[cnt2],c);
        cnt2+=1;
    }
}

// prints symbols and their tokens
void print_symb(char* chararr, int index){
    if(chararr[index] == '.'){
        printf("%-11c%-11d\n",chararr[index], periodsym);
        output[cnt] = periodsym;
        cnt++;
        index++;
    }
    // checks for becomessym
    else if(chararr[index] == ':'){
        index++;
        if(chararr[index] == '='){
            printf("%c%-10c%-11d\n",chararr[index-1], chararr[index], becomessym ); 
            output[cnt] = becomessym;
            cnt++;    
        }
        else{
            printf("Lex Analyzer Error: invalid symbol :(");
        }    
    }
    else if(chararr[index] =='-'){
        printf("%-11c%-11d\n",chararr[index], minussym);
        output[cnt] = minussym;
        cnt++;
        index++;
    }
    else if(chararr[index] ==';'){
        printf("%-11c%-11d\n",chararr[index], semicolonsym);
        output[cnt] = semicolonsym;
        cnt++;
        index++;
    }
    else if(chararr[index] ==','){
        printf("%-11c%-11d\n",chararr[index], commasym);
        output[cnt] = commasym;
        cnt++;
        index++;
    }
    else if(chararr[index]=='>'){
        index++;
        if(chararr[index] == '='){
            printf("%c%-11c%-11d\n",chararr[index-1], chararr[index], geqsym);
            output[cnt] = geqsym;
            cnt++;
            index++;
        }
        else{
            if(chararr[index - 1] == '>' && chararr[index-2] != '<'){
                printf("%-11c%-11d\n",chararr[index - 1], gtrsym);
                output[cnt] = gtrsym;
                cnt++;
            }
        }
    }
    else if(chararr[index] == '='){
        index--;
        if(chararr[index] == ':'){
            return;
        }
        else{
            printf("%-11c%-11d\n", chararr[index], eqsym);
            output[cnt] = eqsym;
            cnt++;
        }
    }
    // checks for <, <=, and <>
    else if(chararr[index] =='<'){
        index++;
        if(chararr[index] == '='){
            printf("%c%-11c%-11d\n",chararr[index-1],chararr[index], leqsym);
            output[cnt] = leqsym;
            cnt++;
            index++;
        }
        else if(chararr[index] == '>'){
            printf("%c%-11c%-11d\n",chararr[index-1], chararr[index], neqsym);
            output[cnt] = neqsym;
            cnt++;
            index++;
        }
        else{
            printf("%-11c%-11d\n",chararr[index], lessym);
            output[cnt] = lessym;
            cnt++;
        }
    }
    else if(chararr[index]=='+'){
        printf("%-11c%-11d\n",chararr[index], plussym);
        output[cnt] = plussym;
        cnt++;
        index++;
    }
    else if(chararr[index]=='/'){
        printf("%-11c%-11d\n",chararr[index], slashsym);
        output[cnt] = slashsym;
        cnt++;
        index++;
    }
    else if(chararr[index]=='*'){
        printf("%-11c%-11d\n",chararr[index], multsym);
        output[cnt] = multsym;
        cnt++;
        index++;
    }
    else if(chararr[index]==')'){
        printf("%-11c%-11d\n",chararr[index], rparentsym);
        output[cnt] = rparentsym;
        cnt++;
        index++;
    }
    else if(chararr[index]=='('){
        printf("%-11c%-11d\n",chararr[index], lparentsym);
        output[cnt] = lparentsym;
        cnt++;
        index++;
    }
    else{
        printf("Lex Analyzer Error: Invalid symbol :(\n");
    }
        
    

}

int main(int argc, char* argv[]){ 
    FILE* infile = fopen(argv[1], "r");
    char c = fgetc(infile);
    printf ("Source Program:\n");
    while (c != EOF)
    {
        printf ("%c", c);
        c = fgetc(infile);
    }
    printf ("\n\n");
    fclose(infile);
    FILE* infile1 = fopen(argv[1], "r");
     char chararr[500];
     char buffarr[12];
     int i = 0;
     int l =0;
     
     //reads input file
     char buff = fgetc(infile1);
     while(buff != EOF){
        chararr[i++] = buff;
        buff = fgetc(infile1);
     }
     chararr[i] = '\0';
     i = 0;
     printf("Lexeme Table:\n\n");
     printf("%-11s%-11s\n", "Lexeme", "token type");
    int j = 0;
     while(chararr[i] != '\0'){
     //starting identifies the lexeme and respective tokens
     
     //first char being stored is letter
        if(isalpha(chararr[i])){
            j = 0;
            while (isalpha(chararr[i]) || isdigit(chararr[i])){
                buffarr[j++] = chararr[i++];
                if(j == 11){
                    break;
                }

            }
            // checks for identifer len over 11 throws error
            if(isalpha(chararr[i]) || isdigit(chararr[i])){
                printf("Lex Analyzer Error: maximum identifier length is 11 :(\n");
                while(isalpha(chararr[i]) || isdigit(chararr[i])){
                    i++;

                }
            
            }
            else{
                buffarr[j] = '\0';
                token_printer(buffarr);
                printf("\n");

            }
        }
        // first character being stored in buffarr is a digit
        else if(isdigit(chararr[i])){
            j = 0;
            while(isdigit(chararr[i])){
                buffarr[j++] = chararr[i++];
                if(j==5){
                    break;
                }
            }
            if(isalpha(chararr[i])){
                printf("Lex Analyzer Error: identifiers cannot begin with digits :(\n");
                while(isalpha(chararr[i]) || isdigit(chararr[i])){
                    i++;
                }
            }
            else if(isdigit(chararr[i])){
                printf("Lex Analyzer Error: maximum number length is 5 :(\n");
                while(isalpha(chararr[i]) || isdigit(chararr[i])){
                    i++;
                }
            }
            else{
                buffarr[j] = '\0';
                printf("%-11s%-11d",buffarr, numbersym);
                arr[cnt1] = atoi(buffarr);
                output[cnt] = numbersym;
                cnt++;
                cnt1++;
                printf("\n");
                
            }
        }
        //if a blank space or cntrl character is hit
        else if(chararr[i] == ' ' || iscntrl(chararr[i])){
            i++;
        }
        
        //if comment is hit
        else if(chararr[i] == '/' && chararr[i+1] =='*'){
            while(chararr[i] != '*' && chararr[i+1] != '/'){
                i++;
                if(chararr[i] == EOF){
                  printf("Lex error: input ended during comment :(\n");
                }
            }
        }
        //if a symbol is hit
        else{
            print_symb(chararr, i);
            ++i;
        }
     }
   
      
    // prints lexeme list
     printf("\nLexeme List:\n");
     int a = 0;
     int b = 0;
     int d = 0;
     while(d < cnt){
       printf("%d " ,output[d]);
       if(output[d]==3){
           printf("%d " ,arr[a]);
           a++;
       }
       else if(output[d]==2){
           printf("%s " ,characters[b]);
           b++;
       }
        d++;
     }
     printf("\n");
     

   
}


