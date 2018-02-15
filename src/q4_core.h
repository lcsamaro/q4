#ifndef Q4_CORE_H
#define Q4_CORE_H

struct q4_state;

void q4_core_nil(struct q4_state *s);
void q4_core_eval(struct q4_state *s);
void q4_core_cons(struct q4_state *s);
void q4_core_car(struct q4_state *s);
void q4_core_cdr(struct q4_state *s);

void q4_core_disp(struct q4_state *s);

void q4_core_add(struct q4_state *s);
void q4_core_sub(struct q4_state *s);
void q4_core_mul(struct q4_state *s);
void q4_core_div(struct q4_state *s);

void q4_io_readln(struct q4_state *s);
void q4_io_write(struct q4_state *s);
void q4_io_writeln(struct q4_state *s);

void q4_math_sin(struct q4_state *s);
void q4_math_cos(struct q4_state *s);

#endif
