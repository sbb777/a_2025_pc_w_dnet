/*
 * APC_Utils.h
 *
 *  Created on: Jul 13, 2023
 *      Author: Yongseok
 */

#ifndef _APC_UTILS_H_
#define _APC_UTILS_H_

#include <APC_Define.h>

// Log Level
#define  LL_ERROR  1
#define  LL_WARN   2
#define  LL_INFO   3
#define  LL_DEBUG  4

// [date][function][loglevel] : msg
#define  LOG_FORMAT  "[%-17s][%-20s] [%-3s]: "


union _HexConvert {
    float    f_val;
    uint32_t u_val;
};

void APC_Delay(uint32_t time_ms);
uint32_t millis(void);
bool isString(const char *p_arg, const char *p_str);

void floatToByte(float f, uint8_t *bytes);
void intToByte(int in, uint8_t *bytes);
void uint32ToByte(uint32_t in, uint8_t *bytes);
void uint16ToByte(uint16_t in, uint8_t *bytes);

char getAlphaNumeric(uint8_t index);


int  setMyLogLevel(int logLevel);
int  getMyLogLevel(void);
void getCurrentDateStr(char* dateStr) ;
void prtLog(int fd, short logLevel, const char* funcName, const char* fmt, ...);

// Data conversion 관련 Functions
extern void deleteChar(char* _srcStr, char _delChar);

extern void convString2Integer (char* _srcStr, int *_destVal, int _length);
extern void convString2Float   (char* _srcStr, float *_destVal, int length, int _expInt);
extern void convString2Double  (char* _srcStr, double *_destVal, int _length, int _expInt);

extern void convInteger2String (char* _destStr, int _srcVal, int _length);
extern void convFloat2String   (char* _destStr, float _srcVal, int _length, int _expInt);
extern void convDouble2String  (char* _destStr, double _srcVal, int _length, int _expInt);
extern void convFloat2HexString(char* _destStr, float _srcVal, int _length);

#endif /* _APC_UTILS_H_ */
