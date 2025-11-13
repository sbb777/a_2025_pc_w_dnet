/*
 * APC_CmdHandler.h
 *
 *  Created on: Sep 13, 2023
 *      Author: Yongseok
 */
#ifndef _APC_CMDHANDLER_H
#define _APC_CMDHANDLER_H

#include <APC_Define.h>

#define     CMD_KEY_SHIFT_COUNT     18

#define     CMD_KEY_C_CLOSE_VALVE                   ('C'<< CMD_KEY_SHIFT_COUNT)
#define     CMD_KEY_O_OPEN_VALVE                    ('O'<< CMD_KEY_SHIFT_COUNT)
#define     CMD_KEY_H_HOLD_VALVE                    ('H'<< CMD_KEY_SHIFT_COUNT)
#define     CMD_KEY_R_SETPOINT_VALVE_POSITION       ('R'<< CMD_KEY_SHIFT_COUNT)
#define     CMD_KEY_r00_SETPOINT_VALVE_POSITION     (('r'<< CMD_KEY_SHIFT_COUNT) + 00) // added 2024-01-29
#define     CMD_KEY_r01_SETPOINT_VALVE_POSITION     (('r'<< CMD_KEY_SHIFT_COUNT) + 01) // added 2024-01-29

#define     CMD_KEY_S_SETPOINT_VALVE_PRESSURE       ('S'<< CMD_KEY_SHIFT_COUNT)
#define     CMD_KEY_A_GET_VALVE_POSITION            ('A'<< CMD_KEY_SHIFT_COUNT)
#define     CMD_KEY_P_GET_SENSOR_PRESSURE           ('P'<< CMD_KEY_SHIFT_COUNT)

#define     CMD_KEY_i01_GET_SENSOR_CONFIG           (('i'<< CMD_KEY_SHIFT_COUNT) + 1)
#define     CMD_KEY_i02_GET_CONTROLLER_CONFIG       (('i'<< CMD_KEY_SHIFT_COUNT) + 2)   // Not used
#define     CMD_KEY_i02_GET_PRESSURE_CONTROLLER     (('i'<< CMD_KEY_SHIFT_COUNT) + 22)  // Not used
#define     CMD_KEY_i04_GET_VALVE_CONFIG            (('i'<< CMD_KEY_SHIFT_COUNT) + 4)
#define     CMD_KEY_i05_GET_SENSOR_SCALE            (('i'<< CMD_KEY_SHIFT_COUNT) + 5)
#define     CMD_KEY_i20_GET_INTERFACE_CONFIG        (('i'<< CMD_KEY_SHIFT_COUNT) + 20)
#define     CMD_KEY_i21_GET_COMM_RANGE              (('i'<< CMD_KEY_SHIFT_COUNT) + 21)
#define     CMD_KEY_i30_GET_DEVICE_STATUS           (('i'<< CMD_KEY_SHIFT_COUNT) + 30)
#define     CMD_KEY_i32_GET_LEARN_STATUS            (('i'<< CMD_KEY_SHIFT_COUNT) + 32)
#define     CMD_KEY_i34_GET_LEARN_PRESSLIMIT        (('i'<< CMD_KEY_SHIFT_COUNT) + 34)
#define     CMD_KEY_i38_GETPOINT_VALVE_POSITION     (('i'<< CMD_KEY_SHIFT_COUNT) + 38)
#define     CMD_KEY_i39_GET_ENCODER_VALUE           (('i'<< CMD_KEY_SHIFT_COUNT) + 39)
//#define     CMD_KEY_i38_GETPOINT_VALVE_PRESSURE     (('i'<< CMD_KEY_SHIFT_COUNT) + 38)
#define     CMD_KEY_i50_GET_ERROR_STATUS            (('i'<< CMD_KEY_SHIFT_COUNT) + 50)
#define     CMD_KEY_i51_GET_WARNING_STATUS          (('i'<< CMD_KEY_SHIFT_COUNT) + 51)
#define     CMD_KEY_i60_GET_SENSOR1_OFFSET          (('i'<< CMD_KEY_SHIFT_COUNT) + 60)
#define     CMD_KEY_i61_GET_SENSOR2_OFFSET          (('i'<< CMD_KEY_SHIFT_COUNT) + 61)
#define     CMD_KEY_i64_GET_SENSOR1_VALUE           (('i'<< CMD_KEY_SHIFT_COUNT) + 64)
#define     CMD_KEY_i65_GET_SENSOR2_VALUE           (('i'<< CMD_KEY_SHIFT_COUNT) + 65)
#define     CMD_KEY_i68_GET_VALVE_SPEED             (('i'<< CMD_KEY_SHIFT_COUNT) + 68)
#define     CMD_KEY_i70_GET_CONTROL_CYCLECNT        (('i'<< CMD_KEY_SHIFT_COUNT) + 70)
#define     CMD_KEY_i71_GET_ISOLATION_CYCLECNT      (('i'<< CMD_KEY_SHIFT_COUNT) + 71)
#define     CMD_KEY_i72_GET_POWERUP_COUNTER         (('i'<< CMD_KEY_SHIFT_COUNT) + 72)
#define     CMD_KEY_i76_GET_VALVE_MAINSTATUS        (('i'<< CMD_KEY_SHIFT_COUNT) + 76)
#define     CMD_KEY_i80_GET_HW_CONFIG               (('i'<< CMD_KEY_SHIFT_COUNT) + 80)
#define     CMD_KEY_i82_GET_FW_VERSION              (('i'<< CMD_KEY_SHIFT_COUNT) + 82)
#define     CMD_KEY_i83_GET_VALVE_SERIALNO          (('i'<< CMD_KEY_SHIFT_COUNT) + 83)
#define     CMD_KEY_i84_GET_FW_NUMBER               (('i'<< CMD_KEY_SHIFT_COUNT) + 84)

#define     CMD_KEY_c01_SET_ACCESS_MODE             (('c'<< CMD_KEY_SHIFT_COUNT) + 1)
#define     CMD_KEY_c82_RESET_ERROR                 (('c'<< CMD_KEY_SHIFT_COUNT) + 82)
//#define     CMD_KEY_c99_RESET_CLOSE                 (('c'<< CMD_KEY_SHIFT_COUNT) + 99)  // added 2024-01-25

#define     CMD_KEY_s01_SET_SENSOR_CONFIG           (('s'<< CMD_KEY_SHIFT_COUNT) + 1)
//#define     CMD_KEY_s02_SET_CONTROLLER_CONFIG       (('s'<< CMD_KEY_SHIFT_COUNT) + 2)   // Not used
//#define     CMD_KEY_s02_SET_PRESSURE_CONTROLLER     (('s'<< CMD_KEY_SHIFT_COUNT) + 22)  // Not used
#define     CMD_KEY_s04_SET_VALVE_CONFIG            (('s'<< CMD_KEY_SHIFT_COUNT) + 4)
#define     CMD_KEY_s05_SET_SENSOR_SCALE            (('s'<< CMD_KEY_SHIFT_COUNT) + 5)
#define     CMD_KEY_s20_SET_INTERFACE_CONFIG        (('s'<< CMD_KEY_SHIFT_COUNT) + 20)
#define     CMD_KEY_s21_SET_COMM_RANGE              (('s'<< CMD_KEY_SHIFT_COUNT) + 21)

#define     CMD_KEY_Z_SET_ZERO_ADJUST               ('Z'<< CMD_KEY_SHIFT_COUNT)
#define     CMD_KEY_L_START_LEARN                   ('L'<< CMD_KEY_SHIFT_COUNT)
#define     CMD_KEY_d_DOWNLOAD_LEARN_DATA           ('d'<< CMD_KEY_SHIFT_COUNT)
#define     CMD_KEY_u_UPLOAD_LEARN_DATA             ('u'<< CMD_KEY_SHIFT_COUNT)
#define     CMD_KEY_V_SET_VALVE_SPEED               ('V'<< CMD_KEY_SHIFT_COUNT)


// defined for DAM
#define     CMD_KEY_q00_GET_PRE_POS_VAL             (('q'<< CMD_KEY_SHIFT_COUNT) + 0)
#define     CMD_KEY_q01_GET_HW_CONFIG_ALL           (('q'<< CMD_KEY_SHIFT_COUNT) + 1)
#define     CMD_KEY_q02_GET_SW_CONFIG_ALL           (('q'<< CMD_KEY_SHIFT_COUNT) + 2)
#define     CMD_KEY_q03_GET_SENSOR_STATUS_ALL       (('q'<< CMD_KEY_SHIFT_COUNT) + 3)
#define     CMD_KEY_q04_GET_VALVE_CONFIGALL         (('q'<< CMD_KEY_SHIFT_COUNT) + 4)
#define     CMD_KEY_q05_GET_CYCLECNT_ALL            (('q'<< CMD_KEY_SHIFT_COUNT) + 5)
#define     CMD_KEY_q06_GET_PFO_INFOALL             (('q'<< CMD_KEY_SHIFT_COUNT) + 6)
#define     CMD_KEY_q07_GET_COMPAIR_INFOALL         (('q'<< CMD_KEY_SHIFT_COUNT) + 7)
#define     CMD_KEY_q08_GET_VALVE_ERRORALL          (('q'<< CMD_KEY_SHIFT_COUNT) + 8)
#define     CMD_KEY_q09_GET_VALVE_HWINFO            (('q'<< CMD_KEY_SHIFT_COUNT) + 9)
#define     CMD_KEY_q10_GET_DAM_MAIN_INFORM         (('q'<< CMD_KEY_SHIFT_COUNT) + 10)
#define     CMD_KEY_q11_GET_POS_PR_VAL              (('q'<< CMD_KEY_SHIFT_COUNT) + 11)
#define     CMD_KEY_q20_GET_SENSOR_CONFIGALL        (('q'<< CMD_KEY_SHIFT_COUNT) + 20)
#define     CMD_KEY_q21_GET_SENSOR_OFFSETALL        (('q'<< CMD_KEY_SHIFT_COUNT) + 21)
#define     CMD_KEY_q22_GET_LEARN_INFOALL           (('q'<< CMD_KEY_SHIFT_COUNT) + 22)
#define     CMD_KEY_q30_GET_ACCESS_MODE             (('q'<< CMD_KEY_SHIFT_COUNT) + 30)
#define     CMD_KEY_q40_GET_INTERFACE_CONFIG        (('q'<< CMD_KEY_SHIFT_COUNT) + 40)
#define     CMD_KEY_q70_GET_CONTROL_CYCLECNT        (('q'<< CMD_KEY_SHIFT_COUNT) + 70)
#define     CMD_KEY_q71_GET_ISOLATION_CYCLECNT      (('q'<< CMD_KEY_SHIFT_COUNT) + 71)
#define     CMD_KEY_q72_GET_POWERUP_COUNTER         (('q'<< CMD_KEY_SHIFT_COUNT) + 72)
#define     CMD_KEY_q80_GET_FIXED_CONTROLLER_CONFIG (('q'<< CMD_KEY_SHIFT_COUNT) + 80)
#define     CMD_KEY_q81_GET_ADAPTIVE_CONTROLLER_CONFIG (('q'<< CMD_KEY_SHIFT_COUNT) + 81)

#define     CMD_KEY_z01_START_INTERFACE_TRACE       (('z'<< CMD_KEY_SHIFT_COUNT) + 1)
#define     CMD_KEY_z02_STOP_INTERFACE_TRACE        (('z'<< CMD_KEY_SHIFT_COUNT) + 2)
#define     CMD_KEY_z06_SET_PFO_STATE               (('z'<< CMD_KEY_SHIFT_COUNT) + 6)
#define     CMD_KEY_z20_SET_SENSOR_CONFIGALL        (('z'<< CMD_KEY_SHIFT_COUNT) + 20)
#define     CMD_KEY_z22_SET_LEARN_INFOALL           (('z'<< CMD_KEY_SHIFT_COUNT) + 22)
#define     CMD_KEY_z30_SET_ACCESS_MODE             (('z'<< CMD_KEY_SHIFT_COUNT) + 30)
#define     CMD_KEY_z40_SET_INTERFACE_CONFIG        (('z'<< CMD_KEY_SHIFT_COUNT) + 40)
#define     CMD_KEY_z70_RESET_CONTROL_CYCLECNT      (('z'<< CMD_KEY_SHIFT_COUNT) + 70)
#define     CMD_KEY_z71_RESET_ISOLATION_CYCLECNT    (('z'<< CMD_KEY_SHIFT_COUNT) + 71)
#define     CMD_KEY_z72_RESET_POWERUP_COUNTER       (('z'<< CMD_KEY_SHIFT_COUNT) + 72)
#define     CMD_KEY_z80_SET_FIXED_CONTROLLER_CONFIG (('z'<< CMD_KEY_SHIFT_COUNT) + 80)
#define     CMD_KEY_z81_SET_ADAPTIVE_CONTROLLER_CONFIG (('z'<< CMD_KEY_SHIFT_COUNT) + 81)

#define     CMD_KEY_n01_NOTI_INTERFACE_TRACE        (('n'<< CMD_KEY_SHIFT_COUNT) + 1)

//#define     CMD_KEY_TXX_TEST_CONTROLLER             ('T'<< CMD_KEY_SHIFT_COUNT)     // added 2024-01-05
#define     CMD_KEY_t00_TEST_CONTROLLER             (('t'<< CMD_KEY_SHIFT_COUNT) + 0)     // added 2024-03-06
#define     CMD_KEY_t01_TEST_CONTROLLER             (('t'<< CMD_KEY_SHIFT_COUNT) + 1)

#define     CMD_KEY_Y_SYNC_VALVE                    ('Y'<< CMD_KEY_SHIFT_COUNT)     // added 2024-02-08
#define     CMD_KEY_y00_GET_FW_FACTORY_STATE        (('y'<< CMD_KEY_SHIFT_COUNT) + 0)   // added 2024-03-15
#define     CMD_KEY_y01_SET_FW_FACTORY_STATE        (('y'<< CMD_KEY_SHIFT_COUNT) + 1)
#define     CMD_KEY_y02_RESTART_FW                  (('y'<< CMD_KEY_SHIFT_COUNT) + 2)
#define     CMD_KEY_y03_SYNC_MO_DATA                (('y'<< CMD_KEY_SHIFT_COUNT) + 3)

#define     CMD_KEY_a10_VALVE_CONFIG_SETUP          (('a'<< CMD_KEY_SHIFT_COUNT) + 10)
#define     CMD_KEY_a99_VALVE_CONFIG_START_STOP     (('a'<< CMD_KEY_SHIFT_COUNT) + 99)


int doCmdHandler(uint8_t channel, char* recvCmdData, char* respCmdData);
int getCmdKey(char* cmd);
int doCmdParser(char* recvData, char* cmd, char* data);


#endif
