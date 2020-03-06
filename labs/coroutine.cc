#include "labs/coroutine.h"

static int int2string(char* input_string, int n) {
  int length = 0;
  while(n > 0) {
    input_string[10 - (++length)] = '0' + (n % 10);
    n /= 10;
  }
  return length;
}

// Function to do the computation
void f(coroutine_t* pf_coro,f_t* pf_locals,int* pret,bool* pdone) {
    coroutine_t& f_coro = *pf_coro; // boilerplate: to ease the transition from existing code
    int& ret            = *pret;
    bool& done          = *pdone;

    int& i              = pf_locals->i;
    int& j              = pf_locals->j;
    int& k              = pf_locals->k;
    int& num            = pf_locals->num;

    h_begin(f_coro);

    hoh_debug("Begin loop");

    for(i = 0; i < num; i++) {
        for(j = 0; j < num; j++) {
            for(k = 0; k < num; k++) {
                ret = (2 * ret) % 10000007;
                done=false; 
                h_yield(f_coro); // yield (i*j, false)
            }
        }
    }

    done = true; 
    h_end(f_coro);
}

void shell_step_coroutine(shellstate_t& shellstate, coroutine_t& f_coro, f_t& f_locals){
    // Make the state machine here
    if (shellstate.f_state == 0) {
        if (shellstate.f_req == 1) {
            // Setup the coroutine
            coroutine_reset(f_coro);

            f_locals.i = 0;
            f_locals.j = 0;
            f_locals.k = 0;
            f_locals.num = shellstate.f_arg;
            shellstate.f_ret = 1;

            // Change the request and state
            shellstate.f_req = 0;
            shellstate.f_state = 1;
        }
    } else if (shellstate.f_state == 1) {
        // We are busy right now
        if (shellstate.f_done) {
            hoh_debug("Done");
            // Change the state and print the command to the output
            shellstate.f_state = 2;
        } else {
            // Call the coroutine again
            hoh_debug("Calling coro");
            f(&f_coro, &f_locals, &shellstate.f_ret, &shellstate.f_done);
        }
    } else if (shellstate.f_state == 2) {
        hoh_debug("Printing output");

        // Change the state and print the output
        shellstate.f_done  = false;
        shellstate.f_state = 0;
        char ans_string[10];
        int length = int2string(ans_string, shellstate.f_ret);

        // Print the answer string to the terminal
        for(int i = 10 - length; i < 10; i++) {
            shellstate.contents[shellstate.content_ptr][i - 10 + length] = ans_string[i];
        }
        for(int i = length; i < 80; i++) {
            shellstate.contents[shellstate.content_ptr][i] = ' ';
        }
        shellstate.content_ptr++;

        // Go back to the ready state
        hoh_debug("Going back to ready state");
        shellstate.f_state = 0;
    }
}


