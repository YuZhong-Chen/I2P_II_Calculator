#ifndef __PARSER__
#define __PARSER__

#include "lex.h"
#define TBLSIZE 64 // the size of the table that store the variable

// NOTE : remember to change the PRINTER to 0
// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 0

// Call this macro to print error message and exit the program
// This will also print where you called it in your program
#define error(errorNum)                                                       \
    {                                                                         \
        if (PRINTERR)                                                         \
            fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
        err(errorNum);                                                        \
    }

// Error types
typedef enum
{
    UNDEFINED,
    MISPAREN,
    NOTNUMID,
    NOTFOUND,
    RUNOUT,
    NOTLVAL,
    DIVZERO,
    SYNTAXERR,
    UNDEFINEDVARIABLE
} ErrorType;

// Structure of the symbol table
typedef struct
{
    int val;
    char name[MAXLEN];
    int address;
} Symbol;

// Structure of a tree node
typedef struct _Node
{
    TokenSet data;
    int val;
    char lexeme[MAXLEN];
    int address;
    struct _Node *left;
    struct _Node *right;
} BTNode;

typedef struct _memory
{
    int isConst;
    int isUse;
    int _isTemp;
    int val;
} Memory;

// the number of the variables
extern int sbcount;

// The symbol table
extern Symbol table[TBLSIZE];

extern Memory memory[TBLSIZE];

// Initialize the symbol table with builtin variables
extern void initTable(void);

// Initialize the memory table.
extern void initMemory(void);

// find a empty address to use
extern int getEmptyMemoryAddress(int isTemp);

// free a memory that don't need.
extern void freeMemoryAddress(int address);

// Get the value of a variable
extern int getval(char *str);

// Set the value of a variable
extern int setval(char *str, int val);

// Get the address of a variable
extern int getadd(char *str);

// Set the address of a variable
extern int setadd(char *str, int add);

// Make a new node according to token type and lexeme
extern BTNode *makeNode(TokenSet tok, const char *lexe);

// Free the syntax tree
extern void freeTree(BTNode *root);

extern BTNode *assign_expr(void);
extern BTNode *or_expr(void);
extern BTNode *or_expr_tail(BTNode *left);
extern BTNode *xor_expr(void);
extern BTNode *xor_expr_tail(BTNode *left);
extern BTNode *and_expr(void);
extern BTNode *and_expr_tail(BTNode *left);
extern BTNode *addsub_expr(void);
extern BTNode *addsub_expr_tail(BTNode *left);
extern BTNode *muldiv_expr(void);
extern BTNode *muldiv_expr_tail(BTNode *left);
extern BTNode *unary_expr(void);
extern BTNode *factor(void);

extern void statement(void);

// Print error message and exit the program
extern void err(ErrorType errorNum);

#endif // __PARSER__
