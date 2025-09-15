/*
 * Project 1
 * EECS 370 LC-2K Instruction-level simulator
 *
 * Make sure to NOT modify printState or any of the associated functions
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//DO NOT CHANGE THE FOLLOWING DEFINITIONS 

// Machine Definitions
#define MEMORYSIZE 65536 /* maximum number of words in memory (maximum number of lines in a given file)*/
#define NUMREGS 8 /*total number of machine registers [0,7]*/

// File Definitions
#define MAXLINELENGTH 1000 /* MAXLINELENGTH is the max number of characters we read */

// Instruction opcodes
#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5
#define HALT 6
#define NOOP 7

typedef struct 
stateStruct {
    int pc;
    int mem[MEMORYSIZE];
    int reg[NUMREGS];
    int numMemory;
    int numInstructionsExecuted;
} stateType;

void printState(stateType *);

void printStats(stateType *);

static inline int convertNum(int32_t);


int 
main(int argc, char **argv)
{
    char line[MAXLINELENGTH];
    stateType state = {0};
    FILE *filePtr;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s , please ensure you are providing the correct path", argv[1]);
        perror("fopen");
        exit(2);
    }

    /* read the entire machine-code file into memory */
    for (state.numMemory=0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++) {
		    if (state.numMemory >= MEMORYSIZE) {
			      fprintf(stderr, "exceeded memory size\n");
			      exit(2);
		    }
		    if (sscanf(line, "%x", state.mem+state.numMemory) != 1) {
			      fprintf(stderr, "error in reading address %d\n", state.numMemory);
			      exit(2);
		    }
            printf("mem[ %d ] 0x%08X\n", state.numMemory, state.mem[state.numMemory]);
    }

    fclose(filePtr);

    // Main execution loop
    while (1) {
        printState(&state);
        
        // Fetch instruction
        int instruction = state.mem[state.pc];
        
        // Increment PC
        state.pc++;
        
        // Increment instruction count
        state.numInstructionsExecuted++;
        
        // Decode instruction
        int opcode = (instruction >> 22) & 0x7;
        int regA = (instruction >> 19) & 0x7;
        int regB = (instruction >> 16) & 0x7;
        int destReg = instruction & 0x7;
        int offset = convertNum(instruction & 0xFFFF);
        
        // Execute instruction
        switch (opcode) {
            case ADD:
                state.reg[destReg] = state.reg[regA] + state.reg[regB];
                break;
                
            case NOR:
                state.reg[destReg] = ~(state.reg[regA] | state.reg[regB]);
                break;
                
            case LW:
                {
                    int address = state.reg[regA] + offset;
                    if (address < 0 || address >= MEMORYSIZE) {
                        printf("error: memory access out of bounds at address %d\n", address);
                        exit(1);
                    }
                    state.reg[regB] = state.mem[address];
                }
                break;
                
            case SW:
                {
                    int address = state.reg[regA] + offset;
                    if (address < 0 || address >= MEMORYSIZE) {
                        printf("error: memory access out of bounds at address %d\n", address);
                        exit(1);
                    }
                    state.mem[address] = state.reg[regB];
                }
                break;
                
            case BEQ:
                if (state.reg[regA] == state.reg[regB]) {
                    state.pc = state.pc + offset;
                }
                break;
                
            case JALR:
                state.reg[regB] = state.pc;
                state.pc = state.reg[regA];
                break;
                
            case HALT:
                printStats(&state);
                printState(&state);
                exit(0);
                break;
                
            case NOOP:
                // Do nothing
                break;
                
            default:
                printf("error: unrecognized opcode %d\n", opcode);
                exit(1);
        }
    }

    return(0);
}

/*
* DO NOT MODIFY ANY OF THE CODE BELOW. 
*/

void printState(stateType *statePtr) {
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] 0x%08X\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
	  for (i=0; i<NUMREGS; i++) {
	      printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
	  }
    printf("end state\n");
}

// convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int num) 
{
    return num - ( (num & (1<<15)) ? 1<<16 : 0 );
}

/*
 * print end of run statistics like in the spec. **This is not required**,
 * but is helpful in debugging.
 * This should be called once a halt is reached.
 * DO NOT delete this function, or else it won't compile.
 * DO NOT print "@@@" or "end state" in this function
 */
void printStats(stateType *statePtr)
{
    printf("machine halted\n");
    printf("total of %d instructions executed\n", statePtr->numInstructionsExecuted);
    printf("final state of machine:\n");
}

/*
* Write any helper functions that you wish down here. 
*/