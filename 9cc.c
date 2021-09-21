#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Tokenizer
//

typedef enum {
	TK_RESERVED, // symbol
	TK_NUM,      // integer
	TK_EOF,      // string
} TokenKind;

typedef struct Token Token;

// struct Token
struct Token {
	TokenKind kind; // Token type
	Token *next;    // next Token
	int val;        // as kind is TK_NUM, the integer
	char *str;      // Token string
};

// Input programs
char *user_input;

// Current Token
Token *token;

// Reports an error location and exit.
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // Output `pos` blanks
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// Consumes the current token if it matches `op`.
bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
		return false;
	token = token->next;
	return true;
}

// Ensure that the current token is `op`.
void expect(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
		error("'%c'ではありません", op);
	token = token->next;
}

// Ensure that the current token is TK_NUM
int expect_number() {
	if (token->kind != TK_NUM)
		error("数ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

// Create token and join `cur`
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// Input string `p` tokenize returns new tokens.
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // skip blanks
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	// Do tokenize
	token = tokenize(argv[1]);

	// Front assembly
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	// Ensure TK_NUM at initial formula and output mov order.
	printf("  mov rax, %d\n", expect_number());

    // `+ <nums>`or`- <nums>`that token consume.
    // Output assembly
    while (!at_eof()) {
        if (consume('+')) {
            printf("  add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("  sub rax, %d\n", expect_number());
    }

    printf("  ret\n");
    return 0;
}

