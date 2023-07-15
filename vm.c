// Isaac Tam & Ilham Mukati
// COP3402
// Prof Montagne

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//holds our op, l and the m
typedef struct isa{
    int OP;
    int L;
    int M;
}isa;

//#define ARRAY_SIZE 500

//implicit declaration of functions
void stack_out(int PC, int BP, int SP, int GP, int *PAS, int *cols);
void header_print(int PC, isa IR);
int base(int BP, int L);

//declaring neccesary variables
isa IR;
int BP, SP, PC, GP;
int PAS[500] = {0};


int main(int argc, char* argv[]){

    //read in data from infile and putinto array
    FILE* infile = fopen(argv[1], "r");
    int i = 0;
    while(fscanf(infile,"%d", &PAS[i]) != EOF){ 
      ++i;
    }

    //creating our BP SP GP , and PC variables
    BP = 499;
    PC = 0;
    SP = BP + 1;
    GP = BP;
    int EOP = 1;
    int cols[500];
 
    //prints the initial values
    printf("                 PC  BP  SP  stack\n");
    printf("Initial values:  %d  %d  %d\n\n",PC, BP, SP);

    while(EOP){
        //fetch cycle
        IR.OP = PAS[PC];
        IR.L = PAS[PC + 1];
        IR.M = PAS[PC + 2];
        PC = PC + 3;
    
        //execution cycle
        switch(IR.OP){

            //LIT
            case 1:
                SP = SP - 1;
                PAS[SP] = IR.M;
                break;
            //OPR
            case 2:
                switch(IR.M){
                    //RTN
                    case 0:
                        SP = BP + 1;
                        BP = PAS[SP - 2];
                        PC = PAS[SP - 3];
                        cols[SP+1] = 0;
                        break;
                    //ADD
                    case 1:
                        PAS[SP + 1] = PAS[SP + 1] + PAS[SP];
                        SP = SP + 1;
                        break;
                    //SUB
                    case 2: 
                        PAS[SP + 1] = PAS[SP + 1] - PAS[SP];
                        SP = SP + 1;
                        break;
                    //MUL
                    case 3: 
                        PAS[SP + 1] = PAS[SP + 1] * PAS[SP];
                        SP = SP + 1;
                        break;
                    //DIV
                    case 4:
                        PAS[SP + 1] = PAS[SP + 1] / PAS[SP];
                        SP = SP + 1;
                        break;
                    //EQL
                    case 5: 
                        PAS[SP + 1] = PAS[SP + 1] == PAS[SP];
                        SP = SP + 1;
                        break;
                    //NEQ
                    case 6:
                        PAS[SP + 1] = PAS[SP + 1] != PAS[SP];
                        SP = SP + 1;
                        break;
                    //LSS
                    case 7:
                        PAS[SP + 1] = PAS[SP + 1] < PAS[SP];
                        SP = SP + 1;
                        break;
                    //LEQ
                    case 8:
                        PAS[SP + 1] = PAS[SP + 1] <= PAS[SP];
                        SP = SP + 1;
                        break;
                    //GTR
                    case 9:
                        PAS[SP + 1] = PAS[SP + 1] > PAS[SP];
                        SP = SP + 1;
                        break;
                    //GEQ
                    case 10:
                        PAS[SP + 1] = PAS[SP + 1] >= PAS[SP];
                        SP = SP + 1;
                        break;
                    default:
                        break;
                }
               break;
            //LOD
            case 3:
                SP = SP - 1;
                PAS[SP] = PAS[base(BP,IR.L) - IR.M];
                break;
            //STO
            case 4:
                PAS[base(BP,IR.L) - IR.M] = PAS[SP];
                SP = SP + 1;
                break;
            //CAL
            case 5:
                BP = SP - 1;
                PAS[SP - 1] = base(BP,IR.L);
                PAS[SP - 2] = BP; 
                PAS[SP - 3] = PC;
                PC = IR.M;
                cols[SP - 1] = 1; 
                break;
            //INC
            case 6:
                SP = SP - IR.M;
                break;
            //JMP
            case 7: 
                PC = IR.M;
                break;
            //JPC
            case 8:
                if(PAS[SP] == 0){
                    PC = IR.M;
                }
                SP = SP + 1;
                break;
            case 9:
                switch(IR.M){
                    //write
                    case 1:
                        printf("\nOutput result is: %d\n", PAS[SP]);
                        SP = SP + 1;
                        break;
                    //read
                    case 2:
                        SP = SP - 1;
                        printf("\nPlease Enter an Integer: ");
                        scanf("%d", &PAS[SP]);
                        printf("\n");
                        break;
                    //EOP
                    case 3: 
                        EOP = 0;
                            break;
                    default:
                        break;
                }
                break;

            default:
                break;
        } 
        //prints the instructions
        header_print(PC, IR);
        
        //prints the stack 
        stack_out(PC,BP,SP,GP,PAS,cols);

    }

    return 0;
}

int base(int BP, int L){

    int arb = BP; // arb = activation record base
    while ( L > 0)     //find base L levels down
    {
    arb = PAS[arb];
    L--;
    }
    return arb;
}

void header_print(int PC, isa IR){
    switch (IR.OP)
    {
    case 1:
        printf("%s\t%d\t%d\t", "LIT", IR.L, IR.M);
        break;
    case 2:
        switch (IR.M)
        {
        case 0:
            printf("%s\t%d\t%d\t", "RTN", IR.L, IR.M);
            break;
        case 1:
            printf("%s\t%d\t%d\t", "ADD", IR.L, IR.M);
            break;
        case 2:
            printf("%s\t%d\t%d\t", "SUB", IR.L, IR.M);
            break;
        case 3:
            printf("%s\t%d\t%d\t", "MUL", IR.L, IR.M);
            break;
        case 4:
            printf("%s\t%d\t%d\t", "DIV", IR.L, IR.M);
            break;
        case 5:
            printf("%s\t%d\t%d\t", "EQL", IR.L, IR.M);
            break;
        case 6:
            printf("%s\t%d\t%d\t", "NEQ", IR.L, IR.M);
            break;
        case 7:
            printf("%s\t%d\t%d\t", "LSS", IR.L, IR.M);
            break;
        case 8:
            printf("%s\t%d\t%d\t", "LEQ", IR.L, IR.M);
            break;
        case 9:
            printf("%s\t%d\t%d\t", "GTR", IR.L, IR.M);
            break;
        case 10:
            printf("%s\t%d\t%d\t", "GEQ", IR.L, IR.M);
            break;
        default:
            printf("%s\t%d\t%d\t", "ERR", IR.L, IR.M);
            break;
        }
        break;
    case 3:
        printf("%s\t%d\t%d\t", "LOD", IR.L, IR.M);
        break;
    case 4:
        printf("%s\t%d\t%d\t", "STO", IR.L, IR.M);
        break;
    case 5:
        printf("%s\t%d\t%d\t", "CAL", IR.L, IR.M);
        break;
    case 6:
        printf("%s\t%d\t%d\t", "INC", IR.L, IR.M);
        break;
    case 7:
        printf("%s\t%d\t%d\t", "JMP", IR.L, IR.M);
        break;
    case 8:
        printf("%s\t%d\t%d\t", "JPC", IR.L, IR.M);
        break;
    case 9:
        switch (IR.M)
        {
        case 1:
            printf("%s\t%d\t%d\t", "SOU", IR.L, IR.M);
            break;
        case 2:
            printf("%s\t%d\t%d\t", "SIN", IR.L, IR.M);
            break;
        case 3:
            printf("%s\t%d\t%d\t", "EOP", IR.L, IR.M);
            break;
        default:
            printf("%s\t%d\t%d\t", "ERR", IR.L, IR.M);
            break;
        }
        break;

    default:
        printf("%s\t%d\t%d\t", "ERR", IR.L, IR.M);
        break;
    }
}

void stack_out(int PC, int BP, int SP, int GP, int *PAS, int *cols){
    printf("%d\t%d\t%d\t", PC, BP, SP);
    for (int i = GP; i >= SP; --i){
        if (cols[i] == 1)
        printf("| %d ", PAS[i]);
        else
        printf("%d ", PAS[i]);
        }
        printf("\n");
    }
