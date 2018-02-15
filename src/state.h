#ifndef Q4_STATE_H
#define Q4_STATE_H

#include "value.h"
#include "rhhm.h"

#include <stdint.h>

enum q4_op {
	Q4_OP_NOOP = 0, /* noop                   */
	Q4_OP_LIT,      /* push lit to stack      */
	Q4_OP_STORE,    /* store variable         */
	Q4_OP_FETCH,    /* fetch variable         */
	Q4_OP_LAMBDA,   /* define callable symbol */
	Q4_OP_CALL,     /* call symbol            */
	Q4_OP_TCALL,    /* tail call symbol       */
	Q4_OP_JMP,      /* unconditional jump     */
	Q4_OP_JZ,       /* jump if zero           */
	Q4_OP_RET,      /* return                 */
	Q4_OP_CCALL     /* call c function        */
};

#define MAX_SYMBOLS 16384 /* symbol map sz */
#define PROGRAM_SZ  65536
#define STACK_SZ      256
#define RSTACK_SZ    1024

#define SYMBOL_STACK_SZ STACK_SZ
#define SYMBOL_HISTORY_SZ (SYMBOL_STACK_SZ*4)

struct q4_symbol {
  const char *name;
  q4_value value;
};

struct q4_symbol_state {
  uint32_t   symbol;
  q4_value value;
};

struct q4_state {
  struct rhhm symbol_map; /* maps names to symbol indices */
  struct q4_symbol symbols[MAX_SYMBOLS];

  struct q4_symbol_state symbol_history[SYMBOL_HISTORY_SZ];
  //struct q4_symbol_state *symbol_restore_stack[SYMBOL_STACK_SZ], ***srsp;
  uint32_t n_sym;

  uint8_t bc[PROGRAM_SZ], *wrt, *ip; /* program, write/instruction pointer */
  q4_value stack[STACK_SZ], *sp;     /* value stack  */
  uint32_t rstack[RSTACK_SZ], *rsp;  /* return stack */
  //uint32_t lstack[RSTACK_SZ], *lsp; /* loop stack */
};

int  q4_state_init(struct q4_state *s);
void q4_state_destroy(struct q4_state *s);

void q4_state_push(struct q4_state *s, q4_value v);
q4_value q4_state_pop(struct q4_state *s);

void q4_state_compile(struct q4_state *s, char *text);
void q4_state_exec(struct q4_state *s);
void q4_state_eval(struct q4_state *s, char *text);

typedef void(*q4_cfunction)(struct q4_state*);
void q4_state_register(struct q4_state *s, const char *name, q4_cfunction fn);

#endif
