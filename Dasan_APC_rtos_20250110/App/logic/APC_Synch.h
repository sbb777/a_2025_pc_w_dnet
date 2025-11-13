/*
 * APC_Synch.h
 *
 *  Created on: 2023. 10. 23.
 *      Author: Yongseok
 */

#ifndef _APC_SYNCH_H_
#define _APC_SYNCH_H_

#include <APC_Define.h>

typedef enum {
    SynchMode_Full      = 0,
    SynchMode_Close     = 1,
    SynchMode_Open      = 2
} EN_SynchMode;


void initSynch(void);
uint8_t procSynch(uint8_t mode);
bool getSynchStatus(void);

void syncronizeValve(uint8_t sync_mode);

int synchronizeClosed();
int synchronizeOpen(uint8_t mode);

#endif /* _APC_SYNCH_H_ */
