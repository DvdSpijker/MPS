#ifndef MPS_CONFIG_H_
#define MPS_CONFIG_H_

#ifdef __cplusplus 
extern "C" {
#endif

/* Maximum stack size (number of layers). */
#define MPS_CONFIG_STACK_MAX_SIZE 3

/* Automatically allocate and link RX and TX queues on initialization of the stack. */
#define MPS_CONFIG_USE_AUTO_QUEUE_ALLOCATION

/* Layers' run-loop functions are called in top to bottom order. */
#define MPS_CONFIG_RUNLOOP_ORDER_TOP_TO_BOTTOM

/* Layers' run-loop functions are called in bottom to top order. */
//#define MPS_CONFIG_RUNLOOP_ORDER_BOTTOM_TO_TOP


/* Maximum amount of memory to be used by the stack.
 * Memory usage is only tracked if MpsMalloc/MpsFree functions
 * are used. */
#define MPS_CONFIG_MEMORY_SIZE_BYTES 500

#ifdef __cplusplus 
}
#endif


#endif
