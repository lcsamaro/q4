#include "state.h"
#include "rhhm.h"
#include "q4_core.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {

	struct q4_state s;
	if (q4_state_init(&s)) return 1;
	q4_state_register(&s, "nil", q4_core_nil);
	q4_state_register(&s, "eval", q4_core_eval);
	q4_state_register(&s, "cons", q4_core_cons);
	q4_state_register(&s, "car", q4_core_car);
	q4_state_register(&s, "cdr", q4_core_cdr);

	q4_state_register(&s, ".", q4_core_disp);
	q4_state_register(&s, "+", q4_core_add);
	q4_state_register(&s, "-", q4_core_sub);
	q4_state_register(&s, "*", q4_core_mul);
	q4_state_register(&s, "/", q4_core_div);

	q4_state_register(&s, "readln", q4_io_readln);

	q4_state_register(&s, "sin", q4_math_sin);
	q4_state_register(&s, "cos", q4_math_cos);

	char code[] = "			                                        \
							                                      \n\
; core  							                              \n\
[ 1 + ] !inc                                                      \n\
[ 1 - ] !dec                                                      \n\
                                                                  \n\
                                                                  \n\
; stack 							                              \n\
[ !a ] !drop				                                      \n\
[ !ax @ax @ax ] !dup			                                  \n\
[ !ay !by @ay @by ] !swap		                                  \n\
[ !a !b @b @a @b ] !over	                                      \n\
[ !c !b !a @b @c @a ] !rot                                        \n\
[ !c !b !a @c @a @b ] !-rot                                       \n\
                                                                  \n\
[ swap drop ] !nip		  	                                      \n\
[ swap over ] !tuck		  	                                      \n\
                                                                  \n\
                                                                  \n\
; combinators                                                     \n\
[ !f !t _jz _pm t _ret _wm f ] !if                                \n\
[ !t _jz _pm t _ret _wm ] !if-true                                \n\
[ !f _jz _pm _ret _wm f ] !if-false                               \n\
                                                                  \n\
[ swap !t do @t ] !dip  ; x quot - x                              \n\
[ over !t do @t ] !keep ; x quot - x                              \n\
                                                                  \n\
                                                                  \n\
[ dup car ] !dcar                                                 \n\
[ dup cdr ] !dcdr                                                 \n\
                                                                  \n\
[ !fop !s !c @c                                                   \n\
    [ @c car @s fop @c cdr swap @fop foldl ] [ @s ] if ] !foldl   \n\
                                                                  \n\
                                                                  \n\
                                                                  \n\
                                                                  \n\
                                                                  \n\
[ !a a ] !do			  	                                      \n\
                                                                  \n\
[ !fb !fa dup fa swap fb ] !bi                                    \n\
[ !fc !fb !fa ] !tri                                              \n\
                                                                  \n\
[ !q !i @i [ q @i dec @q loop ] if-true ] !loop                   \n\
                                                                  \n\
                                                                  \n\
                                                                  \n\
[ dup [ 1 - fac * ] [ drop 1 ] if ] !fac                          \n\
                                                                  \n\
                                                                  \n\
; REPL                                                            \n\
\"q4\" .                                                          \n\
[ \"\n>\" . readln eval repl ] !repl repl                         \n\
                                                                  \n\
                                                                  \n\
                                                                  \n\
";

	q4_state_eval(&s, strdup(code));

	q4_state_destroy(&s);

	fgetc(stdin);

	return 0;
}
