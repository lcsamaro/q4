#include "lexer.h"

#include <ctype.h>
#include <stdio.h>

int q4_lex(char *in, q4_lex_callback callback, void *context) {
	char *c = in, ch;
	int sep[256] = {0}, str = 0, comment = 0;
	sep['('] = sep[')'] = sep['['] = sep[']'] = 1;
	while (ch = *c++) {
		if(!str && ch == ';') comment = 1;
		if (comment) {
			if (ch == '\n') { comment = 0; in = c; }
			continue;
		}
		if (ch == '\"') {
			if (!(str = !str)) {
				c[-1] = '\0';
				callback(in+1, Q4_TOKEN_STRING, context);
				in = c;
			}
		}
		if (str) continue;
		if (sep[ch]) {
			c[-1] = '\0';
			if (*in) callback(in, Q4_TOKEN_WORD, context);
			callback(NULL, ch, context);
			in = c;
		} else if (isspace(ch)) {
			c[-1] = '\0';
			if (*in) callback(in, Q4_TOKEN_WORD, context);
			in = c;
		}
	}
	if (str) return 1;
	if (*in) callback(in, Q4_TOKEN_WORD, context);
	return 0;
}
