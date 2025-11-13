/*
 * utils.c
 *
 *  Created on: Jul 13, 2023
 *      Author: Yongseok
 */
#include <APC_Utils.h>

#include <unistd.h>
#include <time.h>
#include <math.h>

#ifdef _USE_THREAD
    #include "cmsis_os2.h"
#endif

static char _alphaNumeric[36] = { '0','1','2','3','4','5','6','7','8','9',
        'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
        'P','Q','R','S','T','U','V','W','X','Y','Z' };

void APC_Delay(uint32_t time_ms)
{
#ifdef _USE_THREAD
    osDelay(time_ms);
#else
	HAL_Delay(time_ms);
#endif
}

uint32_t millis(void)
{
	return HAL_GetTick();
}

bool isString(const char *p_arg, const char *p_str)
{
	if (strcmp(p_arg, p_str) == 0)
		return true;
	else
		return false;
}

void floatToByte(float f, uint8_t *bytes)
{
    int length = sizeof(float);

    for(int i = 0; i < length; i++){
        bytes[i] = ((uint8_t *)&f)[i];
    }
}

void intToByte(int in, uint8_t *bytes)
{
    int length = sizeof(int);

    for(int i = 0; i < length; i++){
        bytes[i] = ((uint8_t *)&in)[i];
    }
}

void uint32ToByte(uint32_t in, uint8_t *bytes)
{
    int length = sizeof(uint32_t);

    for(int i = 0; i < length; i++){
        bytes[i] = ((uint8_t *)&in)[i];
    }
}

void uint16ToByte(uint16_t in, uint8_t *bytes)
{
    int length = sizeof(uint16_t);

    for(int i = 0; i < length; i++){
        bytes[i] = ((uint8_t *)&in)[i];
    }
}

char getAlphaNumeric(uint8_t index) {
    return _alphaNumeric[index];
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* old utils */
int gLogLevel = LL_DEBUG;


// Set log level
int  setMyLogLevel(int logLevel) {
    gLogLevel = logLevel;
    return gLogLevel;
}

// Get log level set
int  getMyLogLevel(void) {
    return gLogLevel;
}

// Get now date string
void getCurrentDateStr(char* dateStr) {
    int year = ( __DATE__[7] == '?' ? 1900 : \
        ((__DATE__[7]-'0') * 1000 + (__DATE__[8]-'0') * 100 \
        + (__DATE__[9] - '0') * 10 + __DATE__[10] - '0') );
    int month = ( __DATE__ [2] == '?' ? 1 \
            : __DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6) \
            : __DATE__ [2] == 'b' ? 2 \
            : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
            : __DATE__ [2] == 'y' ? 5 \
            : __DATE__ [2] == 'l' ? 7 \
            : __DATE__ [2] == 'g' ? 8 \
            : __DATE__ [2] == 'p' ? 9 \
            : __DATE__ [2] == 't' ? 10 \
            : __DATE__ [2] == 'v' ? 11 \
            : 12);
    int day = ( __DATE__[4] == '?' ? 1 \
            : ((__DATE__[4] == ' ' ? 0 : \
            ((__DATE__[4] - '0') * 10)) + (__DATE__[5] - '0')) );
    int hour = ((__TIME__[0]-'0')*10 + (__TIME__[1]-'0'));
    int min  = ((__TIME__[3]-'0')*10 + (__TIME__[4]-'0'));
    int sec  = ((__TIME__[3]-'0')*10 + (__TIME__[7]-'0'));
    sprintf(dateStr, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, min, sec);

//    struct tm  *t;
//    time_t now = time(NULL);    // 초 단위로 현재 시간을 구함
//    t = localtime(&now);
//    sprintf(dateStr, "%02d-%02d-%02d %02d:%02d:%02d",
//                     (t->tm_year +1900)-2000, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    return;
}

// Print log data
void prtLog(const int fd, const short logLevel, const char* funcName, const char* fmt, ...) {
    char myLogBuf[256];
    char levelStr[16];
    char dateStr[32];
    va_list arg;

    memset(myLogBuf, 0x00, sizeof(myLogBuf));
    memset(levelStr, 0x00, sizeof(levelStr));
    memset(dateStr,  0x00, sizeof(dateStr));

    // Get current time & make date string
    getCurrentDateStr(dateStr);

    va_start(arg, fmt);
    switch(logLevel) {
        case LL_ERROR :
                strcpy(levelStr, "ERR"); break;
        case LL_WARN :
                strcpy(levelStr, "WAN"); break;
        case LL_INFO :
                strcpy(levelStr, "INF"); break;
        case LL_DEBUG :
                strcpy(levelStr, "DBG"); break;
        default :
                strcpy(levelStr, "???"); break;
    }

    // Build log output Format
    int length = 0;
    if(gLogLevel >= logLevel) {
        if( fd <= 0) {
            length = sprintf(myLogBuf, LOG_FORMAT, dateStr, funcName, levelStr);
            length = vsprintf(myLogBuf+length, fmt, arg);
            printf("%s", myLogBuf);
        }
        else {
            length = sprintf(myLogBuf, LOG_FORMAT, dateStr, funcName, levelStr);
            length = vsprintf(myLogBuf+length, fmt, arg);
            write(fd, myLogBuf, sizeof(myLogBuf));
        }
        va_end(arg);
    }
    return;
}


/** --------------------------------------------------------------------
 * Description : 해당 String에서 특정 char를 삭제한다
 * @return void
 */
void deleteChar(char* _srcStr, char _delChar) {
    for (; *_srcStr != '\0'; _srcStr++) {
        if (*_srcStr == _delChar) {
            strcpy(_srcStr, _srcStr + 1);
            _srcStr--;
        }
    }
}


/** --------------------------------------------------------------------
 * Description : APC String을 원하는 형식의 integer 값으로 변환
 * @param  *_srcStr  Source string
 * @param  *_destVal Converted integer value
 * @param  _length  Converted length
 * @return void
 */
void convString2Integer(char* _srcStr, int *_destVal, int _length) {
    //int tempInt = 0;
    char zFormat[16] = {""};
    sprintf(zFormat, "%%0%dd", _length);
    sscanf(_srcStr, zFormat, _destVal);
}


/** --------------------------------------------------------------------
 * Description : APC String을 원하는 형식의 float 값으로 변환
 * @param  *_srcStr  Source string
 * @param  *_destVal Converted float value
 * @param  _length   Converted length
 * @param  _expInt   Range Config 자릿수
 * @return void
 */
void convString2Float(char* _srcStr, float *_destVal, int _length, int _expInt) {
    int tempInt = 0;
    char zFormat[16] = {""};
    sprintf(zFormat, "%%0%dd", _length);
    sscanf(_srcStr, zFormat, &tempInt);
    *_destVal = (float) ((tempInt * 1.0) / pow(10.0, (_expInt*1.0)));
}

/** --------------------------------------------------------------------
 * Description : APC String을 원하는 형식의 double 값으로 변환
 * @param  *_srcStr  Source string
 * @param  *_destVal Converted double value
 * @param  _length  Converted length
 * @param  _expInt  Range Config 자릿수
 * @return void
 */
void convString2Double(char* _srcStr, double *_destVal, int _length, int _expInt) {
    int tempInt = 0;
    char zFormat[16] = {""};
    sprintf(zFormat, "%%0%dd", _length);
    sscanf(_srcStr, zFormat, &tempInt);
    *_destVal = (double) ((tempInt * 1.0) / pow(10.0, (_expInt*1.0)));
}


/** --------------------------------------------------------------------
 * Description : APC Integer를 String 으로 변환
 * @param  _destStr  Converted string
 * @param  _srcVal   Source int value
 * @param  _length   Converted string length
 * @return void
 */
void convInteger2String(char* _destStr, int _srcVal, int _length) {
    char zFormat[16] = {""};
    sprintf(zFormat, "%%0%dd", _length);
    sprintf(_destStr, zFormat, _srcVal);

}

/** --------------------------------------------------------------------
 * Description : APC Float를 String 으로 변환
 * @param  _destStr  Converted string
 * @param  _srcVal   Source float value
 * @param  _length   Converted string length
 * @param  _expInt   Range Config 자릿수
 * @return void
 */
void convFloat2String(char* _destStr, float _srcVal, int _length, int _expInt) {
    char zFormat[16] = {""};
    sprintf(zFormat, "%%0%d.%df", _length+1, _expInt);
    sprintf(_destStr, zFormat, _srcVal);
    deleteChar(_destStr, '.');

}

/** --------------------------------------------------------------------
 * Description : APC Double을 String 으로 변환
 * @param  _destStr  Converted string
 * @param  _srcVal   Source double value
 * @param  _length   Converted string length
 * @param  _expInt   Range Config 자릿수
 * @return void
 */
void convDouble2String(char* _destStr, double _srcVal, int _length, int _expInt) {
    char zFormat[16] = {""};
    sprintf(zFormat, "%%0%d.%dlf", _length+1, _expInt);
    sprintf(_destStr, zFormat, _srcVal);
    deleteChar(_destStr, '.');

}

/** --------------------------------------------------------------------
 * Description : APC Float을 Hex String 으로 변환
 * @param  _destStr  Converted string
 * @param  _srcVal   Source float value
 * @param  _length   Converted string length
 * @return void
 */
void convFloat2HexString(char* _destStr, float _srcVal, int _length) {
    union _HexConvert zConv;
    zConv.f_val = _srcVal;
    char zFormat[16] = {""};

    sprintf(zFormat, "%%0%dX", _length);
    sprintf(_destStr, zFormat, zConv.u_val);

}
