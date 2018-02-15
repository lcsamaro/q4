#ifndef Q4_LEXER_H
#define Q4_LEXER_H

enum q4_token {
	Q4_TOKEN_WORD, Q4_TOKEN_NUMBER, Q4_TOKEN_STRING,
	Q4_TOKEN_LROUND = '(', Q4_TOKEN_RROUND = ')',
	Q4_TOKEN_LBOX   = '[', Q4_TOKEN_RBOX   = ']'
};

typedef void(*q4_lex_callback)(const char*, int, void*);

int q4_lex(char *in, q4_lex_callback callback, void *context);

#endif
