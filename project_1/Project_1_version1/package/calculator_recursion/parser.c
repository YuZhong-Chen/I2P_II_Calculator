#include "parser.h"
#include "codeGen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sbcount = 0; // the number of the variables
Symbol table[TBLSIZE];
Memory memory[TBLSIZE];

void initTable(void)
{
    initMemory();

    strcpy(table[0].name, "x");
    table[0].address = getEmptyMemoryAddress(0);
    // table[0].val = 0;
    // memory[table[0].address].val = 0;

    strcpy(table[1].name, "y");
    table[1].address = getEmptyMemoryAddress(0);
    // table[1].val = 0;
    // memory[table[1].address].val = 0;

    strcpy(table[2].name, "z");
    table[2].address = getEmptyMemoryAddress(0);
    // table[2].val = 0;
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

// TODO : use the register to calculate temp value.
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
