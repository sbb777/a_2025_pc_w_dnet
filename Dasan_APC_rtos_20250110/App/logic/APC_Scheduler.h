/*
 * APS_Scheduler.h
 *
 *  Created on: 2023. 10. 6.
 *      Author: Yongseok
 */

#ifndef _APC_SCHEDULER_H_
#define _APC_SCHEDULER_H_

#include <APC_Define.h>

//#define SCHEDULE_POWER      _DEF_CH1
//#define SCHEDULE_SENSOR     _DEF_CH2
//#define SCHEDULE_VALVE      _DEF_CH3
//#define SCHEDULE_AIR        _DEF_CH4

void initScheduler(void);
void procScheduler(uint32_t cur_time);

void scheduleInterrupt(uint32_t cur_time);
void scheduleDisplay(uint32_t cur_time);
void schedulePower(uint32_t cur_time);
void scheduleSensor(uint32_t cur_time);
void scheduleValve(uint32_t cur_time);
void scheduleAir(uint32_t cur_time);
void scheduleRemotePort(uint32_t cur_time);
void scheduleSyncData(uint32_t cur_time);

#endif /* _APC_SCHEDULER_H_ */
