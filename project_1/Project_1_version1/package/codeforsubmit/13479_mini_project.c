#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE : in lex.h

#define MAXLEN 256

typedef enum
{
    UNKNOWN,
    END,
    ENDFILE,
    INT,
    ID,
    ADDSUB,
    MULDIV,
    ASSIGN,
    LPAREN,
    RPAREN,
    INCDEC,
    AND,
    OR,
    XOR
} TokenSet;

int match(TokenSet token);
void advance(void);
void goBack(char *c);
char *getLexeme(void);

// NOTE : lex.h

// NOTE : in parser.h

#define TBLSIZE 64
#define PRINTERR 0
#define error(errorNum)                                                       \
    {                                                                         \
        if (PRINTERR)                                                         \
            fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
        err(errorNum);                                                        \
    }

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

typedef struct
{
    int val;
    char name[MAXLEN];
    int address;
} Symbol;

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

int sbcount = 0;
Symbol table[TBLSIZE];
Memory memory[TBLSIZE];

void initTable(void);
void initMemory(void);
int getEmptyMemoryAddress(int isTemp);
void freeMemoryAddress(int address);
int getval(char *str);
int setval(char *str, int val);
int getadd(char *str);
int setadd(char *str, int add);
BTNode *makeNode(TokenSet tok, const char *lexe);
void freeTree(BTNode *root);
BTNode *assign_expr(void);
BTNode *or_expr(void);
BTNode *or_expr_tail(BTNode *left);
BTNode *xor_expr(void);
BTNode *xor_expr_tail(BTNode *left);
BTNode *and_expr(void);
BTNode *and_expr_tail(BTNode *left);
BTNode *addsub_expr(void);
BTNode *addsub_expr_tail(BTNode *left);
BTNode *muldiv_expr(void);
BTNode *muldiv_expr_tail(BTNode *left);
BTNode *unary_expr(void);
BTNode *factor(void);
void statement(void);
void err(ErrorType errorNum);

// NOTE : parser.h

// NOTE : in codeGen.h

int evaluateTree(BTNode *root);
int generateAssemblyCode(BTNode *root);
void printPrefix(BTNode *root);

// NOTE : codeGen.h

// NOTE : in codeGen.c

int evaluateTree(BTNode *root)
{
    // NOTE : retval -> return value
    // NOTE : lv     -> left value
    // NOTE : rv     -> right value
    int retval = 0, lv = 0, rv = 0;

    if (root != NULL)
    {
        switch (root->data)
        {
        case ID:
            retval = getval(root->lexeme);
            break;
        case INT:
            retval = atoi(root->lexeme);
            break;
        case ASSIGN:
            rv = evaluateTree(root->right);
            retval = setval(root->left->lexeme, rv);
            break;
        case ADDSUB:
        case MULDIV:
            lv = evaluateTree(root->left);
            rv = evaluateTree(root->right);
            if (strcmp(root->lexeme, "+") == 0)
            {
                retval = lv + rv;
            }
            else if (strcmp(root->lexeme, "-") == 0)
            {
                if (root->right == NULL)
                {
                    retval = -1 * lv;
                }
                else
                {
                    retval = lv - rv;
                }
            }
            else if (strcmp(root->lexeme, "*") == 0)
            {
                retval = lv * rv;
            }
            else if (strcmp(root->lexeme, "/") == 0)
            {
                if (rv == 0)
                    error(DIVZERO);
                retval = lv / rv;
            }
            break;
        case OR:
            lv = evaluateTree(root->left);
            rv = evaluateTree(root->right);
            retval = lv | rv;
            break;
        case AND:
            lv = evaluateTree(root->left);
            rv = evaluateTree(root->right);
            retval = lv & rv;
            break;
        case XOR:
            lv = evaluateTree(root->left);
            rv = evaluateTree(root->right);
            retval = lv ^ rv;
            break;
        case INCDEC:
            lv = evaluateTree(root->left);
            int left_id = 0;
            for (int i = 0; i < sbcount; i++)
            {
                if (!strcmp(root->left->lexeme, table[i].name))
                {
                    left_id = i;
                    break;
                }
            }

            if (strcmp(root->lexeme, "++") == 0)
            {
                table[left_id].val += 1;
                retval = lv + 1;
            }
            else if (strcmp(root->lexeme, "--") == 0)
            {
                table[left_id].val -= 1;
                retval = lv - 1;
            }
            break;
        default:
            retval = 0;
        }
    }
    return retval;
}

int generateAssemblyCode(BTNode *root)
{
    int retadd = 0, ladd = 0, radd = 0;

    if (root != NULL)
    {
        switch (root->data)
        {
        case ID:
            retadd = getadd(root->lexeme);
            break;
        case INT:
            retadd = getEmptyMemoryAddress(1);
            memory[retadd].isConst = 1;
            memory[retadd].val = atoi(root->lexeme);
            printf("MOV r6 %d\n", atoi(root->lexeme));
            printf("MOV [%d] r6\n", retadd * 4);
            break;
        case ASSIGN:
            radd = generateAssemblyCode(root->right);
            retadd = setadd(root->left->lexeme, radd);
            memory[retadd].isConst = 0;
            break;
        case ADDSUB:
        case MULDIV:
            ladd = generateAssemblyCode(root->left);
            radd = generateAssemblyCode(root->right);
            retadd = getEmptyMemoryAddress(1);
            memory[retadd].isConst = (memory[ladd].isConst && memory[radd].isConst);

            if (strcmp(root->lexeme, "+") == 0)
            {
                if (root->right != NULL)
                {
                    printf("MOV r3 [%d]\n", ladd * 4);
                    printf("MOV r5 [%d]\n", radd * 4);
                    printf("ADD r3 r5\n");
                    printf("MOV [%d] r3\n", retadd * 4);
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val + memory[radd].val;
                    }
                }
                else if (root->right == NULL)
                {
                    printf("MOV r3 [%d]\n", ladd * 4);
                    printf("MOV [%d] r3\n", retadd * 4);
                    if (memory[ladd].isConst)
                    {
                        memory[retadd].isConst = 1;
                        memory[retadd].val = memory[ladd].val;
                    }
                }
            }
            else if (strcmp(root->lexeme, "-") == 0)
            {
                if (root->right == NULL)
                {
                    printf("MOV r5 [%d]\n", ladd * 4);
                    printf("MOV r3 %d\n", 0);
                    printf("SUB r3 r5\n");
                    printf("MOV [%d] r3\n", retadd * 4);
                    if (memory[ladd].isConst)
                    {
                        memory[retadd].isConst = 1;
                        memory[retadd].val = -1 * memory[ladd].val;
                    }
                }
                else
                {
                    printf("MOV r3 [%d]\n", ladd * 4);
                    printf("MOV r5 [%d]\n", radd * 4);
                    printf("SUB r3 r5\n");
                    printf("MOV [%d] r3\n", retadd * 4);
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val - memory[radd].val;
                    }
                }
            }
            else if (strcmp(root->lexeme, "*") == 0)
            {
                printf("MOV r3 [%d]\n", ladd * 4);
                printf("MOV r5 [%d]\n", radd * 4);
                printf("MUL r3 r5\n");
                printf("MOV [%d] r3\n", retadd * 4);
                if (memory[retadd].isConst)
                {
                    memory[retadd].val = memory[ladd].val * memory[radd].val;
                }
            }
            else if (strcmp(root->lexeme, "/") == 0)
            {
                // NOTE : divzero
                if (memory[radd].isConst)
                {
                    if (memory[radd].val == 0)
                    {
                        error(DIVZERO);
                    }
                    else if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val / memory[radd].val;
                    }
                }
                printf("MOV r3 [%d]\n", ladd * 4);
                printf("MOV r5 [%d]\n", radd * 4);
                printf("DIV r3 r5\n");
                printf("MOV [%d] r3\n", retadd * 4);
            }

            break;
        case OR:
            ladd = generateAssemblyCode(root->left);
            radd = generateAssemblyCode(root->right);
            retadd = getEmptyMemoryAddress(1);
            memory[retadd].isConst = (memory[ladd].isConst && memory[radd].isConst);
            printf("MOV r3 [%d]\n", ladd * 4);
            printf("MOV r5 [%d]\n", radd * 4);
            printf("OR r3 r5\n");
            printf("MOV [%d] r3\n", retadd * 4);
            if (memory[retadd].isConst)
            {
                memory[retadd].val = memory[ladd].val | memory[radd].val;
            }

            break;
        case AND:
            ladd = generateAssemblyCode(root->left);
            radd = generateAssemblyCode(root->right);
            retadd = getEmptyMemoryAddress(1);
            memory[retadd].isConst = (memory[ladd].isConst && memory[radd].isConst);
            printf("MOV r3 [%d]\n", ladd * 4);
            printf("MOV r5 [%d]\n", radd * 4);
            printf("AND r3 r5\n");
            printf("MOV [%d] r3\n", retadd * 4);
            if (memory[retadd].isConst)
            {
                memory[retadd].val = memory[ladd].val & memory[radd].val;
            }

            break;
        case XOR:
            ladd = generateAssemblyCode(root->left);
            radd = generateAssemblyCode(root->right);
            retadd = getEmptyMemoryAddress(1);
            memory[retadd].isConst = (memory[ladd].isConst && memory[radd].isConst);
            printf("MOV r3 [%d]\n", ladd * 4);
            printf("MOV r5 [%d]\n", radd * 4);
            printf("XOR r3 r5\n");
            printf("MOV [%d] r3\n", retadd * 4);
            if (memory[retadd].isConst)
            {
                memory[retadd].val = memory[ladd].val ^ memory[radd].val;
            }

            break;
        case INCDEC:
            retadd = getadd(root->left->lexeme);
            if (strcmp(root->lexeme, "++") == 0)
            {
                printf("MOV r3 [%d]\n", retadd * 4);
                printf("MOV r5 1\n");
                printf("ADD r3 r5\n");
                printf("MOV [%d] r3\n", retadd * 4);
            }
            else if (strcmp(root->lexeme, "--") == 0)
            {
                printf("MOV r3 [%d]\n", retadd * 4);
                printf("MOV r5 1\n");
                printf("SUB r3 r5\n");
                printf("MOV [%d] r3\n", retadd * 4);
            }
            break;
        default:
            retadd = 0;
            error(SYNTAXERR);
        }
    }

    if (memory[ladd]._isTemp)
    {
        freeMemoryAddress(ladd);
    }
    if (memory[radd]._isTemp)
    {
        freeMemoryAddress(radd);
    }

    return retadd;
}

void printPrefix(BTNode *root)
{
    if (root != NULL)
    {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}

// NOTE : codeGen.c

// NOTE : in lex.c

TokenSet getToken(void);
TokenSet curToken = UNKNOWN;
char lexeme[MAXLEN];

int isVariable(char c);

TokenSet getToken(void)
{
    int i = 0;
    char c = '\0';

    // ignore the space and the tab.
    while ((c = fgetc(stdin)) == ' ' || c == '\t')
        ;

    if (isdigit(c))
    {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN)
        {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }

        if (isVariable(c))
            error(NOTNUMID);

        ungetc(c, stdin); // put back the character that not belone to here.
        lexeme[i] = '\0';
        return INT;
    }
    else if (c == '+' || c == '-')
    {
        char temp = fgetc(stdin);
        if ((c == '+' && temp == '+') || (c == '-' && temp == '-'))
        {
            lexeme[0] = lexeme[1] = c;
            lexeme[2] = '\0';
            return INCDEC;
        }
        else
        {
            ungetc(temp, stdin);
            lexeme[0] = c;
            lexeme[1] = '\0';
            return ADDSUB;
        }
    }
    else if (c == '*' || c == '/')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    }
    else if (c == '\n')
    {
        lexeme[0] = '\0';
        return END;
    }
    else if (c == '=')
    {
        strcpy(lexeme, "="); // equal to lexeme[0] = '='; ?
        return ASSIGN;
    }
    else if (c == '(')
    {
        strcpy(lexeme, "(");
        return LPAREN;
    }
    else if (c == ')')
    {
        strcpy(lexeme, ")");
        return RPAREN;
    }
    else if (c == '&')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return AND;
    }
    else if (c == '|')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return OR;
    }
    else if (c == '^')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return XOR;
    }
    else if (isVariable(c))
    {
        if (isdigit(c))
            error(SYNTAXERR);

        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isVariable(c) && i < MAXLEN)
        {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin); // put back the character that not belone to here.
        lexeme[i] = '\0';
        return ID;
    }
    else if (c == EOF)
    {
        return ENDFILE;
    }

    return UNKNOWN;
}

// move the action to the next.
void advance(void)
{
    curToken = getToken();
}

void goBack(char *c)
{
    if (curToken == UNKNOWN)
    {
        return;
    }
    else if (c[0] == '\0')
    {
        int size = strlen(lexeme) - 1;
        for (int i = size; i >= 0; i--)
        {
            ungetc(lexeme[i], stdin);
        }
    }
    else if (c[0] == '\n')
    {
        ungetc('\n', stdin);
    }
    else if (c[0] == '@')
    {
        ungetc(EOF, stdin);
    }
    else
    {
        int size = strlen(c) - 1;
        for (int i = size; i >= 0; i--)
        {
            ungetc(c[i], stdin);
        }
    }
    return;
}

// NOTE : check why UNKNOWN
int match(TokenSet token)
{
    if (curToken == UNKNOWN)
        advance();
    return token == curToken;
}

char *getLexeme(void)
{
    return lexeme;
}

int isVariable(char c)
{
    if (isalpha(c) || isdigit(c) || c == '_')
    {
        return 1;
    }

    return 0;
}

// NOTE : lex.c

// NOTE : in parser.c

void initTable(void)
{
    initMemory();

    strcpy(table[0].name, "x");
    // table[0].val = 0;
    table[0].address = getEmptyMemoryAddress(0);
    // memory[table[0].address].val = 0;

    // printf("MOV [%d] r0\n", table[0].address * 4);

    strcpy(table[1].name, "y");
    // table[1].val = 0;
    table[1].address = getEmptyMemoryAddress(0);
    // memory[table[1].address].val = 0;

    strcpy(table[2].name, "z");
    // table[2].val = 0;
    table[2].address = getEmptyMemoryAddress(0);
    // memory[table[2].address].val = 0;

    sbcount = 3;

    for (int i = 3; i < TBLSIZE; i++)
    {
        table[i].name[0] = '\0';
    }

    return;
}

void initMemory(void)
{
    for (int i = 0; i < TBLSIZE; i++)
    {
        memory[i].isUse = 0;
        memory[i]._isTemp = 1;
        memory[i].val = 0;
        memory[i].isConst = 0;
    }
    return;
}

int getEmptyMemoryAddress(int isTemp)
{
    for (int i = 0; i < TBLSIZE; i++)
    {
        if (!memory[i].isUse)
        {
            memory[i].isUse = 1;
            memory[i]._isTemp = isTemp;
            memory[i].val = 0;
            memory[i].isConst = 0;
            return i;
        }
    }

    error(RUNOUT);

    return -1;
}

void freeMemoryAddress(int address)
{
    memory[address].isUse = 0;
    memory[address]._isTemp = 1;
    memory[address].val = 0;
    memory[address].isConst = 0;

    return;
}

int getval(char *str)
{
    int i = 0;

    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;

    // NOTE : add the detect of "using nondeclared variable".

    if (sbcount >= TBLSIZE)
        error(RUNOUT);

    error(UNDEFINEDVARIABLE);

    strcpy(table[sbcount].name, str);
    table[sbcount].val = 0;
    sbcount++;
    return 0;
}

int setval(char *str, int val)
{
    int i = 0;

    for (i = 0; i < sbcount; i++)
    {
        if (strcmp(str, table[i].name) == 0)
        {
            table[i].val = val;
            return val;
        }
    }

    if (sbcount >= TBLSIZE)
        error(RUNOUT);

    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    sbcount++;
    return val;
}

int getadd(char *str)
{
    int i = 0;

    for (i = 0; i < sbcount; i++)
    {
        if (!strcmp(str, table[i].name))
        {
            return table[i].address;
        }
    }

    // NOTE : add the detect of "using nondeclared variable".

    if (sbcount >= TBLSIZE)
        error(RUNOUT);

    error(UNDEFINEDVARIABLE);

    return 0;
}

int setadd(char *str, int add)
{
    int i = 0;

    for (i = 0; i < sbcount; i++)
    {
        if (!strcmp(str, table[i].name))
        {
            printf("MOV r6 [%d]\n", add * 4);
            printf("MOV [%d] r6\n", table[i].address * 4);
            memory[table[i].address].isConst = memory[add].isConst;
            return table[i].address;
        }
    }

    if (sbcount >= TBLSIZE)
        error(RUNOUT);

    strcpy(table[sbcount].name, str);
    table[sbcount].address = getEmptyMemoryAddress(0);

    printf("MOV r6 [%d]\n", add * 4);
    printf("MOV [%d] r6\n", table[sbcount].address * 4);

    return table[sbcount++].address;
}

BTNode *makeNode(TokenSet tok, const char *lexe)
{
    BTNode *node = (BTNode *)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    node->address = -100;

    for (int i = 0; i < sbcount; i++)
    {
        if (!strcmp(node->lexeme, table[i].name))
        {
            node->address = table[i].address;
            break;
        }
    }

    return node;
}

void freeTree(BTNode *root)
{
    if (root != NULL)
    {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }

    return;
}

// NOTE : assign_expr := ID ASSIGN assign_expr | or_expr
BTNode *assign_expr(void)
{
    BTNode *retp = NULL, *left = NULL;

    if (match(ID))
    {
        left = makeNode(ID, getLexeme());
        advance();
        if (match(ASSIGN))
        {
            retp = makeNode(ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = assign_expr();
        }
        else
        {
            if (match(END))
            {
                goBack("\n");
            }
            else if (match(ENDFILE))
            {
                goBack("@");
            }
            else
            {
                goBack("\0");
            }
            goBack(left->lexeme);
            advance();
            free(left);
            retp = or_expr();
        }
    }
    else
    {
        retp = or_expr();
    }

    return retp;
}

// NOTE : or_expr := xor_expr or_expr_tail
BTNode *or_expr(void)
{
    BTNode *retp = xor_expr();
    return or_expr_tail(retp);
}

// NOTE : or_expr_tail := OR xor_expr or_expr_tail | NiL
BTNode *or_expr_tail(BTNode *left)
{
    if (match(OR))
    {
        BTNode *retp = makeNode(OR, getLexeme());
        advance();
        retp->left = left;
        retp->right = xor_expr();
        return or_expr_tail(retp);
    }
    else
    {
        return left;
    }
}

// NOTE : xor_expr := and_expr xor_expr_tail
BTNode *xor_expr(void)
{
    BTNode *retp = and_expr();
    return xor_expr_tail(retp);
}

// NOTE : xor_expr_tail := XOR and_expr xor_expr_tail | NiL
BTNode *xor_expr_tail(BTNode *left)
{
    if (match(XOR))
    {
        BTNode *retp = makeNode(XOR, getLexeme());
        advance();
        retp->left = left;
        retp->right = and_expr();
        return xor_expr_tail(retp);
    }
    else
    {
        return left;
    }
}

// NOTE : and_expr := addsub_expr and_expr_tail
BTNode *and_expr(void)
{
    BTNode *retp = addsub_expr();
    return and_expr_tail(retp);
}

// NOTE : and_expr_tail := AND addsub_expr and_expr_tail | NiL
BTNode *and_expr_tail(BTNode *left)
{
    if (match(AND))
    {
        BTNode *retp = makeNode(AND, getLexeme());
        advance();
        retp->left = left;
        retp->right = addsub_expr();
        return and_expr_tail(retp);
    }
    else
    {
        return left;
    }
}

// NOTE : addsub_expr := muldiv_expr addsub_expr_tail
BTNode *addsub_expr(void)
{
    BTNode *retp = muldiv_expr();
    return addsub_expr_tail(retp);
}

// NOTE : addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL
BTNode *addsub_expr_tail(BTNode *left)
{
    if (match(ADDSUB))
    {
        BTNode *retp = makeNode(ADDSUB, getLexeme());
        advance();
        retp->left = left;
        retp->right = muldiv_expr();
        return addsub_expr_tail(retp);
    }
    else
    {
        return left;
    }
}

// NOTE : muldiv_expr := unary_expr muldiv_expr_tail
BTNode *muldiv_expr(void)
{
    BTNode *retp = unary_expr();
    return muldiv_expr_tail(retp);
}

// NOTE : muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL
BTNode *muldiv_expr_tail(BTNode *left)
{
    if (match(MULDIV))
    {
        BTNode *retp = makeNode(MULDIV, getLexeme());
        advance();
        retp->left = left;
        retp->right = unary_expr();
        return muldiv_expr_tail(retp);
    }
    else
    {
        return left;
    }
}

// NOTE : unary_expr := ADDSUB unary_expr | factor
BTNode *unary_expr(void)
{
    BTNode *retp = NULL;

    if (match(ADDSUB))
    {
        retp = makeNode(ADDSUB, getLexeme());
        advance();
        retp->left = unary_expr();
    }
    else
    {
        retp = factor();
    }

    return retp;
}

// NOTE : factor := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN
BTNode *factor(void)
{
    BTNode *retp = NULL;

    if (match(INT))
    {
        retp = makeNode(INT, getLexeme());
        advance();
    }
    else if (match(ID))
    {
        retp = makeNode(ID, getLexeme());
        advance();
    }
    else if (match(INCDEC))
    {
        retp = makeNode(INCDEC, getLexeme());
        advance();

        if (match(ID))
        {
            //  NOTE : put the ID at the left node
            retp->left = makeNode(ID, getLexeme());
            advance();
        }
        else
        {
            error(SYNTAXERR);
        }
    }
    else if (match(LPAREN))
    {
        advance();
        retp = assign_expr();
        if (match(RPAREN))
            advance();
        else
            error(MISPAREN);
    }
    else
    {
        // if (strcmp(getLexeme(), "\0"))
        //     printf("%s\n", getLexeme());
        error(NOTNUMID);
    }

    return retp;
}

// NOTE : statement := ENDFILE | END | assign_expr END
void statement(void)
{
    BTNode *retp = NULL;

    if (match(ENDFILE))
    {
        printf("MOV r0 [%d]\n", table[0].address * 4);
        printf("MOV r1 [%d]\n", table[1].address * 4);
        printf("MOV r2 [%d]\n", table[2].address * 4);
        printf("EXIT 0\n");
        exit(0);
    }
    else if (match(END))
    {
        // printf(">> ");
        advance();
    }
    else
    {
        retp = assign_expr();
        if (match(END) || match(ENDFILE))
        {
            // printf("answer : %d\n", evaluateTree(retp));
            // printf("Prefix traversal: ");
            // printPrefix(retp);
            // printf("\n");
            generateAssemblyCode(retp);
            // printf("\n");
            freeTree(retp);
            // printf(">> ");
            advance();
        }
        else
        {
            error(SYNTAXERR);
        }
    }
}

// NOTE : add more error detect in need.
void err(ErrorType errorNum)
{
    if (PRINTERR)
    {
        fprintf(stderr, "error: ");
        switch (errorNum)
        {
        case MISPAREN:
            fprintf(stderr, "mismatched parenthesis\n");
            break;
        case NOTNUMID:
            fprintf(stderr, "number or identifier expected\n");
            break;
        case NOTFOUND:
            fprintf(stderr, "variable not defined\n");
            break;
        case RUNOUT:
            fprintf(stderr, "out of memory\n");
            break;
        case NOTLVAL:
            fprintf(stderr, "lvalue required as an operand\n");
            break;
        case DIVZERO:
            fprintf(stderr, "divide by constant zero\n");
            break;
        case SYNTAXERR:
            fprintf(stderr, "syntax error\n");
            break;
        case UNDEFINEDVARIABLE:
            fprintf(stderr, "use the variable that are not initialize\n");
            break;
        default:
            fprintf(stderr, "undefined error\n");
            break;
        }
    }
    printf("EXIT 1\n");
    exit(0);
}

// NOTE : parser.c

int main()
{
    // freopen("user_input.txt", "r", stdin);
    // freopen("user_output.txt", "w", stdout);

    initTable();
    // printf(">> ");

    while (1)
    {
        statement();
    }
    return 0;
}
