#include "labs/coroutine.h"

static int int2string(char* input_string, int n) {
  int length = 0;
  while(n > 0) {
    input_string[10 - (++length)] = '0' + (n % 10);
    n /= 10;
  }
  return length;
}

void shell_step_coroutine(shellstate_t& shellstate, coroutine_t& f_coro, f_t& f_locals){
    if (!shellstate.coro_active) {
        f_locals.i = 0;
        f_locals.ans = 1;
    } else {
        // Do a step of the computation and increment
        int mod = 10000007;
        if (f_locals.i == shellstate.coro_arg * shellstate.coro_arg * shellstate.coro_arg) {
            // We are done -- print the result and deactivate the coroutine
            shellstate.coro_active = false;
            char ans_string[10];
            int length = int2string(ans_string, f_locals.ans);
            // Print the answer string to the terminal
            for(int i = 10 - length; i < 10; i++) {
                shellstate.contents[shellstate.content_ptr][i - 10 + length] = ans_string[i];
            }
            for(int i = length; i < 80; i++) {
                shellstate.contents[shellstate.content_ptr][i] = ' ';
            }
            shellstate.content_ptr++;
            return; 
        }
        f_locals.i++;
        f_locals.ans = (2 * f_locals.ans) % mod;
    }
}


