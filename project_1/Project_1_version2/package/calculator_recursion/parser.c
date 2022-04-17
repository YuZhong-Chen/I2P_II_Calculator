#include "parser.h"
#include "codeGen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sbcount = 0;
Symbol table[TBLSIZE];

void initTable(void)
{
    for (int i = 0; i < 80; i++)
    {
        memory[i].isTemp = memory[i].isUse = memory[i].isConst = memory[i].val = 0;
    }

    strcpy(table[0].name, "x");
    table[0].address = getSpace(0, 0, 0);

    strcpy(table[1].name, "y");
    table[1].address = getSpace(0, 0, 0);

    strcpy(table[2].name, "z");
    table[2].address = getSpace(0, 0, 0);

    sbcount = 3;
}

int getadd(char *str)
{
    int i = 0;

    for (i = 0; i < sbcount; i++)
        if (!strcmp(str, table[i].name))
            return table[i].address;

    if (sbcount >= TBLSIZE)
        error(RUNOUT);

    error(SYNTAXERR);

    return 0;
}

int setadd(char *str, int add)
{
    int i = 0;

    for (i = 0; i < sbcount; i++)
    {
        if (!strcmp(str, table[i].name))
        {
            if (add >= 65)
            {
                printf("MOV [%d] r%d\n", table[i].address * 4, add - 65);
            }
            else
            {
                printf("MOV r6 [%d]\n", add * 4);
                printf("MOV [%d] r6\n", table[i].address * 4);
            }
            return add;
        }
    }

    if (sbcount >= TBLSIZE)
        error(RUNOUT);

    strcpy(table[sbcount].name, str);
    table[sbcount].address = getSpace(0, 0, 0);
    if (add >= 65)
    {
        printf("MOV [%d] r%d\n", table[sbcount].address * 4, add - 65);
    }
    else
    {
        printf("MOV r6 [%d]\n", add * 4);
        printf("MOV [%d] r6\n", table[sbcount].address * 4);
    }

    sbcount++;
    return add;
}

BTNode *makeNode(TokenSet tok, const char *lexe)
{
    BTNode *node = (BTNode *)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;

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
}

void statement(void)
{
    BTNode *retp = NULL;

    if (match(ENDFILE))
    {
        printf("MOV r0 [0]\n");
        printf("MOV r1 [4]\n");
        printf("MOV r2 [8]\n");
        printf("EXIT 0\n");
        exit(0);
    }
    else if (match(END))
    {
        advance();
    }
    else
    {
        retp = assign_expr();
        if (match(END))
        {
            evaluateTree(retp);

            // printf("Prefix traversal: ");
            // printPrefix(retp);
            // printf("\n\n");

            freeTree(retp);
            for (int i = 0; i < 80; i++)
            {
                if (memory[i].isTemp)
                    freeSpace(i);
            }
            advance();
        }
        else
        {
            error(SYNTAXERR);
        }
    }
}

// assign_expr := ID ASSIGN assign_expr | or_expr
BTNode *assign_expr(void)
{
    BTNode *retp = NULL, *left = or_expr();

    if (match(ASSIGN))
    {
        if (left->data != ID)
        {
            error(SYNTAXERR);
        }
        retp = makeNode(ASSIGN, getLexeme());
        advance();
        retp->left = left;
        retp->right = assign_expr();
    }
    else
    {
        retp = left;
    }
    return retp;
}

BTNode *or_expr(void)
{
    BTNode *node = xor_expr();
    return or_expr_tail(node);
}

BTNode *or_expr_tail(BTNode *left)
{
    BTNode *node = NULL;

    if (match(OR))
    {
        node = makeNode(OR, getLexeme());
        advance();
        node->left = left;
        node->right = xor_expr();
        return or_expr_tail(node);
    }
    else
    {
        return left;
    }
}

BTNode *xor_expr(void)
{
    BTNode *node = and_expr();
    return xor_expr_tail(node);
}

BTNode *xor_expr_tail(BTNode *left)
{
    BTNode *node = NULL;

    if (match(XOR))
    {
        node = makeNode(XOR, getLexeme());
        advance();
        node->left = left;
        node->right = and_expr();
        return xor_expr_tail(node);
    }
    else
    {
        return left;
    }
}

BTNode *and_expr(void)
{
    BTNode *node = addsub_expr();
    return and_expr_tail(node);
}

BTNode *and_expr_tail(BTNode *left)
{
    BTNode *node = NULL;

    if (match(AND))
    {
        node = makeNode(AND, getLexeme());
        advance();
        node->left = left;
        node->right = addsub_expr();
        return and_expr_tail(node);
    }
    else
    {
        return left;
    }
}

BTNode *addsub_expr(void)
{
    BTNode *node = muldiv_expr();
    return addsub_expr_tail(node);
}

BTNode *addsub_expr_tail(BTNode *left)
{
    BTNode *node = NULL;

    if (match(ADDSUB))
    {
        node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = left;
        node->right = muldiv_expr();
        return addsub_expr_tail(node);
    }
    else
    {
        return left;
    }
}

BTNode *muldiv_expr(void)
{
    BTNode *node = unary_expr();
    return muldiv_expr_tail(node);
}

BTNode *muldiv_expr_tail(BTNode *left)
{
    BTNode *node = NULL;

    if (match(MULDIV))
    {
        node = makeNode(MULDIV, getLexeme());
        advance();
        node->left = left;
        node->right = unary_expr();
        return muldiv_expr_tail(node);
    }
    else
    {
        return left;
    }
}

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
    else if (!match(ASSIGN))
    {
        error(NOTNUMID);
    }

    return retp;
}

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
        default:
            fprintf(stderr, "undefined error\n");
            break;
        }
    }
    printf("EXIT 1\n");
    exit(0);
}
