/*
 * APC_AccessMode.h
 *
 *  Created on: Oct 24, 2023
 *      Author: Yongseok
 */

#ifndef _APC_ACCESSMODE_H_
#define _APC_ACCESSMODE_H_

#include <APC_Define.h>


#define LOCAL_LIFE_TIME (1 * 60 * 1000)    // 1min

typedef enum {
    AccessMode_Local        = 0,
    AccessMode_Remote       = 1,
    AccessMode_Locked       = 2
} EN_AccessMode;


bool initAccessMode(void);

void refreshAccessMode(uint8_t ch, uint32_t counter);

uint8_t setAccessMode(uint8_t ch, uint8_t mode);
uint8_t getAccessMode(void);



#endif /* _APC_ACCESSMODE_H_ */
