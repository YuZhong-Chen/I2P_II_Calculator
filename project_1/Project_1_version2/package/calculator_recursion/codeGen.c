#include "codeGen.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Memory memory[80];

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
                    error(DIVZERO);
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
        default:
            retadd = 0;
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
