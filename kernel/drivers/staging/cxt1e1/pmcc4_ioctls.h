

#ifndef _INC_PMCC4_IOCTLS_H_
#define _INC_PMCC4_IOCTLS_H_


#include "sbew_ioc.h"

enum
{
    // C4_GET_PORT = 0,
    // C4_SET_PORT,
    // C4_GET_CHAN,
    // C4_SET_CHAN,
    C4_DEL_CHAN = 0,
    // C4_CREATE_CHAN,
    // C4_GET_CHAN_STATS,
    // C4_RESET,
    // C4_DEBUG,
    C4_RESET_STATS,
    C4_LOOP_PORT,
    C4_RW_FRMR,
    C4_RW_MSYC,
    C4_RW_PLD
};

#define C4_GET_PORT          SBE_IOC_PORT_GET
#define C4_SET_PORT          SBE_IOC_PORT_SET
#define C4_GET_CHAN          SBE_IOC_CHAN_GET
#define C4_SET_CHAN          SBE_IOC_CHAN_SET
// #define C4_DEL_CHAN          XXX
#define C4_CREATE_CHAN       SBE_IOC_CHAN_NEW
#define C4_GET_CHAN_STATS    SBE_IOC_CHAN_GET_STAT
#define C4_RESET             SBE_IOC_RESET_DEV
#define C4_DEBUG             SBE_IOC_LOGLEVEL
// #define C4_RESET_STATS       XXX
// #define C4_LOOP_PORT         XXX
// #define C4_RW_FRMR           XXX
// #define C4_RW_MSYC           XXX
// #define C4_RW_PLD            XXX

struct c4_chan_stats_wrap
{
    int         channum;
    struct sbecom_chan_stats stats;
};

#endif   /* _INC_PMCC4_IOCTLS_H_ */
