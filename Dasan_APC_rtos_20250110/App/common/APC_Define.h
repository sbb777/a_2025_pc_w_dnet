/*
 * APC_Define.h
 *
 *  Created on: Jul 12, 2023
 *      Author: Yongseok
 */

#ifndef _APC_DEFINE_H_
#define _APC_DEFINE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"


/***************************************************/
#define     CMD_PROCESSING_OK       0

#define     CMD_PARSING_OK          0
#define     CMD_PARSING_NOK         -1
/***************************************************/

#define APC_HIGH        true
#define APC_LOW         false

#define _DEF_CH1		0
#define _DEF_CH2		1
#define _DEF_CH3		2
#define _DEF_CH4        3
#define _DEF_CH5        4
#define _DEF_CH6        5
#define _DEF_CH7        6
#define _DEF_CH8        7

#define APC_PORT_INTERNAL   0
#define APC_PORT_LOCAL	    1
#define APC_PORT_SENSOR     2
#define APC_PORT_REMOTE     3

#define PERCENT     100
#define PERMILL     1000

#define _ADDR_MO_DATA     0x08080000  // Sector 8 (128KB)
#define _ADDR_LEARN_DATA  0x080A0000  // Sector 9 (128KB)

#define _USE_THREAD
#define _USE_REMOTE_RS232
#define __REV_MTECH__

// XXX only for exhibition
//#define _USE_EXHIBITION_MODE

/*********************************************************************************/
#define APC_BUILD_YEAR ( __DATE__[7] == '?' ? 1900 : \
    ((__DATE__[7]-'0') * 1000 + (__DATE__[8]-'0') * 100 \
    + (__DATE__[9] - '0') * 10 + __DATE__[10] - '0') )

#define APC_BUILD_MONTH ( __DATE__ [2] == '?' ? 1 \
    : __DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6) \
    : __DATE__ [2] == 'b' ? 2 \
    : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
    : __DATE__ [2] == 'y' ? 5 \
    : __DATE__ [2] == 'l' ? 7 \
    : __DATE__ [2] == 'g' ? 8 \
    : __DATE__ [2] == 'p' ? 9 \
    : __DATE__ [2] == 't' ? 10 \
    : __DATE__ [2] == 'v' ? 11 \
    : 12)

#define APC_BUILD_DAY ( __DATE__[4] == '?' ? 1 \
    : ((__DATE__[4] == ' ' ? 0 : \
    ((__DATE__[4] - '0') * 10)) + (__DATE__[5] - '0')) )

#define APC_BUILD_HOUR ((__TIME__[0]-'0')*10 + (__TIME__[1]-'0'))
#define APC_BUILD_MIN  ((__TIME__[3]-'0')*10 + (__TIME__[4]-'0'))
#define APC_BUILD_SEC  ((__TIME__[6]-'0')*10 + (__TIME__[7]-'0'))


#endif /* _APC_DEFINE_H_ */
