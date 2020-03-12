#pragma once

#include "util/config.h"


//
// preempt_t : State for your timer/preemption handler
//
// Note:
//  We've one instance of core_t per each core.
//  To access this instance, you need to use %gs:0
//  (The entire kernel doesn't have any global/static variables)
//
// %gs:core_offset_preempt will point to start of preempt_t instance
//
// for example: 
// %gs:0 will return pointer to core_t
// %gs:core_offset_mainstack will return core_t::main_stack
// %gs:core_offset_preempt+0 will return core_t::saved_stack
//
// etc.
//
// See Definition of core_t in x86/main.h
//

struct preempt_t{
  // your data structure, if any
  int curr_idx;
  addr_t saved_stack[6]; //feel free to change it - provided as an example
  int yielding;
};


//
// 
// This macro is being called from x86/except.cc
//
//
// _name: label name
// _f   : C function to be called 
//        ex: we may have to do send EOI to LAPIC or PIC etc.
//
#  define  _ring0_preempt(_name,_f)            \
  __asm(                                       \
      "  .text                                      \n\t"\
      " " STR(_name) ":                             \n\t"\
      "  pushl %edx                                 \n\t"\
      "  pushl %ecx                                 \n\t"\
      "  pushl %eax                                 \n\t"\
      "  call " STR(_f) "                           \n\t"\
      "  popl  %eax                                 \n\t"\
      "  popl  %ecx                                 \n\t"\
      "  popl  %edx                                 \n\t"\
      "  cmp $1, %gs:" STR(core_offset_preempt + 28) "   \n\t"\
      "  sti                                        \n\t"\
      "  je iret_toring0                           \n\t"\
      "  pushl %eax                                 \n\t"\
      "  pushl %ebx                                 \n\t"\
      "  pushl %ecx                                 \n\t"\
      "  pushl %edx                                 \n\t"\
      "  pushl %ebp                                 \n\t"\
      "  pushl %edi                                 \n\t"\
      "  pushl %esi                                 \n\t"\
      "  movl %esp, %ebp                            \n\t"\
      "  sub $512, %esp                              \n\t"\
      "  and $0xfffffff0, %esp                        \n\t"\
      "  fxsave (%esp)                              \n\t"\
      "  pushl %ebp                                 \n\t"\
      "  pushl $1f                                  \n\t"\
      "  movl %gs:" STR(core_offset_preempt) ", %eax   \n\t"\
      "  cmp $0, %eax                                   \n\t"\
      "  je zero                                    \n\t"\
       "  cmp $1, %eax                                   \n\t"\
      "  je one                                    \n\t"\
      "  cmp $2, %eax                                   \n\t"\
      "  je two                                    \n\t"\
      "  cmp $3, %eax                                   \n\t"\
      "  je three                                    \n\t"\
      "  cmp $4, %eax                                   \n\t"\
      "  je four                                    \n\t"\
      "  cmp $5, %eax                                   \n\t"\
      "  je five                                    \n\t"\
      "  zero: movl %esp, %gs:" STR(core_offset_preempt + 4) "   \n\t"\
      "  jmp continue \n\t"\
      "  one: movl %esp, %gs:" STR(core_offset_preempt + 8) "   \n\t"\
      "  jmp continue \n\t"\
      "  two: movl %esp, %gs:" STR(core_offset_preempt + 12) "   \n\t"\
      "  jmp continue \n\t"\
      "  three: movl %esp, %gs:" STR(core_offset_preempt + 16) "   \n\t"\
      "  jmp continue \n\t"\
      "  four: movl %esp, %gs:" STR(core_offset_preempt + 20) "   \n\t"\
      "  jmp continue \n\t"\
      "  five: movl %esp, %gs:" STR(core_offset_preempt + 24) "   \n\t"\
      "  jmp continue \n\t"\
      "  continue: movl %gs:" STR(core_offset_mainstack) ", %esp   \n\t"\
      "  sti                                        \n\t"\
      "  ret                                        \n\t"\
      "1:                                           \n\t"\
      "  popl %ebp                                  \n\t"\
      "  fxrstor (%esp)                              \n\t"\
      "  movl %ebp, %esp                            \n\t"\
      "  popl %esi                                   \n\t"\
      "  popl %edi                                   \n\t"\
      "  popl %ebp                                   \n\t"\
      "  popl %edx                                   \n\t"\
      "  popl %ecx                                   \n\t"\
      "  popl %ebx                                   \n\t"\
      "  popl %eax                                   \n\t"\
      "  sti                                        \n\t"\
      "  jmp iret_toring0                           \n\t"\
      )                                        \


