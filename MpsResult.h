#ifndef MPS_RESULT_H_
#define MPS_RESULT_H_

typedef enum {
    MPS_RESULT_INVALID_STATE    = -5,
    MPS_RESULT_INVALID_ARG      = -4,
    MPS_RESULT_NO_MEM           = -3,
    MPS_RESULT_NO_SPACE         = -2,
    MPS_RESULT_ERROR            = -1,
    MPS_RESULT_OK               = 0,
    MPS_RESULT_FAIL             = 1,
    MPS_RESULT_TIMEOUT          = 2,
    MPS_RESULT_BUSY             = 3,
} MpsResult_t;


#endif /* MPS_RESULT_H_ */