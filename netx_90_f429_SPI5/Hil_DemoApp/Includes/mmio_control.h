
#ifndef MMIO_CONTROL_H_
#define MMIO_CONTROL_H_

#include "cifXUser.h"
#include <stdbool.h>

// Copied from App_DemoApplication.c
#define DRV_DIO_LINE_MMIO 1

#pragma pack(push, 1)
typedef struct DIO_SET_LINE_CONFIG_REQ_Ttag {
    CIFX_PACKET_HEADER tHeader;
    uint32_t ulLine;
    uint32_t ulType;
    uint32_t ulFlags;
} DIO_SET_LINE_CONFIG_REQ;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct DIO_SET_LINE_STATE_REQ_Ttag {
    CIFX_PACKET_HEADER tHeader;
    uint32_t ulLine;
    uint32_t ulType;
    uint32_t ulState;
} DIO_SET_LINE_STATE_REQ;
#pragma pack(pop)

int32_t SetMMIO7AsOutput(CIFXHANDLE hChannel);
int32_t SetMMIO7State(CIFXHANDLE hChannel, bool fHigh);

#endif // MMIO_CONTROL_H_
