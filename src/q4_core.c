#include "q4_core.h"

#include "state.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void q4_core_nil(struct q4_state *s) {
	q4_state_push(s, q4_value_make_nil());
}

void q4_core_eval(struct q4_state *s) {
	q4_value v = *--s->sp;
	char *str = strdup(q4_value_get_string(v));

	s->wrt++;

	uint32_t *end = s->wrt;
    *s->rsp++ = (uint32_t)(s->ip - s->bc);


	q4_state_compile(s, str);
	*s->wrt = Q4_OP_RET;

    s->ip = end;
	q4_state_exec(s);

	s->wrt = end;

	s->wrt--;
}

void q4_core_cons(struct q4_state *s) {
	q4_value cdr = q4_state_pop(s);
	q4_value car = q4_state_pop(s);
	q4_value cons = q4_value_make_cons(car, cdr);
	q4_value_destroy(car);
	q4_value_destroy(cdr);
	q4_state_push(s, cons);
}

void q4_core_car(struct q4_state *s) {
	q4_value v = q4_state_pop(s);
	struct q4_cons *cons = q4_value_get_cons(v);
	q4_state_push(s, cons->car);
}

void q4_core_cdr(struct q4_state *s) {
	q4_value v = q4_state_pop(s);
	struct q4_cons *cons = q4_value_get_cons(v);
	q4_state_push(s, cons->cdr);
}

void q4_core_disp(struct q4_state *s) {
	q4_value_disp(*--s->sp);
}

void q4_core_add(struct q4_state *s) {
	s->sp -= 2;
	*s->sp++ = q4_value_add(*s->sp, s->sp[1]);
}

void q4_core_sub(struct q4_state *s) {
	s->sp -= 2;
	*s->sp++ = q4_value_sub(*s->sp, s->sp[1]);
}

void q4_core_mul(struct q4_state *s) {
	s->sp -= 2;
	*s->sp++ = q4_value_mul(*s->sp, s->sp[1]);
}

void q4_core_div(struct q4_state *s) {
	s->sp -= 2;
	*s->sp++ = q4_value_div(*s->sp, s->sp[1]);
}

static char *inputString(FILE* fp, size_t size) { /* TODO: rewrite, this function is from stackoverflow */
	//The size is extended by the input with the value of the provisional
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char)*size);//size is start size
    if(!str)return str;
    while(EOF!=(ch=fgetc(fp)) && ch != '\n'){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str)return str;
        }
    }
    str[len++]='\0';

    return realloc(str, sizeof(char)*len);
}

void q4_io_readln(struct q4_state *s) {
	*s->sp++ = q4_value_make_string(inputString(stdin, 16));
}

void q4_io_write(struct q4_state *s) {

}

void q4_io_writeln(struct q4_state *s) {

}

void q4_math_sin(struct q4_state *s) {
	double val = (*--s->sp).d;
	q4_value res;
	res.d = sin(val);
	*s->sp++ = res;
}

void q4_math_cos(struct q4_state *s) {
	double val = (*--s->sp).d;
	q4_value res;
	res.d = cos(val);
	*s->sp++ = res;
}
