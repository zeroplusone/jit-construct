/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.3.0, DynASM x64 version 1.3.0
** DO NOT EDIT! The original file is in "jit-x64-copyLoops.dasc".
*/

#line 1 "jit-x64-copyLoops.dasc"
#include <stdint.h>
#include "util.h"

//|.arch x64
#if DASM_VERSION != 10300
#error "Version mismatch between DynASM and included encoding engine"
#endif
#line 5 "jit-x64-copyLoops.dasc"
//|.actionlist actions
static const unsigned char actions[75] = {
  83,72,137,252,251,255,72,252,255,195,255,72,252,255,203,255,252,254,3,255,
  252,254,11,255,15,182,59,72,184,237,237,252,255,208,255,72,184,237,237,252,
  255,208,136,3,255,138,11,255,0,139,233,255,198,3,0,255,128,59,0,15,132,245,
  249,255,128,59,0,15,133,245,249,255,91,195,255
};

#line 6 "jit-x64-copyLoops.dasc"
//|
//|// Use rbx as our cell pointer.
//|// Since rbx is a callee-save register, it will be preserved
//|// across our calls to getchar and putchar.
//|.define PTR, rbx
//|
//|// Macro for calling a function.
//|// In cases where our target is <=2**32 away we can use
//|//   | call &addr
//|// But since we don't know if it will be, we use this safe
//|// sequence instead.
//|.macro callp, addr
//|  mov64  rax, (uintptr_t)addr
//|  call   rax
//|.endmacro

#define Dst &state
#define MAX_NESTING 256

int copyLoops(char** p) 
{
    if( *((*p)+1) == '-' &&  *((*p)+2) == '>' && *((*p)+3) == '+' && *((*p)+4) == '<' && *((*p)+5) == ']' ) {
        (*p) += 5;
        return 1;
    }
    /* other patterns return 0 */
    return 0;

}

int main(int argc, char *argv[])
{
	if (argc < 2) err("Usage: jit-x64 <inputfile>");
	dasm_State *state;
	initjit(&state, actions);

	unsigned int maxpc = 0;
	int pcstack[MAX_NESTING];
	int *top = pcstack, *limit = pcstack + MAX_NESTING;
    int copyTimes=0, i;

	// Function prologue.
	//|  push PTR
	//|  mov  PTR, rdi      // rdi store 1st argument
	dasm_put(Dst, 0);
#line 50 "jit-x64-copyLoops.dasc"

	for (char *p = read_file(argv[1]); *p; p++) {
		switch (*p) {
		case '>':
			//|  inc  PTR
			dasm_put(Dst, 6);
#line 55 "jit-x64-copyLoops.dasc"
			break;
		case '<':
			//|  dec  PTR
			dasm_put(Dst, 11);
#line 58 "jit-x64-copyLoops.dasc"
			break;
		case '+':
			//|  inc  byte [PTR]
			dasm_put(Dst, 16);
#line 61 "jit-x64-copyLoops.dasc"
			break;
		case '-':
			//|  dec  byte [PTR]
			dasm_put(Dst, 20);
#line 64 "jit-x64-copyLoops.dasc"
			break;
		case '.':
			//|  movzx edi, byte [PTR]
			//|  callp putchar
			dasm_put(Dst, 24, (unsigned int)((uintptr_t)putchar), (unsigned int)(((uintptr_t)putchar)>>32));
#line 68 "jit-x64-copyLoops.dasc"
			break;
		case ',':
			//|  callp getchar
			//|  mov   byte [PTR], al
			dasm_put(Dst, 35, (unsigned int)((uintptr_t)getchar), (unsigned int)(((uintptr_t)getchar)>>32));
#line 72 "jit-x64-copyLoops.dasc"
			break;
		case '[':
			if (top == limit) err("Nesting too deep.");
			// Each loop gets two pclabels: at the beginning and end.
			// We store pclabel offsets in a stack to link the loop
			// begin and end together.
            copyTimes=copyLoops(&p);
            if(copyTimes > 0) {
                //|  mov cl, byte [PTR]
                dasm_put(Dst, 45);
#line 81 "jit-x64-copyLoops.dasc"
                for(i = 1; i <= copyTimes; ++i) { 
                    //|  add byte [PTR+i], cl
                    dasm_put(Dst, 48, i);
#line 83 "jit-x64-copyLoops.dasc"
                }
                //|  mov byte [PTR], 0
                dasm_put(Dst, 52);
#line 85 "jit-x64-copyLoops.dasc"
		    } else {
			    maxpc += 2;
			    *top++ = maxpc;
			    dasm_growpc(&state, maxpc);
			    //|  cmp  byte [PTR], 0
			    //|  je   =>(maxpc-2)
			    //|=>(maxpc-1):
			    dasm_put(Dst, 56, (maxpc-2), (maxpc-1));
#line 92 "jit-x64-copyLoops.dasc"
			}
            break;
		case ']':
           	if (top == pcstack) err("Unmatched ']'");
	        top--;
			//|  cmp  byte [PTR], 0
			//|  jne  =>(*top-1)
			//|=>(*top-2):
			dasm_put(Dst, 64, (*top-1), (*top-2));
#line 100 "jit-x64-copyLoops.dasc"
			break;
		}
	}

	// Function epilogue.
	//|  pop  PTR
	//|  ret
	dasm_put(Dst, 72);
#line 107 "jit-x64-copyLoops.dasc"

	void (*fptr)(char*) = jitcode(&state);
	char *mem = calloc(30000, 1);
	fptr(mem);
	free(mem);
	free_jitcode(fptr);
	return 0;
}
