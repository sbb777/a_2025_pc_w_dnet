/*
 * APC_CmdFilter.c
 *
 *  Created on: Apr 18, 2024
 *      Author: Yongseok
 */

#include <APC_CmdFilter.h>

#include <APC_AccessMode.h>
#include <APC_ControlMode.h>
#include <APC_Utils.h>
#include <APC_Error.h>
#include <APC_CmdHandler.h>

uint8_t filterAccessMode(uint8_t req_ch)
{
    uint8_t curAccessMode = getAccessMode();
    if (req_ch == APC_PORT_LOCAL && curAccessMode == AccessMode_Locked) {
        /*
         * AccessMode_Locked 해제는 Remote에서만 가능
         */
        prtLog(0, LL_WARN, __FUNCTION__, "AccessMode is RemoteLocked\n");
        return Status_WrongAccessModeLocked;
    }
    return 0;
}

uint8_t filterControlMode(int cmd_key)
{
    EN_A_ControlMode curControlMode = getControlMode();
    if (curControlMode < ControlMode_POSITION) {
        prtLog(0, LL_WARN, __FUNCTION__, "ControlMode[%d] is abnormal\n", curControlMode);
        return Status_WrongControlMode;
    }
    return 0;
}

int checkControlMode(int cmdKey)
{
    int result = CMD_PROCESSING_OK;

    EN_A_ControlMode controlMode = getControlMode();
    switch (cmdKey)
    {
        case CMD_KEY_C_CLOSE_VALVE  :
        case CMD_KEY_O_OPEN_VALVE   :
        case CMD_KEY_H_HOLD_VALVE   :
        case CMD_KEY_R_SETPOINT_VALVE_POSITION   :
        case CMD_KEY_r00_SETPOINT_VALVE_POSITION   :    // only LOCAL_PORT
        case CMD_KEY_r01_SETPOINT_VALVE_POSITION   :    // only LOCAL_PORT

        case CMD_KEY_S_SETPOINT_VALVE_PRESSURE   :
        case CMD_KEY_A_GET_VALVE_POSITION :
        case CMD_KEY_P_GET_SENSOR_PRESSURE :
        case CMD_KEY_V_SET_VALVE_SPEED :

        case CMD_KEY_i01_GET_SENSOR_CONFIG :
        case CMD_KEY_i04_GET_VALVE_CONFIG :
        case CMD_KEY_i05_GET_SENSOR_SCALE :
        case CMD_KEY_i21_GET_COMM_RANGE :
        case CMD_KEY_i20_GET_INTERFACE_CONFIG :
        case CMD_KEY_i30_GET_DEVICE_STATUS :
        case CMD_KEY_i32_GET_LEARN_STATUS :
        case CMD_KEY_i34_GET_LEARN_PRESSLIMIT :
        case CMD_KEY_i38_GETPOINT_VALVE_POSITION :
        case CMD_KEY_i39_GET_ENCODER_VALUE:
        case CMD_KEY_i50_GET_ERROR_STATUS :
        case CMD_KEY_i51_GET_WARNING_STATUS :
        case CMD_KEY_i60_GET_SENSOR1_OFFSET :
        case CMD_KEY_i61_GET_SENSOR2_OFFSET :
        case CMD_KEY_i64_GET_SENSOR1_VALUE :
        case CMD_KEY_i65_GET_SENSOR2_VALUE :
        case CMD_KEY_i68_GET_VALVE_SPEED :
        case CMD_KEY_i70_GET_CONTROL_CYCLECNT :
        case CMD_KEY_i71_GET_ISOLATION_CYCLECNT :
        case CMD_KEY_i72_GET_POWERUP_COUNTER :
        case CMD_KEY_i76_GET_VALVE_MAINSTATUS :
        case CMD_KEY_i80_GET_HW_CONFIG :
        case CMD_KEY_i82_GET_FW_VERSION :
        case CMD_KEY_i83_GET_VALVE_SERIALNO :
        case CMD_KEY_i84_GET_FW_NUMBER :

        case CMD_KEY_s01_SET_SENSOR_CONFIG :
        case CMD_KEY_s04_SET_VALVE_CONFIG :
        case CMD_KEY_s05_SET_SENSOR_SCALE :
        case CMD_KEY_s20_SET_INTERFACE_CONFIG :
        case CMD_KEY_s21_SET_COMM_RANGE :

        case CMD_KEY_Z_SET_ZERO_ADJUST :
        case CMD_KEY_L_START_LEARN :
        case CMD_KEY_d_DOWNLOAD_LEARN_DATA :
        case CMD_KEY_u_UPLOAD_LEARN_DATA :
        case CMD_KEY_c01_SET_ACCESS_MODE :
        case CMD_KEY_c82_RESET_ERROR :

        case CMD_KEY_q00_GET_PRE_POS_VAL :
        case CMD_KEY_q01_GET_HW_CONFIG_ALL:
        case CMD_KEY_q02_GET_SW_CONFIG_ALL:
        case CMD_KEY_q03_GET_SENSOR_STATUS_ALL:
        case CMD_KEY_q04_GET_VALVE_CONFIGALL:
        case CMD_KEY_q05_GET_CYCLECNT_ALL:
        case CMD_KEY_q06_GET_PFO_INFOALL:
        case CMD_KEY_q07_GET_COMPAIR_INFOALL:
        case CMD_KEY_q08_GET_VALVE_ERRORALL:
        case CMD_KEY_q09_GET_VALVE_HWINFO:
        case CMD_KEY_q10_GET_DAM_MAIN_INFORM:
        case CMD_KEY_q20_GET_SENSOR_CONFIGALL:
        case CMD_KEY_q21_GET_SENSOR_OFFSETALL:
        case CMD_KEY_q22_GET_LEARN_INFOALL:
        case CMD_KEY_q30_GET_ACCESS_MODE:
        case CMD_KEY_q70_GET_CONTROL_CYCLECNT:
        case CMD_KEY_q71_GET_ISOLATION_CYCLECNT:
        case CMD_KEY_q72_GET_POWERUP_COUNTER:
        case CMD_KEY_q80_GET_FIXED_CONTROLLER_CONFIG:
        case CMD_KEY_q81_GET_ADAPTIVE_CONTROLLER_CONFIG:

        case CMD_KEY_n01_NOTI_INTERFACE_TRACE:

        case CMD_KEY_z01_START_INTERFACE_TRACE:
        case CMD_KEY_z02_STOP_INTERFACE_TRACE:
        case CMD_KEY_z06_SET_PFO_STATE:
        case CMD_KEY_z20_SET_SENSOR_CONFIGALL:
        case CMD_KEY_z22_SET_LEARN_INFOALL:
        case CMD_KEY_z30_SET_ACCESS_MODE:
        case CMD_KEY_z70_RESET_CONTROL_CYCLECNT:
        case CMD_KEY_z71_RESET_ISOLATION_CYCLECNT:
        case CMD_KEY_z72_RESET_POWERUP_COUNTER:
        case CMD_KEY_z80_SET_FIXED_CONTROLLER_CONFIG:
        case CMD_KEY_z81_SET_ADAPTIVE_CONTROLLER_CONFIG:

        case CMD_KEY_t00_TEST_CONTROLLER:
        case CMD_KEY_t01_TEST_CONTROLLER:

        case CMD_KEY_Y_SYNC_VALVE:
        case CMD_KEY_y00_GET_FW_FACTORY_STATE:
        case CMD_KEY_y01_SET_FW_FACTORY_STATE:
        case CMD_KEY_y02_RESTART_FW:
        case CMD_KEY_y03_SYNC_MO_DATA:
            if (controlMode < ControlMode_POSITION)
                result = Status_WrongControlMode;
            break;

        case CMD_KEY_a10_VALVE_CONFIG_SETUP:
        case CMD_KEY_a99_VALVE_CONFIG_START_STOP:
            break;

        default :
            break;
    }

    if (result != CMD_PROCESSING_OK) {
        prtLog(0, LL_WARN, __FUNCTION__, "Current ControlMode[%d] is NOT supported.\n", controlMode);
    }
    return result;
}

