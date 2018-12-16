#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

struct token {
    char value[100];
} tokens[20] = {
};
int tslen;
int tsi;

void
tokenize() {
    char c;
    char buf[100], *bp = buf;

    for (; (c = getchar()) != EOF; ) {
        if (isspace(c)) {
            if (bp != buf) {
                *bp = '\0';
                strcpy(tokens[tslen++].value, buf);
                bp = buf;
            }
        } else {
            *bp++ = c;
        }
    }

    int i;
    for (i = 0; i < tslen; ++i) {
        printf("[%s]\n", tokens[i].value);
    }
}

struct token *
curt(void) {
    return &tokens[tsi];
}

struct token *
gett(void) {
    if (tsi >= tslen) {
        return NULL; 
    }

    return &tokens[tsi++];
}

void
ungett(void) {
    if (tsi > 0) {
        --tsi;
    }
}

/* BNF

<expr>   ::= <term> [ ('+'|'-') <term> ]*
<term>   ::= <factor> [ ('*'|'/') <factor> ]*
<factor> ::= <number> | '(' <expr> ')'
<number> ::= [ 0~9 ]*

*/

int expr(void);
int term(void);
int factor(void);
int number(void);

int
expr(void) {
    struct token *t;
    int val = term();

    for (; (t = gett()); ) {
        if (strcmp(t->value, "+") != 0 ||
            strcmp(t->value, "-") != 0) {
            break;
        }
        int val2 = term();
        if (strcmp(t->value, "+") == 0) {
            val += val2;
        } else {
            val -= val2;
        }
    }

    return val;
}

// <term> ::= <factor> [ ('*'|'/') <factor> ]*
int
term(void) {
    struct token *t;
    int val = factor();

    for (; (t = gett()); ) {
        if (strcmp(t->value, "*") != 0 ||
            strcmp(t->value, "/") != 0) {
            break;
        }
        int val2 = factor();

        if (strcmp(t->value, "*") == 0) {
            val *= val2;
        } else {
            val /= val2;
        }
    }

    return val;
}

// <factor> ::= <number> | '(' <expr> ')'
int
factor(void) {
    struct token *t = curt();
    if (isdigit(t->value[0])) {
        return number();
    }

    // 構文が正しければ次は '('
    gett(); // skip '('
    int val = expr();
    gett(); // skip ')'
    return val;
}

int
number(void) {
    struct token *t = gett();
    return atoi(t->value);
}

int
main(void) {
    tokenize();
    int val = expr();
    printf("%d\n", val);
    return 0;
}
