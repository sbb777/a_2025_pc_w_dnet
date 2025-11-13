/*
 * APC_PidHandler.h
 *
 *  Created on: Jan 23, 2024
 *      Author: Yongseok
 */

#ifndef _APC_PIDHANDLER_H_
#define _APC_PIDHANDLER_H_

#include <APC_Define.h>


void initPid();

bool isPidRunning();
void setPidRunState(bool bState);
bool getPidRunState();
void setPidInitState(bool bState);
bool getPidInitState();

void setSV(int _newSV);
void setPid();
void resetPid();

bool procPid();

int calculatePidOutput(int _newPV);

#endif /* _APC_PIDHANDLER_H_ */
