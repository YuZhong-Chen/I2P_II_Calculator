#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLEN 256
#define TBLSIZE 64
#define PRINTERR 0
#define error() err();
typedef enum
{
    UNKNOWN,
    END,
    ENDFILE,
    INT,
    ID,
    ADDSUB,
    MULDIV,
    INCDEC,
    OR,
    AND,
    XOR,
    ASSIGN,
    LPAREN,
    RPAREN
} TokenSet;
typedef struct
{
    int isTemp;
    int isConst;
    int isUse;
    int val;
} Memory;
typedef struct _Node
{
    TokenSet data;
    int val;
    char lexeme[MAXLEN];
    struct _Node *left;
    struct _Node *right;
} BTNode;
typedef struct
{
    int val;
    char name[MAXLEN];
    int address;
} Symbol;
int sbcount = 0;
Symbol table[TBLSIZE];
Memory memory[80];
int match(TokenSet token);
void advance(void);
char *getLexeme(void);
int getSpace(int Temp, int Const, int Value);
void freeSpace(int address);
int evaluateTree(BTNode *root);
void printPrefix(BTNode *root);
void initTable(void);
int getadd(char *str);
int setadd(char *str, int add);
BTNode *makeNode(TokenSet tok, const char *lexe);
void freeTree(BTNode *root);
void statement(void);
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
void err();
TokenSet getToken(void);
TokenSet curToken = UNKNOWN;
char lexeme[MAXLEN];
TokenSet getToken(void)
{
    int i = 0;
    char c = '\0';

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
        ungetc(c, stdin);
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
    else if (c == '|')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return OR;
    }
    else if (c == '&')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return AND;
    }
    else if (c == '^')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return XOR;
    }
    else if (c == '\n')
    {
        lexeme[0] = '\0';
        return END;
    }
    else if (c == '=')
    {
        strcpy(lexeme, "=");
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
    else if (isalpha(c) || c == '_')
    {
        lexeme[0] = c;
        for (int i = 1;; i++)
        {
            c = fgetc(stdin);
            if (isdigit(c) || isalpha(c) || (c == '_'))
            {
                lexeme[i] = c;
            }
            else
            {
                ungetc(c, stdin);
                lexeme[i] = '\0';
                break;
            }
        }
        return ID;
    }
    else if (c == EOF)
    {
        return ENDFILE;
    }
    else
    {
        return UNKNOWN;
    }
}

void advance(void)
{
    curToken = getToken();
}

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

int getSpace(int Temp, int Const, int Value)
{
    if (Temp)
    {
        int find = 0;
        int i;
        for (i = 65; i <= 70; i++)
        {
            if (!memory[i].isUse)
            {
                find = 1;
                break;
            }
        }

        if (find)
        {
            memory[i].isUse = 1;
            memory[i].isConst = Const;
            memory[i].val = Value;
            memory[i].isTemp = Temp;
            return i;
        }
    }

    int find = 0;

    for (int i = 0; i < 80; i++)
    {
        if (!memory[i].isUse)
        {
            find = i;
            break;
        }
    }

    memory[find].isUse = 1;
    memory[find].isConst = Const;
    memory[find].isTemp = Temp;
    memory[find].val = Value;

    return find;
}

void freeSpace(int address)
{
    memory[address].isUse = memory[address].isTemp = memory[address].isConst = memory[address].val = 0;

    return;
}

int evaluateTree(BTNode *root)
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
            retadd = getSpace(1, 1, atoi(root->lexeme));
            if (retadd >= 65)
            {
                printf("MOV r%d %d\n", retadd - 65, atoi(root->lexeme));
            }
            else
            {
                printf("MOV r6 %d\n", atoi(root->lexeme));
                printf("MOV [%d] r6\n", retadd * 4);
            }
            break;
        case ASSIGN:
            radd = evaluateTree(root->right);
            retadd = setadd(root->left->lexeme, radd);
            break;
        case ADDSUB:
        case MULDIV:
            ladd = evaluateTree(root->left);
            radd = evaluateTree(root->right);

            if (strcmp(root->lexeme, "+") == 0)
            {
                if (root->right != NULL)
                {
                    if (ladd >= 65 && radd >= 65)
                    {
                        printf("ADD r%d r%d\n", ladd - 65, radd - 65);
                        retadd = ladd;
                        memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                        if (memory[retadd].isConst)
                        {
                            memory[retadd].val = memory[ladd].val + memory[radd].val;
                        }
                        freeSpace(radd);
                    }
                    else if (ladd >= 65 && radd <= 64)
                    {
                        printf("MOV r6 [%d]\n", radd * 4);
                        printf("ADD r%d r6\n", ladd - 65);
                        retadd = ladd;
                        memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                        if (memory[retadd].isConst)
                        {
                            memory[retadd].val = memory[ladd].val + memory[radd].val;
                        }
                        if (memory[radd].isTemp)
                        {
                            freeSpace(radd);
                        }
                    }
                    else if (ladd <= 64 && radd >= 65)
                    {
                        printf("MOV r6 [%d]\n", ladd * 4);
                        printf("ADD r%d r6\n", radd - 65);
                        retadd = radd;
                        memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                        if (memory[retadd].isConst)
                        {
                            memory[retadd].val = memory[ladd].val + memory[radd].val;
                        }
                        if (memory[ladd].isTemp)
                        {
                            freeSpace(ladd);
                        }
                    }
                    else
                    {
                        retadd = getSpace(1, 0, 0);
                        if (retadd >= 65)
                        {
                            printf("MOV r%d [%d]\n", retadd - 65, ladd * 4);
                            printf("MOV r6 [%d]\n", radd * 4);
                            printf("ADD r%d r6\n", retadd - 65);
                            memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                            if (memory[retadd].isConst)
                            {
                                memory[retadd].val = memory[ladd].val + memory[radd].val;
                            }
                            if (memory[ladd].isTemp)
                            {
                                freeSpace(ladd);
                            }
                            if (memory[radd].isTemp)
                            {
                                freeSpace(radd);
                            }
                        }
                        else
                        {
                            freeSpace(retadd);
                            printf("MOV r6 [%d]\n", ladd * 4);
                            printf("MOV r7 [%d]\n", radd * 4);
                            printf("ADD r6 r7\n");
                            printf("MOV [%d] r6\n", ladd * 4);
                            retadd = ladd;
                            memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                            if (memory[retadd].isConst)
                            {
                                memory[retadd].val = memory[ladd].val + memory[radd].val;
                            }

                            if (memory[radd].isTemp)
                            {
                                freeSpace(radd);
                            }
                        }
                    }
                }
                else
                {
                    retadd = ladd;
                }
            }
            else if (strcmp(root->lexeme, "-") == 0)
            {
                if (root->right == NULL)
                {
                    if (ladd >= 65)
                    {
                        printf("MOV r6 0\n");
                        printf("SUB r6 r%d\n", ladd - 65);
                        printf("MOV r%d r6\n", ladd - 65);
                        retadd = ladd;
                        memory[retadd].val *= -1;
                    }
                    else
                    {
                        retadd = getSpace(1, memory[ladd].isConst, (-1) * memory[ladd].val);
                        if (retadd >= 65)
                        {
                            printf("MOV r%d 0\n", retadd - 65);
                            printf("MOV r6 [%d]\n", ladd * 4);
                            printf("SUB r%d r6\n", retadd - 65);
                        }
                        else
                        {
                            freeSpace(retadd);
                            retadd = ladd;
                            memory[retadd].val *= -1;
                            printf("MOV r6 0\n");
                            printf("MOV r7 [%d]\n", retadd * 4);
                            printf("SUB r6 r7\n");
                            printf("MOV [%d] r6", retadd * 4);
                        }
                        if (memory[ladd].isTemp)
                        {
                            freeSpace(ladd);
                        }
                    }
                }
                else if (root->right != NULL)
                {
                    if (ladd >= 65 && radd >= 65)
                    {
                        printf("SUB r%d r%d\n", ladd - 65, radd - 65);
                        retadd = ladd;
                        memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                        if (memory[retadd].isConst)
                        {
                            memory[retadd].val = memory[ladd].val - memory[radd].val;
                        }
                        freeSpace(radd);
                    }
                    else if (ladd >= 65 && radd <= 64)
                    {
                        printf("MOV r6 [%d]\n", radd * 4);
                        printf("SUB r%d r6\n", ladd - 65);
                        retadd = ladd;
                        memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                        if (memory[retadd].isConst)
                        {
                            memory[retadd].val = memory[ladd].val - memory[radd].val;
                        }
                        if (memory[radd].isTemp)
                        {
                            freeSpace(radd);
                        }
                    }
                    else if (ladd <= 64 && radd >= 65)
                    {
                        printf("MOV r6 [%d]\n", ladd * 4);
                        printf("SUB r%d r6\n", radd - 65);
                        retadd = radd;
                        memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                        if (memory[retadd].isConst)
                        {
                            memory[retadd].val = memory[ladd].val - memory[radd].val;
                        }
                        if (memory[ladd].isTemp)
                        {
                            freeSpace(ladd);
                        }
                    }
                    else
                    {
                        retadd = getSpace(1, 0, 0);
                        if (retadd >= 65)
                        {
                            printf("MOV r%d [%d]\n", retadd - 65, ladd * 4);
                            printf("MOV r6 [%d]\n", radd * 4);
                            printf("SUB r%d r6\n", retadd - 65);
                            memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                            if (memory[retadd].isConst)
                            {
                                memory[retadd].val = memory[ladd].val - memory[radd].val;
                            }
                            if (memory[ladd].isTemp)
                            {
                                freeSpace(ladd);
                            }
                            if (memory[radd].isTemp)
                            {
                                freeSpace(radd);
                            }
                        }
                        else
                        {
                            freeSpace(retadd);
                            printf("MOV r6 [%d]\n", ladd * 4);
                            printf("MOV r7 [%d]\n", radd * 4);
                            printf("SUB r6 r7\n");
                            printf("MOV [%d] r6\n", ladd * 4);
                            retadd = ladd;
                            memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                            if (memory[retadd].isConst)
                            {
                                memory[retadd].val = memory[ladd].val - memory[radd].val;
                            }
                            if (memory[radd].isTemp)
                            {
                                freeSpace(radd);
                            }
                        }
                    }
                }
            }
            else if (strcmp(root->lexeme, "*") == 0)
            {
                if (ladd >= 65 && radd >= 65)
                {
                    printf("MUL r%d r%d\n", ladd - 65, radd - 65);
                    retadd = ladd;
                    memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val * memory[radd].val;
                    }
                    freeSpace(radd);
                }
                else if (ladd >= 65 && radd <= 64)
                {
                    printf("MOV r6 [%d]\n", radd * 4);
                    printf("MUL r%d r6\n", ladd - 65);
                    retadd = ladd;
                    memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val * memory[radd].val;
                    }
                    if (memory[radd].isTemp)
                    {
                        freeSpace(radd);
                    }
                }
                else if (ladd <= 64 && radd >= 65)
                {
                    printf("MOV r6 [%d]\n", ladd * 4);
                    printf("MUL r%d r6\n", radd - 65);
                    retadd = radd;
                    memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val * memory[radd].val;
                    }
                    if (memory[ladd].isTemp)
                    {
                        freeSpace(ladd);
                    }
                }
                else
                {
                    retadd = getSpace(1, 0, 0);
                    if (retadd >= 65)
                    {
                        printf("MOV r%d [%d]\n", retadd - 65, ladd * 4);
                        printf("MOV r6 [%d]\n", radd * 4);
                        printf("MUL r%d r6\n", retadd - 65);
                        memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                        if (memory[retadd].isConst)
                        {
                            memory[retadd].val = memory[ladd].val * memory[radd].val;
                        }
                        if (memory[ladd].isTemp)
                        {
                            freeSpace(ladd);
                        }
                        if (memory[radd].isTemp)
                        {
                            freeSpace(radd);
                        }
                    }
                    else
                    {
                        freeSpace(retadd);
                        printf("MOV r6 [%d]\n", ladd * 4);
                        printf("MOV r7 [%d]\n", radd * 4);
                        printf("MUL r6 r7\n");
                        printf("MOV [%d] r6\n", ladd * 4);
                        retadd = ladd;
                        memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                        if (memory[retadd].isConst)
                        {
                            memory[retadd].val = memory[ladd].val * memory[radd].val;
                        }
                        if (memory[radd].isTemp)
                        {
                            freeSpace(radd);
                        }
                    }
                }
            }
            else if (strcmp(root->lexeme, "/") == 0)
            {
                if (memory[radd].isConst && memory[radd].val == 0)
                {
                    error();
                }

                if (ladd >= 65 && radd >= 65)
                {
                    printf("DIV r%d r%d\n", ladd - 65, radd - 65);
                    retadd = ladd;
                    memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val / memory[radd].val;
                    }
                    freeSpace(radd);
                }
                else if (ladd >= 65 && radd <= 64)
                {
                    printf("MOV r6 [%d]\n", radd * 4);
                    printf("DIV r%d r6\n", ladd - 65);
                    retadd = ladd;
                    memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val / memory[radd].val;
                    }
                    if (memory[radd].isTemp)
                    {
                        freeSpace(radd);
                    }
                }
                else if (ladd <= 64 && radd >= 65)
                {
                    printf("MOV r6 [%d]\n", ladd * 4);
                    printf("DIV r6 r%d\n", radd - 65);
                    printf("MOV r%d r6\n", radd - 65);
                    retadd = radd;
                    memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val / memory[radd].val;
                    }
                    if (memory[ladd].isTemp)
                    {
                        freeSpace(ladd);
                    }
                }
                else
                {
                    retadd = getSpace(1, 0, 0);
                    if (retadd >= 65)
                    {
                        printf("MOV r%d [%d]\n", retadd - 65, ladd * 4);
                        printf("MOV r6 [%d]\n", radd * 4);
                        printf("DIV r%d r6\n", retadd - 65);
                        memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                        if (memory[retadd].isConst)
                        {
                            memory[retadd].val = memory[ladd].val / memory[radd].val;
                        }
                        if (memory[ladd].isTemp)
                        {
                            freeSpace(ladd);
                        }
                        if (memory[radd].isTemp)
                        {
                            freeSpace(radd);
                        }
                    }
                    else
                    {
                        freeSpace(retadd);
                        printf("MOV r6 [%d]\n", ladd * 4);
                        printf("MOV r7 [%d]\n", radd * 4);
                        printf("DIV r6 r7\n");
                        printf("MOV [%d] r6\n", ladd * 4);
                        retadd = ladd;
                        memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                        if (memory[retadd].isConst)
                        {
                            memory[retadd].val = memory[ladd].val / memory[radd].val;
                        }
                        if (memory[radd].isTemp)
                        {
                            freeSpace(radd);
                        }
                    }
                }
            }
            break;
        case INCDEC:
            ladd = evaluateTree(root->left);
            retadd = ladd;
            if (!strcmp(root->lexeme, "++"))
            {
                printf("MOV r6 [%d]\n", ladd * 4);
                printf("MOV r7 1\n");
                printf("ADD r6 r7\n");
                printf("MOV [%d] r6\n", retadd * 4);
            }
            else if (!strcmp(root->lexeme, "--"))
            {
                printf("MOV r6 [%d]\n", ladd * 4);
                printf("MOV r7 1\n");
                printf("SUB r6 r7\n");
                printf("MOV [%d] r6\n", retadd * 4);
            }
            break;
        case OR:
            ladd = evaluateTree(root->left);
            radd = evaluateTree(root->right);

            if (ladd >= 65 && radd >= 65)
            {
                printf("OR r%d r%d\n", ladd - 65, radd - 65);
                retadd = ladd;
                memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                if (memory[retadd].isConst)
                {
                    memory[retadd].val = memory[ladd].val | memory[radd].val;
                }
                freeSpace(radd);
            }
            else if (ladd >= 65 && radd <= 64)
            {
                printf("MOV r6 [%d]\n", radd * 4);
                printf("OR r%d r6\n", ladd - 65);
                retadd = ladd;
                memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                if (memory[retadd].isConst)
                {
                    memory[retadd].val = memory[ladd].val | memory[radd].val;
                }
                if (memory[radd].isTemp)
                {
                    freeSpace(radd);
                }
            }
            else if (ladd <= 64 && radd >= 65)
            {
                printf("MOV r6 [%d]\n", ladd * 4);
                printf("OR r%d r6\n", radd - 65);
                retadd = radd;
                memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                if (memory[retadd].isConst)
                {
                    memory[retadd].val = memory[ladd].val | memory[radd].val;
                }
                if (memory[ladd].isTemp)
                {
                    freeSpace(ladd);
                }
            }
            else
            {
                retadd = getSpace(1, 0, 0);
                if (retadd >= 65)
                {
                    printf("MOV r%d [%d]\n", retadd - 65, ladd * 4);
                    printf("MOV r6 [%d]\n", radd * 4);
                    printf("OR r%d r6\n", retadd - 65);
                    memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val | memory[radd].val;
                    }
                    if (memory[ladd].isTemp)
                    {
                        freeSpace(ladd);
                    }
                    if (memory[radd].isTemp)
                    {
                        freeSpace(radd);
                    }
                }
                else
                {
                    freeSpace(retadd);
                    printf("MOV r6 [%d]\n", ladd * 4);
                    printf("MOV r7 [%d]\n", radd * 4);
                    printf("OR r6 r7\n");
                    printf("MOV [%d] r6\n", ladd * 4);
                    retadd = ladd;
                    memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val | memory[radd].val;
                    }
                    if (memory[radd].isTemp)
                    {
                        freeSpace(radd);
                    }
                }
            }
            break;
        case AND:
            ladd = evaluateTree(root->left);
            radd = evaluateTree(root->right);

            if (ladd >= 65 && radd >= 65)
            {
                printf("AND r%d r%d\n", ladd - 65, radd - 65);
                retadd = ladd;
                memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                if (memory[retadd].isConst)
                {
                    memory[retadd].val = memory[ladd].val & memory[radd].val;
                }
                freeSpace(radd);
            }
            else if (ladd >= 65 && radd <= 64)
            {
                printf("MOV r6 [%d]\n", radd * 4);
                printf("AND r%d r6\n", ladd - 65);
                retadd = ladd;
                memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                if (memory[retadd].isConst)
                {
                    memory[retadd].val = memory[ladd].val & memory[radd].val;
                }
                if (memory[radd].isTemp)
                {
                    freeSpace(radd);
                }
            }
            else if (ladd <= 64 && radd >= 65)
            {
                printf("MOV r6 [%d]\n", ladd * 4);
                printf("AND r%d r6\n", radd - 65);
                retadd = radd;
                memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                if (memory[retadd].isConst)
                {
                    memory[retadd].val = memory[ladd].val & memory[radd].val;
                }
                if (memory[ladd].isTemp)
                {
                    freeSpace(ladd);
                }
            }
            else
            {
                retadd = getSpace(1, 0, 0);
                if (retadd >= 65)
                {
                    printf("MOV r%d [%d]\n", retadd - 65, ladd * 4);
                    printf("MOV r6 [%d]\n", radd * 4);
                    printf("AND r%d r6\n", retadd - 65);
                    memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val & memory[radd].val;
                    }
                    if (memory[ladd].isTemp)
                    {
                        freeSpace(ladd);
                    }
                    if (memory[radd].isTemp)
                    {
                        freeSpace(radd);
                    }
                }
                else
                {
                    freeSpace(retadd);
                    printf("MOV r6 [%d]\n", ladd * 4);
                    printf("MOV r7 [%d]\n", radd * 4);
                    printf("AND r6 r7\n");
                    printf("MOV [%d] r6\n", ladd * 4);
                    retadd = ladd;
                    memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val & memory[radd].val;
                    }
                    if (memory[radd].isTemp)
                    {
                        freeSpace(radd);
                    }
                }
            }

            break;
        case XOR:
            ladd = evaluateTree(root->left);
            radd = evaluateTree(root->right);

            if (ladd >= 65 && radd >= 65)
            {
                printf("XOR r%d r%d\n", ladd - 65, radd - 65);
                retadd = ladd;
                memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                if (memory[retadd].isConst)
                {
                    memory[retadd].val = memory[ladd].val ^ memory[radd].val;
                }
                freeSpace(radd);
            }
            else if (ladd >= 65 && radd <= 64)
            {
                printf("MOV r6 [%d]\n", radd * 4);
                printf("XOR r%d r6\n", ladd - 65);
                retadd = ladd;
                memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                if (memory[retadd].isConst)
                {
                    memory[retadd].val = memory[ladd].val ^ memory[radd].val;
                }
                if (memory[radd].isTemp)
                {
                    freeSpace(radd);
                }
            }
            else if (ladd <= 64 && radd >= 65)
            {
                printf("MOV r6 [%d]\n", ladd * 4);
                printf("XOR r%d r6\n", radd - 65);
                retadd = radd;
                memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                if (memory[retadd].isConst)
                {
                    memory[retadd].val = memory[ladd].val ^ memory[radd].val;
                }
                if (memory[ladd].isTemp)
                {
                    freeSpace(ladd);
                }
            }
            else
            {
                retadd = getSpace(1, 0, 0);
                if (retadd >= 65)
                {
                    printf("MOV r%d [%d]\n", retadd - 65, ladd * 4);
                    printf("MOV r6 [%d]\n", radd * 4);
                    printf("XOR r%d r6\n", retadd - 65);
                    memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val ^ memory[radd].val;
                    }
                    if (memory[ladd].isTemp)
                    {
                        freeSpace(ladd);
                    }
                    if (memory[radd].isTemp)
                    {
                        freeSpace(radd);
                    }
                }
                else
                {
                    freeSpace(retadd);
                    printf("MOV r6 [%d]\n", ladd * 4);
                    printf("MOV r7 [%d]\n", radd * 4);
                    printf("XOR r6 r7\n");
                    printf("MOV [%d] r6\n", ladd * 4);
                    retadd = ladd;
                    memory[retadd].isConst = memory[ladd].isConst && memory[radd].isConst;
                    if (memory[retadd].isConst)
                    {
                        memory[retadd].val = memory[ladd].val ^ memory[radd].val;
                    }
                    if (memory[radd].isTemp)
                    {
                        freeSpace(radd);
                    }
                }
            }

            break;
        }
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
        error();

    error();

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
        error();

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
        advance();
    else
    {
        retp = assign_expr();
        if (match(END))
        {
            evaluateTree(retp);
            freeTree(retp);
            for (int i = 0; i < 80; i++)
                if (memory[i].isTemp)
                    freeSpace(i);
            advance();
        }
        else
            error();
    }
}
BTNode *assign_expr(void)
{
    BTNode *retp = NULL, *left = or_expr();
    if (match(ASSIGN))
    {
        if (left->data != ID)
        {
            error();
        }
        retp = makeNode(ASSIGN, getLexeme());
        advance();
        retp->left = left;
        retp->right = assign_expr();
    }
    else
        retp = left;
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
        return left;
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
        return left;
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
        return left;
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
        return left;
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
        return left;
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
        retp = factor();
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
            error();
    }
    else if (match(LPAREN))
    {
        advance();
        retp = assign_expr();
        if (match(RPAREN))
            advance();
        else
            error();
    }
    else if (!match(ASSIGN))
        error();
    return retp;
}
void err()
{
    printf("EXIT 1\n");
    exit(0);
}
int main()
{
    initTable();
    while (1)
        statement();
    return 0;
}