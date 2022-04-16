#include "lex.h"
#include "parser.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];

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

    error(SYNTAXERR);
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
