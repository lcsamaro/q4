#include "lexer.h"
#include "state.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

int q4_state_init(struct q4_state *s) {
  s->wrt = s->bc;
  s->sp  = s->stack;
  s->rsp = s->rstack;

  s->n_sym = 0;
  return rhhm_init(&s->symbol_map, MAX_SYMBOLS);
}

void q4_state_destroy(struct q4_state *s) {
  rhhm_destroy(&s->symbol_map);
}

void q4_state_push(struct q4_state *s, q4_value v) {
  *s->sp++ = v;
}

q4_value q4_state_pop(struct q4_state *s) {
  return *--s->sp;
}

//void q4_state_rpush(struct q4_state *s, q4_value v) {
//  *s->sp++ = v;
//}
//
//q4_value q4_state_rpop(struct q4_state *s) {
//  return *--s->sp;
//}

void q4_state_exec(struct q4_state *s) {
  uint8_t dbg;
  while (1) {
    switch (dbg = *s->ip++) {
    case Q4_OP_LIT:
      memcpy(s->sp++, s->ip, sizeof(q4_value));
      s->ip += sizeof(q4_value);
      break;
    case Q4_OP_STORE:
      s->sp -= 2;
      s->symbols[q4_value_get_integer(s->sp[1])].value = s->sp[0];
      break;
    case Q4_OP_FETCH: {
      s->sp--;
      int loc = q4_value_get_integer(*s->sp);
      *s->sp++ = s->symbols[loc].value;
      } break;
    case Q4_OP_JMP: { /* operand: addr (32) */
      uint32_t loc;
      memcpy(&loc, s->ip, sizeof(uint32_t));
      s->ip = s->bc + loc;
      } break;
    case Q4_OP_JZ: { /* operand: addr (32) */
      q4_value v = (*--s->sp);
      if (q4_value_isfalse(v)) {
        uint32_t loc;
        memcpy(&loc, s->ip, sizeof(uint32_t));
        s->ip = s->bc + loc;
      } else {
        s->ip += sizeof(uint32_t);
      }
      } break;
	case Q4_OP_TCALL: {
	  q4_symbol sym = q4_value_get_symbol(*(--s->sp));
      uint32_t loc = q4_value_get_integer(s->symbols[sym].value); /* todo: get quote instead of integer */
      s->ip = s->bc + loc;
	  } break;
    case Q4_OP_CALL: { /* no operand, uses top of stack */
      // push current symbols state
      q4_symbol sym = q4_value_get_symbol(*(--s->sp));
      uint32_t loc = q4_value_get_integer(s->symbols[sym].value); /* todo: get quote instead of integer */
      *s->rsp++ = (uint32_t)(s->ip - s->bc);
      s->ip = s->bc + loc;
      } break;
    case Q4_OP_RET:
      // pop current symbols state
      s->ip = s->bc + *--s->rsp;
      break;
    case Q4_OP_CCALL: {
      q4_cfunction f;
      memcpy(&f, s->ip, sizeof(q4_cfunction));
      s->ip += sizeof(q4_cfunction);
      f(s);
      } break;
    default: /*printf("exiting: %d", dbg);*/ return;
    }
  }
}

/* vm eval */
#define MAX_COMPILER_MARKS 128
#define Q4_COMPILER_HERE ((uint32_t)(c->s->wrt - c->s->bc))
struct q4_compiler {
  uint8_t *marks[MAX_COMPILER_MARKS], **cmark;
  uint8_t lastop;
  struct q4_state *s;
};

static int q4_compiler_init(struct q4_compiler *c, struct q4_state *s) {
  s->ip = s->wrt;

  c->s = s;
  c->cmark = c->marks;
  c->lastop = Q4_OP_NOOP;
  return 0;
};

static void q4_compiler_emit_op(struct q4_compiler *c, uint8_t v) {
  *c->s->wrt++ = v;
  c->lastop = v;
}

static void q4_compiler_emit_boxed_lit(struct q4_compiler *c, q4_value v) {
  q4_compiler_emit_op(c, Q4_OP_LIT);
  memcpy(c->s->wrt, &v, sizeof(q4_value));
  c->s->wrt += sizeof(q4_value);
}

static void q4_compiler_emit_cfunction(struct q4_compiler *c, q4_cfunction fn) {
  memcpy(c->s->wrt, &fn, sizeof(q4_cfunction));
  c->s->wrt += sizeof(q4_cfunction);
}

static uint32_t q4_compiler_get_symbol(struct q4_compiler *c, const char *t) {
  int32_t r;
  if ((r = rhhm_get(&c->s->symbol_map, t)) == -1)
    rhhm_insert(&c->s->symbol_map, t, r = c->s->n_sym++);
  return r;
}

static void q4_compiler_mark(struct q4_compiler *c) {
  //printf("mark: %d\n", Q4_COMPILER_HERE);
  *c->cmark++ = c->s->wrt;
  c->s->wrt += sizeof(uint32_t);
}

static uint32_t q4_compiler_get_mark(struct q4_compiler *c) {
  return c->cmark[-1] - c->s->bc;
}

static void q4_compiler_write_mark(struct q4_compiler *c) {
  uint32_t l = Q4_COMPILER_HERE;
  memcpy(*--c->cmark, &l, sizeof(uint32_t));
  //printf("write mark: %d\n", *c->cmark - c->s->bc);
}

static void q4_compiler_emit_ret(struct q4_compiler *c) {
  //if (c->lastop == Q4_OP_CALL && c->s->wrt[-1] == Q4_OP_CALL) /* TODO: better handling with marks and jumps */
	//  c->s->wrt[-1] = Q4_OP_TCALL;
  //else
	  *c->s->wrt++ = Q4_OP_RET;
}

void lex_callback(const char *t, int tt, void *context) {
  struct q4_compiler *c = context;

  switch (tt) {
  case Q4_TOKEN_WORD:
    /* prefix handling*/
    switch(*t) {
    case '_': /* intrinsics */
      if (!strcmp(t+1, "pm")) {
        q4_compiler_mark(c);
      } else if (!strcmp(t+1, "wm")) {
        q4_compiler_write_mark(c);
      } else if (!strcmp(t+1, "jmp")) {
        q4_compiler_emit_op(c, Q4_OP_JMP);
      } else if (!strcmp(t+1, "jz")) {
        q4_compiler_emit_op(c, Q4_OP_JZ);
      } else if (!strcmp(t+1, "ret")) {
		q4_compiler_emit_ret(c);
      }
      break;
    case '\'':
      q4_compiler_emit_boxed_lit(c, q4_value_make_symbol(q4_compiler_get_symbol(c, t + 1)));
      break;
    case '!':
	  if (t[1]) {
        q4_symbol sym = q4_compiler_get_symbol(c, t + 1);
        q4_compiler_emit_boxed_lit(c, q4_value_make_symbol(sym));
	  }
      q4_compiler_emit_op(c, Q4_OP_STORE);
      break;
    case '@':
	  if (t[1]) {
        q4_symbol sym = q4_compiler_get_symbol(c, t + 1);
        q4_compiler_emit_boxed_lit(c, q4_value_make_symbol(sym));
	  }
      q4_compiler_emit_op(c, Q4_OP_FETCH);
      break;
    default:
      if (isdigit(*t) || (*t == '-' && isdigit(t[1]))) { /* number */
        /* TODO: integer types */
        q4_value v;
        v.d = strtod(t, NULL);
        q4_compiler_emit_boxed_lit(c, v);
      } else { /* word */
        q4_symbol sym = q4_compiler_get_symbol(c, t);
        q4_compiler_emit_boxed_lit(c, q4_value_make_symbol(sym));
        q4_compiler_emit_op(c, Q4_OP_CALL);
        //printf("emit sym call: %d\n", sym);
      }
    }
    break;
  case Q4_TOKEN_STRING: {
	  q4_value v = q4_value_make_string(t); // todo: strdup
      q4_compiler_emit_boxed_lit(c, v);
    } break;
  case Q4_TOKEN_LBOX: /* begin quote */
    /* jmp to end of current quote */
    q4_compiler_emit_op(c, Q4_OP_JMP);
    q4_compiler_mark(c);
    break;
  case Q4_TOKEN_RBOX: { /* end quote */
    uint32_t mark = q4_compiler_get_mark(c);
    q4_compiler_emit_ret(c);
    /* fills the jmp address at the beginning of the quote to HERE */
    q4_compiler_write_mark(c);
    /* push current quote addr to stack */
    //printf("emit quote: %d\n", mark + sizeof(uint32_t));
    q4_compiler_emit_boxed_lit(c, q4_value_make_quote(
    mark + sizeof(uint32_t)));
    } break;
  case Q4_TOKEN_LROUND: /* begin cons */
    break;
  case Q4_TOKEN_RROUND: /* end cons */
    break;
  default:
    break;
  }
}

void dump_bc(struct q4_state *s) {
  uint8_t *c = s->bc;
  puts("\nbc dump ---------------------------------begin-");
  while (c != s->wrt) {
    printf("%02x ", *c++);
    if ((c - s->bc) % 16 == 0) printf("\n");
  }
  puts("\nbc dump -----------------------------------end-");
}

void q4_state_compile(struct q4_state *s, char *text) {
  struct q4_compiler c;
  if (q4_compiler_init(&c, s)) return;
  q4_lex(text, lex_callback, &c);
  *s->wrt = Q4_OP_NOOP;
}

void q4_state_eval(struct q4_state *s, char *text) {
  //uint8_t *ip = s->wrt;
  //q4_state_compile(s, text);
  //s->ip = ip;
  //q4_state_exec(s);
  struct q4_compiler c;
  if (q4_compiler_init(&c, s)) return;

  uint8_t *ip = s->wrt;

  q4_lex(text, lex_callback, &c);
  *s->wrt = Q4_OP_NOOP;
  //dump_bc(s);

  s->ip = ip;

  q4_state_exec(s);
}

void q4_state_register(struct q4_state *s, const char *name, q4_cfunction fn) {
  struct q4_compiler c;
  if (q4_compiler_init(&c, s)) return;
  uint32_t sym = q4_compiler_get_symbol(&c, name);
  s->symbols[sym].value = q4_value_make_integer(s->wrt - s->bc);
  q4_compiler_emit_op(&c, Q4_OP_CCALL);
  q4_compiler_emit_cfunction(&c, fn);
  q4_compiler_emit_op(&c, Q4_OP_RET);
}
