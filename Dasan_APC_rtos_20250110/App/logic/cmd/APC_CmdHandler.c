/* ----------------------------------------------------------------------
    Description : 명령어 분서기 동작 구성도

         doCmdHandler(null terminated command full string)   / FW_CmdHandler.c
                    |
                    -->  doCmdParser()          / FW_CmdHandler.c
                    |
                    -->  cmdKey = getCmdKey(cmd)        / FW_CmdHandler.c
                    |
                    -->  switch(cmdKey)         / 처리할 각 명령어 function으로 분기
                           case  CMD_KEY_R_SETPOINT_VALVE_POSITION :
                                   cmds_R_SETPOINT_VALVE_POSITION(ch, recvCmd, recvData, respCmdData); break;  / 실제 수행될 cmd func.
                           ................

                    / 각 명렁어에 대한 처리 함수
                    / cmds -> 규격에 정의되어있는...
                    / cmdd -> 다산에 만 정의된...
                    / R_   -> VAT 명령어
                    / SETPOINT_VALVE_POSITION -> 다산 명렁어 이름
                    cmds_R_SETPOINT_VALVE_POSITION()
                        -. data length array 정의
                        -. data value string array 정의
                        -. data 추출 : extractCmdData()
                        -. MO 변수에 data type 별 data assign
                        -. 명령어별 처리해야 하는 action 정의
                        -. response cmd data 구성
                        -. return

 */

#include <APC_CmdHandler.h>

#include <APC_CmdFilter.h>
#include <APC_CmdFunctions.h>
#include <APC_Utils.h>
#include "APC_Error.h"

/* ----------------------------------------------------------------------
    Description : Main Handler for command's processing
    Parameters  : -. Received command data (null terminated string deleted \r\n char)
                  -. Response command data (null terminated string deleted \r\n char)
    Return      : CMD_PROCESSING_OK / CMD_PROCESSING_NOK
 */
int doCmdHandler(uint8_t ch, char* recvCmdData, char* respCmdData) {
    char recvCmd[32] = { 0 };
    char recvData[128] = { 0 };

    // Parsing : recvCmdData -> recvCmd, recvData
    int parsingResult = doCmdParser(recvCmdData, recvCmd, recvData);
    if (parsingResult != CMD_PARSING_OK) {
        return CMD_PARSING_NOK;
    }
    prtLog(0, LL_DEBUG, __FUNCTION__, "recvCmdData=[%s], cmd=[%s], data=[%s]\n", recvCmdData, recvCmd, recvData);

    /*
     * basic filtering
     */
    uint8_t accessModeFilter  = filterAccessMode(ch);
    uint8_t controlModeFilter = filterControlMode(ch);

    int cmdKey = getCmdKey(recvCmd);
    int cmdResult = CMD_PROCESSING_OK;

    switch (cmdKey)
    {
        case CMD_KEY_C_CLOSE_VALVE  :
            if (accessModeFilter > 0)   return accessModeFilter;
            if (controlModeFilter > 0)  return controlModeFilter;
            cmdResult = cmds_C_CLOSE_VALVE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_O_OPEN_VALVE   :
            if (accessModeFilter > 0)   return accessModeFilter;
            if (controlModeFilter > 0)  return controlModeFilter;
            cmdResult = cmds_O_OPEN_VALVE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_H_HOLD_VALVE   :
            if (accessModeFilter > 0)   return accessModeFilter;
            if (controlModeFilter > 0)  return controlModeFilter;
            cmdResult = cmds_H_HOLD_VALVE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_R_SETPOINT_VALVE_POSITION   :
            if (accessModeFilter > 0)   return accessModeFilter;
            if (controlModeFilter > 0)  return controlModeFilter;
            cmdResult = cmds_R_SETPOINT_VALVE_POSITION(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_r00_SETPOINT_VALVE_POSITION   :    // only LOCAL_PORT
        case CMD_KEY_r01_SETPOINT_VALVE_POSITION   :    // only LOCAL_PORT
            if (accessModeFilter > 0)   return accessModeFilter;
            if (controlModeFilter > 0)  return controlModeFilter;
            cmdResult = cmdd_rXX_SETPOINT_VALVE_POSITION(ch, recvCmd, recvData, respCmdData); break;

        case CMD_KEY_S_SETPOINT_VALVE_PRESSURE   :
            if (accessModeFilter > 0)   return accessModeFilter;
            if (controlModeFilter > 0)  return controlModeFilter;
            cmdResult = cmds_S_SETPOINT_VALVE_PRESSURE(ch, recvCmd, recvData, respCmdData); break;
//        case CMD_KEY_i38_GETPOINT_VALVE_PRESSURE :
//            cmdResult = cmds_i38_GETPOINT_VALVE_PRESSURE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_A_GET_VALVE_POSITION :
            //if (controlModeFilter > 0)  return controlModeFilter;
            cmdResult = cmds_A_GET_VALVE_POSITION(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_P_GET_SENSOR_PRESSURE :
            cmdResult = cmds_P_GET_SENSOR_PRESSURE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_V_SET_VALVE_SPEED :
            if (accessModeFilter > 0)   return accessModeFilter;
            if (controlModeFilter > 0)  return controlModeFilter;
            cmdResult = cmds_V_SET_VALVE_SPEED(ch, recvCmd, recvData, respCmdData); break;

        case CMD_KEY_i01_GET_SENSOR_CONFIG :
            cmdResult = cmds_i01_GET_SENSOR_CONFIG(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i02_GET_CONTROLLER_CONFIG :  // Not used
            cmdResult = cmds_i02_GET_CONTROLLER_CONFIG(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i02_GET_PRESSURE_CONTROLLER : // Not used
            cmdResult = cmds_i02Z00_GET_PRESSURE_CONTROLLER(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i04_GET_VALVE_CONFIG :
            cmdResult = cmds_i04_GET_VALVE_CONFIG(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i05_GET_SENSOR_SCALE :
            cmdResult = cmds_i05_GET_SENSOR_SCALE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i21_GET_COMM_RANGE :
            cmdResult = cmds_i21_GET_COMM_RANGE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i20_GET_INTERFACE_CONFIG :
            cmdResult = cmds_i20_GET_INTERFACE_CONFIG(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i30_GET_DEVICE_STATUS :
            cmdResult = cmds_i30_GET_DEVICE_STATUS(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i32_GET_LEARN_STATUS :
            cmdResult = cmds_i32_GET_LEARN_STATUS(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i34_GET_LEARN_PRESSLIMIT :
            cmdResult = cmds_i34_GET_LEARN_PRESSLIMIT(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i38_GETPOINT_VALVE_POSITION :
            cmdResult = cmds_i38_GETPOINT_VALVE_POSITION(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i39_GET_ENCODER_VALUE :
            cmdResult = cmds_i39_GET_ENCODER_VALUE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i50_GET_ERROR_STATUS :
            cmdResult = cmds_i50_GET_ERROR_STATUS(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i51_GET_WARNING_STATUS :
            cmdResult = cmds_i51_GET_WARNING_STATUS(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i60_GET_SENSOR1_OFFSET :
            cmdResult = cmds_i60_GET_SENSOR1_OFFSET(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i61_GET_SENSOR2_OFFSET :
            cmdResult = cmds_i61_GET_SENSOR2_OFFSET(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i64_GET_SENSOR1_VALUE :
            cmdResult = cmds_i64_GET_SENSOR1_PRESSURE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i65_GET_SENSOR2_VALUE :
            cmdResult = cmds_i65_GET_SENSOR2_PRESSURE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i68_GET_VALVE_SPEED :
            cmdResult = cmds_i68_GET_VALVE_SPEED(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i70_GET_CONTROL_CYCLECNT :
            cmdResult = cmds_i70_GET_CONTROL_CYCLECNT(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i71_GET_ISOLATION_CYCLECNT :
            cmdResult = cmds_i71_GET_ISOLATION_CYCLECNT(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i72_GET_POWERUP_COUNTER :
            cmdResult = cmds_i72_GET_POWERUP_COUNTER(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i76_GET_VALVE_MAINSTATUS :
            //if (controlModeFilter > 0)  return controlModeFilter;
            cmdResult = cmds_i76_GET_VALVE_MAINSTATUS(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i80_GET_HW_CONFIG :
            cmdResult = cmds_i80_GET_HW_CONFIG(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i82_GET_FW_VERSION :
            cmdResult = cmds_i82_GET_FW_VERSION(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i83_GET_VALVE_SERIALNO :
            cmdResult = cmds_i83_GET_VALVE_ModelName(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_i84_GET_FW_NUMBER :
            cmdResult = cmds_i84_GET_FW_NUMBER(ch, recvCmd, recvData, respCmdData); break;

        case CMD_KEY_s01_SET_SENSOR_CONFIG :
            if (accessModeFilter > 0)   return accessModeFilter;
            cmdResult = cmds_s01_SET_SENSOR_CONFIG(ch, recvCmd, recvData, respCmdData); break;
//        case CMD_KEY_s02_SET_CONTROLLER_CONFIG :  // Not used
//            cmdResult = cmds_s02_SET_CONTROLLER_CONFIG(ch, recvCmd, recvData, respCmdData); break;
//        case CMD_KEY_s02_SET_PRESSURE_CONTROLLER : // Not used
//            cmdResult = cmds_s02Z00_SET_PRESSURE_CONTROLLER(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_s04_SET_VALVE_CONFIG :
            if (accessModeFilter > 0)   return accessModeFilter;
            cmdResult = cmds_s04_SET_VALVE_CONFIG(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_s05_SET_SENSOR_SCALE :
            if (accessModeFilter > 0)   return accessModeFilter;
            cmdResult = cmds_s05_SET_SENSOR_SCALE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_s20_SET_INTERFACE_CONFIG :
            if (accessModeFilter > 0)   return accessModeFilter;
            cmdResult = cmds_s20_SET_INTERFACE_CONFIG(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_s21_SET_COMM_RANGE :
            if (accessModeFilter > 0)   return accessModeFilter;
            cmdResult = cmds_s21_SET_COMM_RANGE(ch, recvCmd, recvData, respCmdData); break;

        case CMD_KEY_Z_SET_ZERO_ADJUST :
            if (accessModeFilter > 0)   return accessModeFilter;
            cmdResult = cmds_Z_SET_ZERO_ADJUST(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_L_START_LEARN :
            if (accessModeFilter > 0)   return accessModeFilter;
            cmdResult = cmds_L_START_LEARN(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_d_DOWNLOAD_LEARN_DATA :
            cmdResult = cmds_d_DOWNLOAD_LEARN_DATA(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_u_UPLOAD_LEARN_DATA :
            cmdResult = cmds_u_UPLOAD_LEARN_DATA(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_c01_SET_ACCESS_MODE :
            cmdResult = cmds_c01_SET_ACCESS_MODE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_c82_RESET_ERROR :
            cmdResult = cmds_c82_RESET_ERROR(ch, recvCmd, recvData, respCmdData); break;
//        case CMD_KEY_c99_RESET_CLOSE :
//            cmdResult = cmdd_c99_RESET_CLOSE(ch, recvCmd, recvData, respCmdData); break;

            // DASAN local commands
        case CMD_KEY_q00_GET_PRE_POS_VAL :
            cmdResult = cmdd_q00_GET_PRE_POS_VAL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q01_GET_HW_CONFIG_ALL:
            cmdResult = cmdd_q01_GET_HW_CONFIG_ALL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q02_GET_SW_CONFIG_ALL:
            cmdResult = cmdd_q02_GET_SW_CONFIG_ALL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q03_GET_SENSOR_STATUS_ALL:
            cmdResult = cmdd_q03_GET_SENSOR_STATUS_ALL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q04_GET_VALVE_CONFIGALL:
            cmdResult = cmdd_q04_GET_VALVE_CONFIGALL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q05_GET_CYCLECNT_ALL:
            cmdResult = cmdd_q05_GET_CYCLECNT_ALL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q06_GET_PFO_INFOALL:
            cmdResult = cmdd_q06_GET_PFO_INFOALL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q07_GET_COMPAIR_INFOALL:
            cmdResult = cmdd_q07_GET_COMPAIR_INFOALL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q08_GET_VALVE_ERRORALL:
            cmdResult = cmdd_q08_GET_VALVE_ERRORALL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q09_GET_VALVE_HWINFO:
            cmdResult = cmdd_q09_GET_VALVE_HWINFO(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q10_GET_DAM_MAIN_INFORM:
            cmdResult = cmdd_q10_GET_DAM_MAIN_INFORM(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q11_GET_POS_PR_VAL:
            cmdResult = cmdd_q11_GET_POS_PR_VAL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q20_GET_SENSOR_CONFIGALL:
            cmdResult = cmdd_q20_GET_SENSOR_CONFIGALL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q21_GET_SENSOR_OFFSETALL:
            cmdResult = cmdd_q21_GET_SENSOR_OFFSETALL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q22_GET_LEARN_INFOALL:
            cmdResult = cmdd_q22_GET_LEARN_INFOALL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q30_GET_ACCESS_MODE:
            cmdResult = cmdd_q30_GET_ACCESS_MODE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q40_GET_INTERFACE_CONFIG:
            cmdResult = cmdd_q40_GET_INTERFACE_CONFIG(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q70_GET_CONTROL_CYCLECNT:
            cmdResult = cmdd_q70_GET_CONTROL_CYCLECNT(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q71_GET_ISOLATION_CYCLECNT:
            cmdResult = cmdd_q71_GET_ISOLATION_CYCLECNT(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q72_GET_POWERUP_COUNTER:
            cmdResult = cmdd_q72_GET_POWERUP_COUNTER(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q80_GET_FIXED_CONTROLLER_CONFIG:
            cmdResult = cmdd_q80_GET_FIXED_CONTROLLER_CONFIG(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_q81_GET_ADAPTIVE_CONTROLLER_CONFIG:
            cmdResult = cmdd_q81_GET_ADAPTIVE_CONTROLLER_CONFIG(ch, recvCmd, recvData, respCmdData); break;

        case CMD_KEY_n01_NOTI_INTERFACE_TRACE:
            cmdResult = cmdd_n01_NOTI_INTERFACE_TRACE(ch, recvCmd, recvData, respCmdData); break;

        case CMD_KEY_z01_START_INTERFACE_TRACE:
            cmdResult = cmdd_z01_START_INTERFACE_TRACE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_z02_STOP_INTERFACE_TRACE:
            cmdResult = cmdd_z02_STOP_INTERFACE_TRACE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_z06_SET_PFO_STATE:
            cmdResult = cmdd_z06_SET_PFO_STATE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_z20_SET_SENSOR_CONFIGALL:
            cmdResult = cmdd_z20_SET_SENSOR_CONFIGALL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_z22_SET_LEARN_INFOALL:
            cmdResult = cmdd_z22_SET_LEARN_INFOALL(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_z30_SET_ACCESS_MODE:
            cmdResult = cmdd_z30_SET_ACCESS_MODE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_z40_SET_INTERFACE_CONFIG:
            cmdResult = cmdd_z40_SET_INTERFACE_CONFIG(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_z70_RESET_CONTROL_CYCLECNT:
            cmdResult = cmdd_z70_RESET_CONTROL_CYCLECNT(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_z71_RESET_ISOLATION_CYCLECNT:
            cmdResult = cmdd_z71_RESET_ISOLATION_CYCLECNT(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_z72_RESET_POWERUP_COUNTER:
            cmdResult = cmdd_z72_RESET_POWERUP_COUNTER(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_z80_SET_FIXED_CONTROLLER_CONFIG:
            cmdResult = cmdd_z80_SET_FIXED_CONTROLLER_CONFIG(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_z81_SET_ADAPTIVE_CONTROLLER_CONFIG:
            cmdResult = cmdd_z81_SET_ADAPTIVE_CONTROLLER_CONFIG(ch, recvCmd, recvData, respCmdData); break;

//        case CMD_KEY_TXX_TEST_CONTROLLER:
//            cmdResult = cmdd_TXX_Test_Valve(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_t00_TEST_CONTROLLER:
            if (accessModeFilter > 0)   return accessModeFilter;
            if (controlModeFilter > 0)   return controlModeFilter;
            cmdResult = cmdd_t00_Stop_Test(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_t01_TEST_CONTROLLER:
            if (accessModeFilter > 0)   return accessModeFilter;
            if (controlModeFilter > 0)   return controlModeFilter;
            cmdResult = cmdd_t01_Start_Test(ch, recvCmd, recvData, respCmdData); break;

        case CMD_KEY_Y_SYNC_VALVE:
            if (accessModeFilter > 0)   return accessModeFilter;
            //if (controlModeFilter > 0)   return controlModeFilter;
            cmdResult = cmdd_Y_SYNC_VALVE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_y00_GET_FW_FACTORY_STATE:
            cmdResult = cmdd_y00_GET_FW_FACTORY_STATE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_y01_SET_FW_FACTORY_STATE:
            if (accessModeFilter > 0)   return accessModeFilter;
            //if (controlModeFilter > 0)   return controlModeFilter;
            cmdResult = cmdd_y01_SET_FW_FACTORY_STATE(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_y02_RESTART_FW:
            if (accessModeFilter > 0)   return accessModeFilter;
            //if (controlModeFilter > 0)   return controlModeFilter;
            cmdResult = cmdd_y02_RESTART_FW(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_y03_SYNC_MO_DATA:
            if (accessModeFilter > 0)   return accessModeFilter;
            //if (controlModeFilter > 0)   return controlModeFilter;
            cmdResult = cmdd_y03_SYNC_DATA(ch, recvCmd, recvData, respCmdData); break;

        case CMD_KEY_a10_VALVE_CONFIG_SETUP:
            if (accessModeFilter > 0)   return accessModeFilter;
            //if (controlModeFilter > 0)   return controlModeFilter;
            cmdResult = cmdd_a10_VALVE_CONFIG_SETUP(ch, recvCmd, recvData, respCmdData); break;
        case CMD_KEY_a99_VALVE_CONFIG_START_STOP:
            if (accessModeFilter > 0)   return accessModeFilter;
            //if (controlModeFilter > 0)   return controlModeFilter;
            cmdResult = cmdd_a99_VALVE_CONFIG_START_STOP(ch, recvCmd, recvData, respCmdData); break;

        default :
            prtLog(0, LL_ERROR, __FUNCTION__, "Not defined Command key. cmd=[%s], cmdKey=[%d]\n", recvCmd, cmdKey);
            cmdResult = CMD_PARSING_NOK;
            break;
    }
    return cmdResult;
}

/* ----------------------------------------------------------------------
    Description : Calculate command hash key value
    Parameters  : -. only command part string
    Return      : command hash key value
 */
int getCmdKey(char* cmd) {
    // Extract recved data with dataLen[]
    int cmdKey = -1;
    if(cmd[2] == 0) {
        cmdKey = (cmd[0] << CMD_KEY_SHIFT_COUNT);
    }
    else if (strncmp(cmd, "i:02Z00", 7) == 0 || strncmp(cmd, "s:02Z00", 7) == 0) {
        cmdKey = (cmd[0] << CMD_KEY_SHIFT_COUNT) + 22;
    }
    else {
        cmdKey = (cmd[0] << CMD_KEY_SHIFT_COUNT) + atoi(cmd+2);       // cmd+1 = ':', so skip
    }
    return cmdKey;
}

/* ----------------------------------------------------------------------
    Description : Parsing the remote and service command from RS232
    Parameters  : -. Received data from RS232
    Return      : CMD_PARSE_OK | CMD_PARSE_NOK
 */
int doCmdParser(char* recvCmdData, char* cmd, char* data) {
    uint16_t cmdLen  = 0;

    if (recvCmdData[1] != ':') {
        prtLog(0, LL_WARN, __FUNCTION__, "recvCmdData=[%s] is ColonMissing\n", recvCmdData);
        return Format_ColonMissing;
    }

    switch (recvCmdData[0])
    {
        case 'C' : cmdLen = 2; break;
        case 'O' : cmdLen = 2; break;
        case 'H' : cmdLen = 2; break;
        case 'R' : cmdLen = 2; break;
        case 'S' : cmdLen = 2; break;
        case 'A' : cmdLen = 2; break;  // GET-VALVE-POSITION
        case 'P' : cmdLen = 2; break;  // GET-SENSOR-PRESSURE
        case 'Z' : cmdLen = 2; break;
        case 'L' : cmdLen = 2; break;
        case 'V' : cmdLen = 2; break;
        case 'i' :
            cmdLen = strncmp(recvCmdData, "i:02Z00", 7) == 0 ? 7 : 4;
            break;
        case 'c' : cmdLen = 4; break;
        case 's' :
            cmdLen = strncmp(recvCmdData, "s:02Z00", 7) == 0 ? 7 : 4;
            break;
        case 'q' : cmdLen = 4; break;
        case 'z' : cmdLen = 4; break;
        case 'd' : cmdLen = 2; break;
        case 'u' : cmdLen = 2; break;

        case 'r' : cmdLen = 4; break;   // added 2024-01-24
        case 'Y' : cmdLen = 2; break;   // added 2024-02-08
        case 'y' : cmdLen = 4; break;
        //case 'T' : cmdLen = 2; break;   // added 2023-12-30
        case 't' : cmdLen = 4; break;   // added 2024-03-06
        case 'a' : cmdLen = 4; break;   // added 2024-04-06

        default  :
            prtLog(0, LL_ERROR, __FUNCTION__, "Not defined cmd=[%c]\n", recvCmdData[0]);
            return CMD_PARSING_NOK;
    }

    if (strlen(recvCmdData) < cmdLen)   return CMD_PARSING_NOK;

    uint16_t dataLen  = strlen(recvCmdData) - cmdLen;
    // Get only command from recved data
    strncpy(cmd, recvCmdData, cmdLen);
    strncpy(data, (char*) recvCmdData+cmdLen, dataLen);
    //prtLog(0, LL_DEBUG, __FUNCTION__, "cmd=[%s], data=[%s] mismatched Length=[%d]\n", cmd, data, dataLen);
    return CMD_PARSING_OK;
}
