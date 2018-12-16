#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

enum {
    IF,
    LBRA,
    RBRA,
    LPAR,
    RPAR,
    EQ,
    DIGIT,
    IDE,
    STR,
};

struct token {
    int32_t type;
    char value[100];
} tokens[20] = {
};
int32_t tsi;

int32_t
toktype(const char *tok) {
    if (strcmp(tok, "if") == 0) {
        return IF;
    } else if (strcmp(tok, "{") == 0) {
        return LBRA;
    } else if (strcmp(tok, "}") == 0) {
        return RBRA;
    } else if (strcmp(tok, "(") == 0) {
        return LPAR;
    } else if (strcmp(tok, ")") == 0) {
        return RPAR;
    } else if (strcmp(tok, "==") == 0) {
        return EQ;
    } else if (isdigit(tok[0])) {
        return DIGIT;
    } else {
        return IDE;
    }
}

void
parse(void) {
    char buf[100] = {0};
    int32_t bi = 0;
    char c;

    for (; (c = getchar()) != EOF; ) {
        if (isspace(c)) {
            if (bi > 0) {
                strcpy(tokens[tsi].value, buf);
                tokens[tsi].type = toktype(buf);
                tsi++;
                bi = 0;
            }
        } else if (c == '"') {
            for (; (c = getchar()) != EOF; ) {
                if (c == '"') {
                    break;
                }
                buf[bi++] = c;
            }
            buf[bi] = '\0';
            strcpy(tokens[tsi].value, buf);
            tokens[tsi].type = STR;
            tsi++;
            bi = 0;
        } else if (c == '(' || c == ')') {
            if (bi > 0) {
                strcpy(tokens[tsi].value, buf);
                tokens[tsi].type = toktype(buf);
                tsi++;
                bi = 0;
            }
            
            sprintf(buf, "%c", c);
            strcpy(tokens[tsi].value, buf);
            tokens[tsi].type = toktype(buf);
            tsi++;
            bi = 0;
        } else {
            buf[bi++] = c;
            buf[bi] = '\0';
        }
    }
}

void
showtokens(void) {
    int32_t i;
    for (i = 0; i < tsi; ++i) {
        printf("type[%d] value[%s]\n", tokens[i].type, tokens[i].value);
    }
}

struct node {
    int32_t type;
    char value[100];
    struct node *lhs;
    struct node *rhs;
};

struct node *
tok2node(struct token *t) {
    struct node *n = calloc(1, sizeof(*n));
    n->type = t->type;
    memmove(n->value, t->value, sizeof n->value);
    return n;
}

/*
      if
    /     \
   ==      print
  /  \    /      \
 1    1  "string" nil
*/

void
buildtree(void) {
    struct node *root = NULL;
    int32_t i;

    for (i = 0; i < tsi; ++i) {
        struct node *node = tok2node(&tokens[i]);
        if (root == NULL) {
            root = node;
        }

        switch (node->type) {
        case IF:
            break;
        }
    }
}

int
main(void) {
    parse();
    showtokens();
    buildtree();
    return 0;
}
