#include "value.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define bv_type_mask  UINT64_C(0xffff000000000000)
#define bv_value_mask UINT64_C(0x0000ffffffffffff)

#define bv_nil        UINT64_C(0x7ff7000000000000) /* value       */
#define bv_cptr       UINT64_C(0x7ff8000000000000) /* unused      */
#define bv_sstring    UINT64_C(0x7ff9000000000000) /* unused      */
#define bv_string     UINT64_C(0x7ffa000000000000) /* ref counted */
#define bv_cons       UINT64_C(0x7ffb000000000000) /* ref counted */
#define bv_quote      UINT64_C(0x7ffc000000000000) /* value       */
#define bv_integer    UINT64_C(0x7ffd000000000000) /* value       */
#define bv_symbol     UINT64_C(0x7ffe000000000000) /* value       */

uint64_t q4_value_type(q4_value v) { return v.u & bv_type_mask; }

q4_value q4_value_copy(q4_value bv) {
	q4_value n = bv;
	switch (q4_value_type(bv)) {
	case bv_string:
	case bv_cons: {
		struct q4_ref *r = (struct q4_ref*)(bv.u & bv_value_mask);
		r->ref++;
		} break;
	}
	return n;
}

void q4_value_destroy(q4_value bv) {
	uint64_t t = q4_value_type(bv);
	if (t == bv_string || t == bv_cons) {
		struct q4_ref *r = (struct q4_ref*)(bv.u & bv_value_mask);
		if (!(--r->ref)) {
			if (t == bv_cons) {
				struct q4_cons *cons = r->value;
				q4_value_destroy(cons->car);
				q4_value_destroy(cons->cdr);
			}
			free(r->value);
			r->value = NULL;
		}
	}
}

q4_value q4_value_make_nil() {
	q4_value v; v.u = bv_nil; return v;
}

q4_value q4_value_make_integer(uint32_t v) {
	q4_value bv; bv.u = bv_integer | v; return bv;
}

uint32_t q4_value_get_integer(q4_value bv) {
	return (uint32_t)(bv.u ^ bv_integer);
}

q4_value q4_value_make_quote(q4_quote v) {
	q4_value bv; bv.u = bv_quote | v; return bv;
}

q4_quote q4_value_get_quote(q4_value bv) {
	return (uint32_t)(bv.u ^ bv_quote);
}

q4_value q4_value_make_symbol(q4_symbol v) {
	q4_value bv;
	bv.u = bv_symbol | v;
	return bv;
}

q4_symbol q4_value_get_symbol(q4_value bv) {
	return (uint32_t)(bv.u ^ bv_symbol);
}

q4_value q4_value_make_cons(q4_value lhs, q4_value rhs) {
	q4_value bv;
	struct q4_cons *cons = malloc(sizeof(struct q4_cons));
	struct q4_ref *cr = malloc(sizeof(struct q4_ref));
	cr->ref = 1;
	cr->value = cons;

	cons->car = q4_value_copy(lhs);
	cons->cdr = q4_value_copy(rhs);

	bv.u = bv_cons | (uint64_t)cr;
	return bv;
}

struct q4_cons *q4_value_get_cons(q4_value bv) {
	return ((struct q4_ref*)(bv.u ^ bv_cons))->value;
}

q4_value q4_value_make_string(char *s) {
	q4_value bv;
	struct q4_ref *sr = malloc(sizeof(struct q4_ref));
	sr->ref = 1;
	sr->value = s;
	bv.u = bv_string | (uint64_t)sr;
	return bv;
}

char *q4_value_get_string(q4_value bv) {
	return ((struct q4_ref*)(bv.u ^ bv_string))->value;
}

void q4_value_disp(q4_value bv) {
  switch (q4_value_type(bv)) {
  case bv_nil     : printf("nil"); break;
  case bv_cptr    : printf("(cptr)"); break;
  case bv_sstring : printf("(sstring)"); break;
  case bv_string  : printf("%s", q4_value_get_string(bv)); break;
  case bv_cons    : {
    struct q4_cons *c = q4_value_get_cons(bv);
    printf("( ");
    q4_value_disp(c->car);
	printf(" . ");
    q4_value_disp(c->cdr);
    printf(")");
    } break;
  case bv_quote   : printf("%d (quote)", q4_value_get_quote(bv)); break;
  case bv_symbol  : printf("%d (symbol)", q4_value_get_symbol(bv)); break;
  case bv_integer : printf("%d", q4_value_get_integer(bv)); break;
  default         : printf("%lf", bv.d);
  }
}

q4_value q4_value_add(q4_value lhs, q4_value rhs) {
	lhs.d += rhs.d;
	return lhs;
}

q4_value q4_value_sub(q4_value lhs, q4_value rhs) {
	lhs.d -= rhs.d;
	return lhs;
}

q4_value q4_value_mul(q4_value lhs, q4_value rhs) {
	lhs.d *= rhs.d;
	return lhs;
}

q4_value q4_value_div(q4_value lhs, q4_value rhs) {
	lhs.d /= rhs.d;
	return lhs;
}

int q4_value_isfalse(q4_value v) {
	if (q4_value_type(v) == bv_nil) return 1;
	if (isnan(v.d)) return 0;
	return v.d == 0.0;
}
