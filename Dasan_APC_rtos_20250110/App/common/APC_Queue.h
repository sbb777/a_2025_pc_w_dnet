/*
 * APC_Queue.h
 *
 *  Created on: 2023. 11. 4.
 *      Author: Yongseok
 */

#ifndef _APC_QUEUE_H_
#define _APC_QUEUE_H_

#include <APC_Define.h>

typedef struct
{
  uint32_t in;
  uint32_t out;
  uint32_t len;

  uint8_t *p_buf;
} ApcQueueType;


bool     createQueue(ApcQueueType *p_node, uint8_t *p_buf, uint32_t size);
bool     pushQueue(ApcQueueType *p_node, uint8_t *p_data, uint32_t length);
bool     popQueue(ApcQueueType *p_node, uint8_t *p_data, uint32_t length);
uint32_t availableQueue(ApcQueueType *p_node);
void     flushQueue(ApcQueueType *p_node);


#endif /* _APC_QUEUE_H_ */
