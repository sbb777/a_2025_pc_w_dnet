/*
 * APC_Test.h
 *
 *  Created on: Mar 6, 2024
 *      Author: Yongseok
 */

#ifndef _APC_TEST_H_
#define _APC_TEST_H_

#include <APC_Define.h>


void initTest(void);
void procTest(uint64_t job_val);
void setTest(uint16_t ratio, uint16_t msec, uint16_t count);

#endif /* _APC_TEST_H_ */
