#ifndef MPS_CONFIG_H_
#define MPS_CONFIG_H_

/* Maximum stack size (number of layers). */
#define MPS_CONFIG_STACK_MAX_SIZE 3

/* Layers' run-loop functions are called in top to bottom order. */
#define MPS_CONFIG_RUNLOOP_ORDER_TOP_TO_BOTTOM

/* Layers' run-loop functions are called in bottom to top order. */
//#define MPS_CONFIG_RUNLOOP_ORDER_BOTTOM_TO_TOP


#define MPS_CONFIG_USE_PRIOR_RTOS

#define MPS_CONFIG_USE_STD_MALLOC

/* Maximum amount of memory to be used by the stack.
 * Memory usage is only tracked if MpsMalloc/MpsFree functions
 * are used. */
#define MPS_CONFIG_MEMORY_SIZE_BYTES 500


#endif
