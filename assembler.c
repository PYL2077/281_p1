/**
 * Project 1
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000
#define MAXLABELS 1000

// Structure to store label information
typedef struct {
    char name[MAXLINELENGTH];
    int address;
} Label;

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static inline int isNumber(char *);
static inline void printHexToFile(FILE *, int);

// Global arrays for labels
Label labels[MAXLABELS];
int labelCount = 0;

// Function prototypes
int findLabel(char *labelName);
void addLabel(char *labelName, int address);
int getOpcode(char *opcode);
int assembleLine(char *label, char *opcode, char *arg0, char *arg1, char *arg2, int address);

int main(int argc, char **argv)
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    // Check for blank lines in the middle of the code.
    checkForBlankLinesInCode(inFilePtr);

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    // First pass: collect all labels
    rewind(inFilePtr);
    int address = 0;
    while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        // If there's a label, add it to our label table
        if (strlen(label) > 0) {
            // Check if label already exists
            if (findLabel(label) != -1) {
                printf("error: duplicate definition of label %s\n", label);
                exit(1);
            }
            // Check label length
            if (strlen(label) > 6) {
                printf("error: label %s is longer than 6 characters\n", label);
                exit(1);
            }
            // Check if label starts with a letter
            if (!((label[0] >= 'a' && label[0] <= 'z') || (label[0] >= 'A' && label[0] <= 'Z'))) {
                printf("error: label %s must start with a letter\n", label);
                exit(1);
            }
            addLabel(label, address);
        }
        
        // Check for valid opcode
        if (strcmp(opcode, "add") && strcmp(opcode, "nor") && strcmp(opcode, "lw") && 
            strcmp(opcode, "sw") && strcmp(opcode, "beq") && strcmp(opcode, "jalr") && 
            strcmp(opcode, "halt") && strcmp(opcode, "noop") && strcmp(opcode, ".fill")) {
            printf("error: unrecognized opcode %s\n", opcode);
            exit(1);
        }
        
        address++;
    }

    // Second pass: generate machine code
    rewind(inFilePtr);
    address = 0;
    while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        int machineCode = assembleLine(label, opcode, arg0, arg1, arg2, address);
        printHexToFile(outFilePtr, machineCode);
        address++;
    }

    fclose(inFilePtr);
    fclose(outFilePtr);
    exit(0);
}

// Find label in label table, return address or -1 if not found
int findLabel(char *labelName) {
    for (int i = 0; i < labelCount; i++) {
        if (!strcmp(labels[i].name, labelName)) {
            return labels[i].address;
        }
    }
    return -1;
}

// Add label to label table
void addLabel(char *labelName, int address) {
    if (labelCount >= MAXLABELS) {
        printf("error: too many labels\n");
        exit(1);
    }
    strcpy(labels[labelCount].name, labelName);
    labels[labelCount].address = address;
    labelCount++;
}

// Get opcode number
int getOpcode(char *opcode) {
    if (!strcmp(opcode, "add")) return 0;
    if (!strcmp(opcode, "nor")) return 1;
    if (!strcmp(opcode, "lw")) return 2;
    if (!strcmp(opcode, "sw")) return 3;
    if (!strcmp(opcode, "beq")) return 4;
    if (!strcmp(opcode, "jalr")) return 5;
    if (!strcmp(opcode, "halt")) return 6;
    if (!strcmp(opcode, "noop")) return 7;
    return -1;
}

// Assemble a single line
int assembleLine(char *label, char *opcode, char *arg0, char *arg1, char *arg2, int address) {
    int machineCode = 0;
    int opcodeNum = getOpcode(opcode);
    
    // Handle .fill directive
    if (!strcmp(opcode, ".fill")) {
        if (isNumber(arg0)) {
            int num;
            sscanf(arg0, "%d", &num);
            return num;
        } else {
            // Symbolic address for .fill
            int labelAddr = findLabel(arg0);
            if (labelAddr == -1) {
                printf("error: undefined label %s\n", arg0);
                exit(1);
            }
            return labelAddr;
        }
    }
    
    // Set opcode bits (bits 24-22)
    machineCode |= (opcodeNum << 22);
    
    // Handle different instruction types
    if (!strcmp(opcode, "add") || !strcmp(opcode, "nor")) {
        // R-type instruction
        int regA, regB, destReg;
        
        if (!isNumber(arg0) || !isNumber(arg1) || !isNumber(arg2)) {
            printf("error: non-integer register arguments\n");
            exit(1);
        }
        
        sscanf(arg0, "%d", &regA);
        sscanf(arg1, "%d", &regB);
        sscanf(arg2, "%d", &destReg);
        
        if (regA < 0 || regA > 7 || regB < 0 || regB > 7 || destReg < 0 || destReg > 7) {
            printf("error: registers outside range [0, 7]\n");
            exit(1);
        }
        
        machineCode |= (regA << 19);
        machineCode |= (regB << 16);
        machineCode |= destReg;
        
    } else if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw") || !strcmp(opcode, "beq")) {
        // I-type instruction
        int regA, regB, offset;
        
        if (!isNumber(arg0) || !isNumber(arg1)) {
            printf("error: non-integer register arguments\n");
            exit(1);
        }
        
        sscanf(arg0, "%d", &regA);
        sscanf(arg1, "%d", &regB);
        
        if (regA < 0 || regA > 7 || regB < 0 || regB > 7) {
            printf("error: registers outside range [0, 7]\n");
            exit(1);
        }
        
        // Handle offset field
        if (isNumber(arg2)) {
            sscanf(arg2, "%d", &offset);
        } else {
            // Symbolic address
            int labelAddr = findLabel(arg2);
            if (labelAddr == -1) {
                printf("error: undefined label %s\n", arg2);
                exit(1);
            }
            
            if (!strcmp(opcode, "beq")) {
                // For beq, offset is relative to PC+1
                offset = labelAddr - (address + 1);
            } else {
                // For lw/sw, offset is absolute address
                offset = labelAddr;
            }
        }
        
        // Check if offset fits in 16 bits (-32768 to 32767)
        if (offset < -32768 || offset > 32767) {
            printf("error: offsetField doesn't fit in 16 bits\n");
            exit(1);
        }
        
        machineCode |= (regA << 19);
        machineCode |= (regB << 16);
        // if(offset!= (offset & 0xFFFF))
        // {
        //     puts("offset problem");
        //     printf("%d\n",offset);
        //     exit(1);
        // }
        machineCode |= (offset & 0xFFFF);  // Mask to 16 bits
        
    } else if (!strcmp(opcode, "jalr")) {
        // J-type instruction
        int regA, regB;
        
        if (!isNumber(arg0) || !isNumber(arg1)) {
            printf("error: non-integer register arguments\n");
            exit(1);
        }
        
        sscanf(arg0, "%d", &regA);
        sscanf(arg1, "%d", &regB);
        
        if (regA < 0 || regA > 7 || regB < 0 || regB > 7) {
            printf("error: registers outside range [0, 7]\n");
            exit(1);
        }
        
        machineCode |= (regA << 19);
        machineCode |= (regB << 16);
        
    } else if (!strcmp(opcode, "halt") || !strcmp(opcode, "noop")) {
        // O-type instruction - opcode already set, nothing else needed
    }
    
    return machineCode;
}

// Returns non-zero if the line contains only whitespace.
static int lineIsBlank(char *line) {
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for(int line_idx=0; line_idx < strlen(line); ++line_idx) {
        int line_char_is_whitespace = 0;
        for(int whitespace_idx = 0; whitespace_idx < 4; ++ whitespace_idx) {
            if(line[line_idx] == whitespace[whitespace_idx]) {
                line_char_is_whitespace = 1;
                break;
            }
        }
        if(!line_char_is_whitespace) {
            nonempty_line = 1;
            break;
        }
    }
    return !nonempty_line;
}

// Exits 2 if file contains an empty line anywhere other than at the end of the file.
// Note calling this function rewinds inFilePtr.
static void checkForBlankLinesInCode(FILE *inFilePtr) {
    char line[MAXLINELENGTH];
    int blank_line_encountered = 0;
    int address_of_blank_line = 0;
    rewind(inFilePtr);

    for(int address = 0; fgets(line, MAXLINELENGTH, inFilePtr) != NULL; ++address) {
        // Check for line too long
        if (strlen(line) >= MAXLINELENGTH-1) {
            printf("error: line too long\n");
            exit(1);
        }

        // Check for blank line.
        if(lineIsBlank(line)) {
            if(!blank_line_encountered) {
                blank_line_encountered = 1;
                address_of_blank_line = address;
            }
        } else {
            if(blank_line_encountered) {
                printf("Invalid Assembly: Empty line at address %d\n", address_of_blank_line);
                exit(2);
            }
        }
    }
    rewind(inFilePtr);
}


/*
* NOTE: The code defined below is not to be modifed as it is implimented correctly.
*/

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH-1) {
	printf("error: line too long\n");
	exit(1);
    }

    // Ignore blank lines at the end of the file.
    if(lineIsBlank(line)) {
        return 0;
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);

    return(1);
}

static inline int
isNumber(char *string)
{
    int num;
    char c;
    return((sscanf(string, "%d%c",&num, &c)) == 1);
}


// Prints a machine code word in the proper hex format to the file
static inline void 
printHexToFile(FILE *outFilePtr, int word) {
    fprintf(outFilePtr, "0x%08X\n", word);
}