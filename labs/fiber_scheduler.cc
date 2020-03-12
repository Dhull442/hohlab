#include "labs/fiber_scheduler.h"
#include "util/fiber.h"

//
// stackptrs:      Type: addr_t[stackptrs_size].  array of stack pointers (generalizing: main_stack and f_stack)
// stackptrs_size: number of elements in 'stacks'.
//
// arrays:      Type: uint8_t [arrays_size]. array of memory region for stacks (generalizing: f_array)
// arrays_size: size of 'arrays'. equal to stackptrs_size*STACK_SIZE.
//
// Tip: divide arrays into stackptrs_size parts.
// Tip: you may implement a circular buffer using arrays inside shellstate_t
//      if you choose linked lists, create linked linked using arrays in
//      shellstate_t. (use array indexes as next pointers)
// Note: malloc/new is not available at this point.
//

static int int2string(char* input_string, int n) {
  int length = 0;
  while(n > 0) {
    input_string[10 - (++length)] = '0' + (n % 10);
    n /= 10;
  }
  return length;
}

// Function which will work based on the timer interrupt
void h(addr_t* pmain_stack, addr_t* pf_stack, int* pret, bool* pdone, int* num) {
  addr_t& main_stack = *pmain_stack; // boilerplate: to ease the transition from existing code
  addr_t& f_stack    = *pf_stack;
  int& ret           = *pret;
  bool& done         = *pdone;

  int i, j, k;

  for(i = 0; i < *num; i++) {
      for(j = 0; j < *num; j++) {
          for(k = 0; k < *num; k++) {
              ret = (2 * ret) % 10000007;
              done=false; 
              stack_saverestore(f_stack, main_stack);
          }
      }
  }

  // Computation done -- return
  for(;;) {
    done = true;
    stack_saverestore(f_stack, main_stack);
  }
}

void recfib(addr_t* pmain_stack, addr_t* pf_stack, int* pret, bool* pdone, int* num) {
    addr_t& main_stack = *pmain_stack; // boilerplate: to ease the transition from existing code
    addr_t& f_stack    = *pf_stack;
    int& ret           = *pret;
    bool& done         = *pdone;

    if (*num <= 2) {
        ret = 1;
    } else {
        int a = *num - 1;
        int b = *num - 2;
        int retval;
        recfib(pmain_stack, pf_stack, &retval, pdone, &a);
        ret = retval;
        done = false;
        stack_saverestore(f_stack, main_stack);
        recfib(pmain_stack, pf_stack, &retval, pdone, &b);
        ret += retval;
        done = false;
        stack_saverestore(f_stack, main_stack);
    }
}

void g(addr_t* pmain_stack, addr_t* pf_stack, int* pret, bool* pdone, int* num) {
    addr_t& main_stack = *pmain_stack; // boilerplate: to ease the transition from existing code
    addr_t& f_stack    = *pf_stack;
    int& ret           = *pret;
    bool& done         = *pdone;

    if (*num <= 2) {
        ret = 1;
    } else {
        int a = *num - 1;
        int b = *num - 2;
        int retval;
        recfib(pmain_stack, pf_stack, &retval, pdone, &a);
        ret = retval;
        done = false;
        stack_saverestore(f_stack, main_stack);
        recfib(pmain_stack, pf_stack, &retval, pdone, &b);
        ret += retval;
        done = false;
        stack_saverestore(f_stack, main_stack);
    }


    // Computation done -- return
    for(;;) {
        done = true;
        stack_saverestore(f_stack, main_stack);
    }
}

void shell_step_fiber_scheduler(shellstate_t& shellstate, addr_t& main_stack, preempt_t& preempt, addr_t stackptrs[], size_t stackptrs_size, addr_t arrays, size_t arrays_size, dev_lapic_t& lapic){
    size_t thread_array_size = arrays_size / stackptrs_size;
    
    // Check the request signals for h and g resp
    if (shellstate.h_req == 1) {
        // Spawn a new process for function h
        shellstate.num_h++;
        int free_idx = -1;

        // Find a free slot to do the computation
        for(int i = 0; i < 5; i++) {
            if (!shellstate.thread_occupied[i]) {
                free_idx = i; 
                break;
            }
        }
        
        shellstate.thread_type[free_idx] = false;
        shellstate.thread_occupied[free_idx] = true;
        shellstate.thread_args[free_idx] = shellstate.h_arg;
        shellstate.thread_done[free_idx] = false;
        shellstate.thread_ret[free_idx] = 1;
        shellstate.h_req = 0;
        
        // Initialize the stack here
        stack_init5(stackptrs[free_idx + 1], arrays + (free_idx + 1) * thread_array_size, thread_array_size ,&h, &stackptrs[0], &stackptrs[free_idx + 1], &shellstate.thread_ret[free_idx], &shellstate.thread_done[free_idx], &shellstate.thread_args[free_idx]);
    } else if (shellstate.g_req == 1) {
        // Spawn a new process for function g
        shellstate.num_g++;
        int free_idx = -1;

        // Find a free slot to do the computation
        for(int i = 0; i < 5; i++) {
            if (!shellstate.thread_occupied[i]) {
                free_idx = i; 
                break;
            }
        }
        
        shellstate.thread_occupied[free_idx] = true;
        shellstate.thread_type[free_idx] = true;
        shellstate.thread_args[free_idx] = shellstate.g_arg;
        shellstate.thread_done[free_idx] = false;
        shellstate.g_req = 0;
        
        // Initialize the stack here
        stack_init5(stackptrs[free_idx + 1], arrays + (free_idx + 1) * thread_array_size, thread_array_size ,&g, &stackptrs[0], &stackptrs[free_idx + 1], &shellstate.thread_ret[free_idx], &shellstate.thread_done[free_idx], &shellstate.thread_args[free_idx]);
    }

    // Execute the current function
    if (shellstate.thread_occupied[shellstate.curr_idx]) {
        stack_saverestore(stackptrs[0], stackptrs[shellstate.curr_idx + 1]);

        // If this function has returned, print the result and free the slot
        if (shellstate.thread_done[shellstate.curr_idx]) {
            // Free the slot
            shellstate.thread_done[shellstate.curr_idx] = false;
            shellstate.thread_occupied[shellstate.curr_idx] = false;
            if (shellstate.thread_type[shellstate.curr_idx]) shellstate.num_g--;
            else shellstate.num_h--;

            // Print the result
            char ans_string[10];
            int length = int2string(ans_string, shellstate.thread_ret[shellstate.curr_idx]);

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

    // Move to the next function
    shellstate.curr_idx = (shellstate.curr_idx + 1) % 5;
}
