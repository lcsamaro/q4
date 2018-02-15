#ifndef Q4_VALUE_H
#define Q4_VALUE_H

/* NaN Boxing */
/* assumptions: little endian */

#include <stdint.h>

typedef union { double d; uint64_t u; } q4_value;

struct q4_cons {
	q4_value car;
	q4_value cdr;
};

struct q4_ref {
	void *value;
	uint32_t ref;
};

typedef uint32_t q4_quote;
typedef uint32_t q4_integer;
typedef uint32_t q4_symbol;

q4_value q4_value_copy(q4_value bv);
void q4_value_destroy(q4_value bv);

q4_value q4_value_make_nil();

q4_value q4_value_make_integer(uint32_t v);
uint32_t q4_value_get_integer(q4_value bv);

q4_value q4_value_make_quote(q4_quote v);
q4_quote q4_value_get_quote(q4_value bv);

q4_value q4_value_make_symbol(q4_symbol v);
q4_symbol q4_value_get_symbol(q4_value bv);

q4_value q4_value_make_string(char *s);
char *q4_value_get_string(q4_value bv);

q4_value q4_value_make_cons(q4_value lhs, q4_value rhs);
struct q4_cons *q4_value_get_cons(q4_value bv);

void q4_value_disp(q4_value bv);
q4_value q4_value_add(q4_value lhs, q4_value rhs);
q4_value q4_value_sub(q4_value lhs, q4_value rhs);
q4_value q4_value_mul(q4_value lhs, q4_value rhs);
q4_value q4_value_div(q4_value lhs, q4_value rhs);

int q4_value_isfalse(q4_value v);

#endif
