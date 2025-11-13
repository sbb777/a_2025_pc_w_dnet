/*
 * APC_RemotePort.h
 *
 *  Created on: Feb 23, 2024
 *      Author: Yongseok
 */

#ifndef _APC_REMOTEPORT_H_
#define _APC_REMOTEPORT_H_

#include <APC_Define.h>

typedef enum {
    Baudrate_600        = 0,
    Baudrate_1200       = 1,
    Baudrate_2400       = 2,
    Baudrate_4800       = 3,
    Baudrate_9600       = 4,
    Baudrate_19200      = 5,
    Baudrate_38400      = 6,
    Baudrate_57600      = 7,
    Baudrate_115200     = 8,
    Baudrate_230400     = 9
} EN_Baudrate;

typedef enum {
    ParityBit_even      = 0,
    ParityBit_odd       = 1,
    ParityBit_mark      = 2,
    ParityBit_space     = 3,
    ParityBit_no        = 4
} EN_ParityBit;

typedef enum {
    DataLength_7bit     = 0,
    DataLength_8bit     = 1
} EN_DataLength;

typedef enum {
    StopBits_1      = 0,
    StopBits_2      = 1
} EN_StopBits;

typedef enum {
    CommandSet_IC       = 0,
    CommandSet_PM_V2        = 8
} EN_CommandSet;

typedef enum {
    DigitalInputOpenValve_NotInverted   = 0,
    DigitalInputOpenValve_Inverted      = 1,
    DigitalInputOpenValve_Disabled      = 2
} EN_DigitalInputOpenValve;

typedef enum {
    DigitalInputCloseValve_NotInverted  = 0,
    DigitalInputCloseValve_Inverted     = 1,
    DigitalInputCloseValve_Disabled     = 2
} EN_DigitalInputCloseValve;

////////////////////////////////////////////////////////////////
typedef enum {
    DigitalInput1Function_InterlockOpen     = 0,
    DigitalInput1Function_InterlockClose    = 1
} EN_DigitalInput1Function;

typedef enum {
    DigitalInput2Function_InterlockClose    = 0,
    DigitalInput2Function_InterlockOpen     = 1
} EN_DigitalInput2Function;

//typedef enum {
//    DigitalInputLogic_NotInverted   = 0,
//    DigitalInputLogic_Inverted      = 1
//} EN_DigitalInputLogic;

typedef enum {
    DigitalInputPolarity_NotInverted    = 0,
    DigitalInputPolarity_Inverted       = 1
} EN_DigitalInputPolarity;

typedef enum {
    DigitalInputActivation_Enabled      = 0,
    DigitalInputActivation_Disabled     = 1
} EN_DigitalInputActivation;

typedef enum {
    DigitalOutput1Function_Open         = 0,
    DigitalOutput1Function_Close        = 1,
    DigitalOutput1Function_ConstantlyON = 2
} EN_DigitalOutput1Function;

typedef enum {
    DigitalOutput2Function_Close        = 0,
    DigitalOutput2Function_Open         = 1,
    DigitalOutput2Function_ConstantlyON = 2
} EN_DigitalOutput2Function;

typedef enum {
    DigitalOutputPolarity_NotInverted   = 0,
    DigitalOutputPolarity_Inverted      = 1
} EN_DigitalOutputPolarity;

typedef enum {
    DigitalOutputActivation_Enabled     = 0,
    DigitalOutputActivation_Disabled    = 1
} EN_DigitalOutputActivation;

typedef enum {
    DigitalOutputOpenLogic_Open     = 0,
    DigitalOutputOpenLogic_Warning      = 1
} EN_DigitalOutputOpenLogic;

typedef enum {
    DigitalOutputCloseLogic_Close       = 0,
    DigitalOutputCloseLogic_Warning     = 1
} EN_DigitalOutputCloseLogic;


void initRemotePort(void);

void procRemotePort(uint8_t ch);
void CheckRemote(void);

void writeAOUT_PRESSURE(int pressure);
void writeAOUT_POSITION(int position);

void writeRELAY_CLOSE(bool option);
void writeRELAY_OPEN(bool option);
void writeSPI_CS_E2(bool option);

uint8_t readRemotePort(void);
uint8_t readCLOSE_INPUT(uint8_t input);
uint8_t readOPEN_INPUT(uint8_t input);
uint8_t readPW_MD_ON(uint8_t input);

EN_DigitalInputOpenValve getRemoteDigitalInputOpenValve();
void setRemoteDigitalInputOpenValve(EN_DigitalInputOpenValve openValve);

EN_DigitalInputCloseValve getRemoteDigitalInputCloseValve();
void setRemoteDigitalInputCloseValve(EN_DigitalInputCloseValve closeValve);

EN_DigitalInputOpenValve getRemoteDigitalOutputOpenValve();
void setRemoteDigitalOutputOpenValve(EN_DigitalInputOpenValve openValve);

EN_DigitalInputCloseValve getRemoteDigitalOutputCloseValve();
void setRemoteDigitalOutputCloseValve(EN_DigitalInputCloseValve closeValve);

uint32_t availableRemoteRS232(uint32_t ch);

uint32_t getRemoteRS232Config();
void setRemoteRS232Config(char* data);

EN_Baudrate getRemoteRS232Baudrate();
void setRemoteRS232Baudrate(EN_Baudrate baudRate);

EN_ParityBit getRemoteRS232ParityBit();
void setRemoteRS232ParityBit(EN_ParityBit parityBit);

EN_DataLength getRemoteRS232DataLength();
void setRemoteRS232DataLength(EN_DataLength length);

EN_StopBits getRemoteRS232StopBits();
void setRemoteRS232StopBits(EN_StopBits stopBits);

EN_CommandSet getRemoteRS232CommandSet();
void setRemoteRS232CommandSet(EN_CommandSet commandSet);

uint8_t readRemoteRS232Data(void);
void writeRemoteRS232Data(uint8_t *p_data, uint32_t length);

void printfRemoteRS232(const char *fmt, ...);

void refreshRemoteRS232();

/* ----------------------------------------------------------------------
    Description : MO related with "REMOTE DNET Interface"
*/
extern uint32_t     MO_DnetSystemInfo;
extern uint32_t     MO_DnetFwInfo;
extern uint32_t     MO_DnetSerialNo;
extern uint32_t     MO_DnetMacId;
extern uint32_t     MO_DnetBaudrate;
extern uint32_t     MO_DnetDatatype;
extern uint32_t     MO_DnetInputAssembly;
extern uint32_t     MO_DnetStatus;
extern uint32_t     MO_DnetExceptionStatus;

extern bool         m_bPower_md;
extern bool         m_bIsOpen;
extern bool         m_bIsClose;

#endif /* _APC_REMOTEPORT_H_ */
