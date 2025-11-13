/*
 * APC_Queue.c
 *
 *  Created on: 2023. 11. 4.
 *      Author: Yongseok
 */


#include "APC_Queue.h"


bool createQueue(ApcQueueType *p_node, uint8_t *p_buf, uint32_t size)
{
    p_node->in    = 0;
    p_node->out   = 0;
    p_node->len   = size;
    p_node->p_buf = p_buf;

    return true;
}

bool pushQueue(ApcQueueType *p_node, uint8_t *p_data, uint32_t length)
{
    uint32_t next_in;

    for (int i=0; i<length; i++)
    {
        next_in = (p_node->in + 1) % p_node->len;

        if (next_in != p_node->out)
        {
            if (p_node->p_buf != NULL)
            {
                p_node->p_buf[p_node->in] = p_data[i];
            }
            p_node->in = next_in;
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool popQueue(ApcQueueType *p_node, uint8_t *p_data, uint32_t length)
{
    for (int i=0; i<length; i++)
    {
        if (p_node->p_buf != NULL)
        {
            p_data[i] = p_node->p_buf[p_node->out];
        }

        if (p_node->out != p_node->in)
        {
            p_node->out = (p_node->out + 1) % p_node->len;
        }
        else
        {
            return false;
        }
    }
    return true;
}

uint32_t availableQueue(ApcQueueType *p_node)
{
    return (p_node->in - p_node->out) % p_node->len;
}

void flushQueue(ApcQueueType *p_node)
{
    p_node->in  = 0;
    p_node->out = 0;
}
