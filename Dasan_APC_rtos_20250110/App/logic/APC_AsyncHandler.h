/*
 * APC_AsyncHandler.h
 *
 *  Created on: Jan 30, 2024
 *      Author: Yongseok
 */

#ifndef _APC_ASYNCHANDLER_H_
#define _APC_ASYNCHANDLER_H_

#include <APC_Define.h>

//typedef struct QueueData_ {
//    char command[8];
//    int  index;
//} QueueData;

typedef enum {
    DATA_SYNC_ERASE         = 0,
    DATA_SYNC_PART          = 1,
    DATA_SYNC_FULL          = 2
} Data_Sync_Type;


bool isJobExisted();
bool setJobData(uint32_t job_id, uint64_t job_val);

int  putDataIntoMQ(uint32_t data);

void procAsyncHandler(uint32_t counter);

bool setDataSync(uint32_t addr, uint32_t option);
bool setRestartFw(uint32_t option);
bool setCountSync(uint32_t addr, uint32_t length);


#endif /* _APC_ASYNCHANDLER_H_ */
