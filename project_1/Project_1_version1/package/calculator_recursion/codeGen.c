#include "codeGen.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
            memory[retval].isConst = 0;
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
