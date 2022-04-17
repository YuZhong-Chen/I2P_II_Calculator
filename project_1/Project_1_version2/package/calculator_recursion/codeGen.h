#ifndef __CODEGEN__
#define __CODEGEN__

#include "parser.h"

typedef struct
{
    int isTemp;
    int isConst;
    int isUse;
    int val;
} Memory;

extern Memory memory[80];

extern int getSpace(int Temp, int Const, int Value);

extern void freeSpace(int address);

// Evaluate the syntax tree
extern int evaluateTree(BTNode *root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);

#endif // __CODEGEN__
