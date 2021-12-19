/* Copyright (c) 2010-2012, 2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/*
 *      win32-ucontext: Unix ucontext_t operations on Windows platforms
 *      Copyright(C) 2007 Panagiotis E. Hadjidoukas
 *
 *      Contact Email: phadjido@cs.uoi.gr, xdoukas@ceid.upatras.gr
 *
 *      win32-ucontext is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 *
 *      win32-ucontext is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with SimGrid in the file LICENSE-LGPL-2.1;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */  
    
#include "xbt/win32_ucontext.h"

int getcontext(ucontext_t * ucp) 
{
  int ret;
  
      /* Retrieve the full machine context */ 
      ucp->uc_mcontext.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
  ret = GetThreadContext(GetCurrentThread(), &ucp->uc_mcontext);
  return (ret == 0) ? -1 : 0;
}

int setcontext(const ucontext_t * ucp) 
{
  int ret;
  
      /* Restore the full machine context (already set) */ 
      ret = SetThreadContext(GetCurrentThread(), &ucp->uc_mcontext);
  return (ret == 0) ? -1 : 0;
}

int makecontext(ucontext_t * ucp, void (*func) (), int argc, ...) 
{
  int i;
  va_list ap;
  char *sp;

   /* Stack grows down */
      sp = (char *) (size_t) ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size;
  
      /* Reserve stack space for the arguments (maximum possible: argc*(8 bytes per argument)) */ 
      sp -= argc * sizeof(void*);
  if (sp < (char *) ucp->uc_stack.ss_sp) {
    
        /* errno = ENOMEM; */ 
        return -1;
  }
  
      /* Set the instruction and the stack pointer */
  #ifdef _I_X86_
  ucp->uc_mcontext.Eip = (DWORD) func;
  ucp->uc_mcontext.Esp = (DWORD) sp - sizeof(void*);
  #elif defined _IA64_
  #  error "_IA64_"
  #elif defined _AMD64_
  ucp->uc_mcontext.Rip = (DWORD64) func;
  ucp->uc_mcontext.Rsp = (DWORD64) sp - sizeof(void*);
  #else
  #error "No architecture defined for Windows build. need either _AMD64_ or _I_X86_"
  #endif

      /* Save/Restore the full machine context */ 
      ucp->uc_mcontext.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
  
      /* Copy the arguments */ 
      va_start(ap, argc);
  for (i = 0; i < argc; i++) {
    memcpy(sp, ap, sizeof(void*));
    ap += sizeof(void*);
    sp += sizeof(void*);
  }
  va_end(ap);
  return 0;
}

int swapcontext(ucontext_t * oucp, const ucontext_t * ucp) 
{
  int ret;
  if ((oucp == NULL) || (ucp == NULL)) {
    
        /*errno = EINVAL; */ 
        return -1;
  }
  ret = getcontext(oucp);
  if (ret == 0) {
    ret = setcontext(ucp);
  }
  return ret;
}

