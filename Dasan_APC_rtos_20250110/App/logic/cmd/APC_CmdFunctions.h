/*
 * APC_CmdFunctions.h
 *
 *  Created on: Sep 13, 2023
 *      Author: Yongseok
 */
#ifndef _APC_CMDFUNCTIONS_H
#define _APC_CMDFUNCTIONS_H

#include <APC_Define.h>

int extractCmdData(char* cmd, char* srcData, uint32_t dataLen[], char(*destValueArr)[64]);

int cmds_C_CLOSE_VALVE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_O_OPEN_VALVE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_H_HOLD_VALVE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_R_SETPOINT_VALVE_POSITION(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_rXX_SETPOINT_VALVE_POSITION(uint8_t ch, char* cmd, char* data, char* respData);

int cmds_i38_GETPOINT_VALVE_POSITION(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i39_GET_ENCODER_VALUE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_S_SETPOINT_VALVE_PRESSURE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i38_GETPOINT_VALVE_PRESSURE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_A_GET_VALVE_POSITION(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_P_GET_SENSOR_PRESSURE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i30_GET_DEVICE_STATUS(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i32_GET_LEARN_STATUS(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i34_GET_LEARN_PRESSLIMIT(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i50_GET_ERROR_STATUS(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i51_GET_WARNING_STATUS(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i60_GET_SENSOR1_OFFSET(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i61_GET_SENSOR2_OFFSET(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i64_GET_SENSOR1_PRESSURE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i65_GET_SENSOR2_PRESSURE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i70_GET_CONTROL_CYCLECNT(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i71_GET_ISOLATION_CYCLECNT(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i72_GET_POWERUP_COUNTER(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i76_GET_VALVE_MAINSTATUS(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i80_GET_HW_CONFIG(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i82_GET_FW_VERSION(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i83_GET_VALVE_ModelName(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i84_GET_FW_NUMBER(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_c01_SET_ACCESS_MODE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_s04_SET_VALVE_CONFIG(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i04_GET_VALVE_CONFIG(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_s01_SET_SENSOR_CONFIG(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i01_GET_SENSOR_CONFIG(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_s05_SET_SENSOR_SCALE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i05_GET_SENSOR_SCALE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_s21_SET_COMM_RANGE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i21_GET_COMM_RANGE(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_s20_SET_INTERFACE_CONFIG(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i20_GET_INTERFACE_CONFIG(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_Z_SET_ZERO_ADJUST(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_L_START_LEARN(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_d_DOWNLOAD_LEARN_DATA(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_u_UPLOAD_LEARN_DATA(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_V_SET_VALVE_SPEED(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i68_GET_VALVE_SPEED(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_c82_RESET_ERROR(uint8_t ch, char* cmd, char* data, char* respData);
//int cmdd_c99_RESET_CLOSE(uint8_t ch, char* cmd, char* data, char* respData);

int cmds_s02_SET_CONTROLLER_CONFIG(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i02_GET_CONTROLLER_CONFIG(uint8_t ch, char* cmd, char* data, char* respData);

int cmds_s02Z00_SET_PRESSURE_CONTROLLER(uint8_t ch, char* cmd, char* data, char* respData);
int cmds_i02Z00_GET_PRESSURE_CONTROLLER(uint8_t ch, char* cmd, char* data, char* respData);

// Define DASAN private commands
int cmdd_q00_GET_PRE_POS_VAL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q01_GET_HW_CONFIG_ALL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q02_GET_SW_CONFIG_ALL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q03_GET_SENSOR_STATUS_ALL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q04_GET_VALVE_CONFIGALL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q05_GET_CYCLECNT_ALL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q06_GET_PFO_INFOALL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q07_GET_COMPAIR_INFOALL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q08_GET_VALVE_ERRORALL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q09_GET_VALVE_HWINFO(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q10_GET_DAM_MAIN_INFORM(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q11_GET_POS_PR_VAL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q20_GET_SENSOR_CONFIGALL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q21_GET_SENSOR_OFFSETALL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q22_GET_LEARN_INFOALL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q30_GET_ACCESS_MODE(uint8_t ch, char* cmd, char* data, char* respCmdData);
int cmdd_q40_GET_INTERFACE_CONFIG(uint8_t ch, char* cmd, char* data, char* respCmdData);
int cmdd_q70_GET_CONTROL_CYCLECNT(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q71_GET_ISOLATION_CYCLECNT(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q72_GET_POWERUP_COUNTER(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q80_GET_FIXED_CONTROLLER_CONFIG(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_z81_SET_ADAPTIVE_CONTROLLER_CONFIG(uint8_t ch, char* cmd, char* data, char* respData);

int cmdd_n01_NOTI_INTERFACE_TRACE(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_z01_START_INTERFACE_TRACE(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_z02_STOP_INTERFACE_TRACE(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_z06_SET_PFO_STATE(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_z20_SET_SENSOR_CONFIGALL(uint8_t ch, char* cmd, char* data, char* respCmdData);
int cmdd_z22_SET_LEARN_INFOALL(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_z30_SET_ACCESS_MODE(uint8_t ch, char* cmd, char* data, char* respCmdData);
int cmdd_z40_SET_INTERFACE_CONFIG(uint8_t ch, char* cmd, char* data, char* respCmdData);
int cmdd_z70_RESET_CONTROL_CYCLECNT(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_z71_RESET_ISOLATION_CYCLECNT(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_z72_RESET_POWERUP_COUNTER(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_z80_SET_FIXED_CONTROLLER_CONFIG(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_q81_GET_ADAPTIVE_CONTROLLER_CONFIG(uint8_t ch, char* cmd, char* data, char* respData);

int cmdd_Y_SYNC_VALVE(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_y00_GET_FW_FACTORY_STATE(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_y01_SET_FW_FACTORY_STATE(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_y02_RESTART_FW(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_y03_SYNC_DATA(uint8_t ch, char* cmd, char* data, char* respData);

int cmdd_a99_VALVE_CONFIG_START_STOP(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_a10_VALVE_CONFIG_SETUP(uint8_t ch, char* cmd, char* data, char* respData);
//int cmdd_TXX_Test_Valve(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_t00_Stop_Test(uint8_t ch, char* cmd, char* data, char* respData);
int cmdd_t01_Start_Test(uint8_t ch, char* cmd, char* data, char* respData);

#endif
