#include "labs/fiber.h"

static int int2string(char* input_string, int n) {
  int length = 0;
  while(n > 0) {
    input_string[10 - (++length)] = '0' + (n % 10);
    n /= 10;
  }
  return length;
}

void fiber(addr_t* pmain_stack, addr_t* pf_stack, int* pret, bool* pdone, int* num) {
  hoh_debug("Beginning");
  addr_t& main_stack = *pmain_stack; // boilerplate: to ease the transition from existing code
  addr_t& f_stack    = *pf_stack;
  int& ret           = *pret;
  bool& done         = *pdone;
  
  int i, j, k;

  for(i = 0; i < *num; i++) {
      for(j = 0; j < *num; j++) {
          for(k = 0; k < *num; k++) {
              hoh_debug("Looping");
              ret = (2 * ret) % 10000007;
              done=false; 
          }
      }
  }

  // Computation done -- return
  for(;;) {
    hoh_debug("Done");
    done = true;
  }
}

void shell_step_fiber(shellstate_t& shellstate, addr_t& main_stack, preempt_t& preempt, addr_t& f_stack, addr_t f_array, uint32_t f_arraysize, dev_lapic_t& lapic){
  // Make the state machine here
  if (shellstate.fiber_state == 0) {
    if (shellstate.fiber_req == 1) {
      hoh_debug("Request signal recvd");
      // Setup the stack and coroutine
      stack_init5(preempt.saved_stack, f_array, f_arraysize, &fiber, &main_stack, &preempt.saved_stack, &shellstate.fiber_ret, &shellstate.fiber_done, &shellstate.fiber_arg);
      // Change the state
      shellstate.fiber_req = 0;
      shellstate.fiber_state = 1;
      shellstate.fiber_ret = 1;
    } 
  } else if (shellstate.fiber_state == 1) {
    // We are busy right now
    if (shellstate.fiber_done) {
      // Change the state to done
      shellstate.fiber_state = 2;
    } else {
      // Set the timer
      hoh_debug("Setting the timer");
      lapic.reset_timer_count(100);
      stack_saverestore(main_stack,preempt.saved_stack);
      lapic.reset_timer_count(0);
      hoh_debug("Stopping the timer");
    }
  } else if (shellstate.fiber_state == 2) {
    hoh_debug("We are in the done state");
    // Change the state and print the output
    shellstate.fiber_done = false;
    shellstate.fiber_state = 0;
    char ans_string[10];
    int length = int2string(ans_string, shellstate.fiber_ret);

    // Print the answer string to the terminal
    for(int i = 10 - length; i < 10; i++) {
        shellstate.contents[shellstate.content_ptr][i - 10 + length] = ans_string[i];
    }
    for(int i = length; i < 80; i++) {
        shellstate.contents[shellstate.content_ptr][i] = ' ';
    }
    shellstate.content_ptr++;
  }
}

