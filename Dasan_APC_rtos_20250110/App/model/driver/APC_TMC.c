/*
 * APC_TMC.c
 *
 *  Created on: 2024. 1. 2.
 *      Author: Yongseok
 */

#include <APC_TMC.h>
#include "APC_Utils.h"
#ifdef _USE_THREAD
    #include "FreeRTOS.h"
    #include "task.h"
#endif

#include <APC_Define.h>

#define SET_DELAY       (3)

#define MOTOR_VMAX_VALUE    (0x4C4B00) // (0x4C4B40) Pendulum, 350 rpm     0x53E000
#define MOTOR_AMAX_VALUE    (  0x6030)  // (  0x61A8)


static Ramp_Generator_Register          ramp_generator_register;
static Encoder_Register                 encoder_register;
static Sequencer_Configuration_Register sequencer_configuration_register;

static uint32_t _maxVelocity, _maxAcc;

#ifdef __REV_MTECH__
#else
extern SRAM_HandleTypeDef hsram3;
#endif

int  readTMC457(uint8_t address, uint8_t length);
bool writeTMC457(uint8_t address, int value, uint8_t length);

/**
 * use K-one recommend values.
 */
bool initTMC(uint8_t valve_type, uint16_t valve_size)
{
    // check ValveType_Pendulum
    uint16_t TMC457Mode = (valve_type == 1) ? 0x0310 : 0x0300;      // 0x0010: 0x0000;

    switch(valve_type){
        case 1:
            if(valve_size == 250){
                _maxVelocity = MOTOR_VMAX_VALUE;
                _maxAcc = MOTOR_AMAX_VALUE;
            }
            break;
        case 2:
            if(valve_size == 80){
                _maxVelocity = MOTOR_VMAX_VALUE * 7;  //0x2160EC0;     //MOTOR_VMAX_VALUE * 7;
                _maxAcc = MOTOR_AMAX_VALUE * 6;  //0x249F0;       //MOTOR_AMAX_VALUE * 6;
            }
            else if(valve_size == 100){
                _maxVelocity = MOTOR_VMAX_VALUE * 4;  //0x1312D00;     //MOTOR_VMAX_VALUE * 4;
                _maxAcc = MOTOR_AMAX_VALUE * 3.4;//0x14C08;       //MOTOR_AMAX_VALUE * 3.4;
            }
            else{
                _maxVelocity = MOTOR_VMAX_VALUE * 4;  //0x1312D00;     //MOTOR_VMAX_VALUE * 4;
                _maxAcc = MOTOR_AMAX_VALUE * 3.4;//0x14C08;       //MOTOR_AMAX_VALUE * 3.4;
            }
            break;
    }

    writeTMC457(TMC457_CHOP_CLK_DIV,  0x00DE, 2);    APC_Delay(SET_DELAY);  // fCLK=8MHz, fOSC=36KHz
    writeTMC457(TMC457_SEQ_DAC_SCALE, 0x8F8F, 3);    APC_Delay(SET_DELAY);  // 0x9090 / 0x8188, current_op=8, mixdecay_op=1, current_sb=1, mixdecay_sb=1,  0x9090
    writeTMC457(TMC457_SEQ_MODE,        0x06, 2);    APC_Delay(SET_DELAY);  // 6 (=128 entries)
    writeTMC457(TMC457_ENC_CONST,   0x040000, 4);    APC_Delay(SET_DELAY);
    writeTMC457(TMC457_ENC_MODE,        0x00, 2);    APC_Delay(SET_DELAY);  //
    writeTMC457(TMC457_SWITCH_MODE,     0x0F, 2);    APC_Delay(SET_DELAY);  // stop_L,stop_R,pol_stop_L,pol_stop_R
    writeTMC457(TMC457_BOWMAX,          0x0B, 1);    APC_Delay(SET_DELAY);  // Bow Max

    writeTMC457(TMC457_MODE,      TMC457Mode, 2);    APC_Delay(SET_DELAY);
    // XXX when changed, no effect.
    writeTMC457(TMC457_VTARGET,         0x00, 4);    APC_Delay(SET_DELAY);  // 0x0320 (K-one recommend value)
    writeTMC457(TMC457_VMAX,      _maxVelocity, 4);  APC_Delay(SET_DELAY);
    writeTMC457(TMC457_AMAX_DMAX, _maxAcc, 3);       APC_Delay(SET_DELAY);  // 0x0C80 (K-one recommend value)

    writeTMC457(TMC457_PID_P,           500, 3);        // 300
    writeTMC457(TMC457_PID_I,            80, 3);        // 15
    writeTMC457(TMC457_PID_D,            60, 3);        // 2
    writeTMC457(TMC457_PID_ICLIP,         4, 2);        // 1
    writeTMC457(TMC457_PID_DCLKDIV,      20, 1);        // 1
    writeTMC457(TMC457_PID_DV_CLIP,  800000, 4);        // 500000
    writeTMC457(TMC457_PID_TOLERANCE,     2, 3);        // 1

    refreshTMCValues();

    return true;
}

void refreshTMCValues(void)
{
    ramp_generator_register.mode.all     = (uint16_t) readTMC457(TMC457_MODE,    2);
    ramp_generator_register.x_actual     =            readTMC457(TMC457_XACTUAL, 4);
    ramp_generator_register.v_actual     =            readTMC457(TMC457_VACTUAL, 4);
    ramp_generator_register.v_max        =            readTMC457(TMC457_VMAX,    4);
    ramp_generator_register.v_target     =            readTMC457(TMC457_VTARGET, 4);
    ramp_generator_register.a_max        =            readTMC457(TMC457_AMAX,    3);
    ramp_generator_register.d_max        =            readTMC457(TMC457_DMAX,    3);
    ramp_generator_register.d_stop       =            readTMC457(TMC457_DSTOP,   3);
    ramp_generator_register.bow_max      =            readTMC457(TMC457_BOWMAX,  1);
    ramp_generator_register.x_target     =            readTMC457(TMC457_XTARGET, 4);
    ramp_generator_register.x_compare    =            readTMC457(TMC457_XCOMPARE, 4);
    ramp_generator_register.status.all   =  (uint8_t) readTMC457(TMC457_STATUS,  1);
    ramp_generator_register.a_actual     =            readTMC457(TMC457_AACTUAL, 4);
    ramp_generator_register.sd_scale.all = (uint16_t) readTMC457(TMC457_SDSCALE, 2);
    ramp_generator_register.a_max_d_max  =            readTMC457(TMC457_AMAX_DMAX, 3);

    encoder_register.enc_const           =            readTMC457(TMC457_ENC_CONST, 4);
    encoder_register.enc_x               =            readTMC457(TMC457_ENC_X,     4);
    encoder_register.enc_mode.all        = (uint16_t) readTMC457(TMC457_ENC_MODE,  2);
    encoder_register.enc_status          =            readTMC457(TMC457_ENC_STATUS, 1);
    encoder_register.enc_latch           =            readTMC457(TMC457_ENC_LATCH, 4);
    encoder_register.x_act_latch         =            readTMC457(TMC457_XLATCH,    4);
    encoder_register.enc_warn_dist       =            readTMC457(TMC457_ENC_WARNDIST, 3);

    sequencer_configuration_register.seq_mode      = readTMC457(TMC457_SEQ_MODE,  2);
    sequencer_configuration_register.dacscale_icntl.all = (uint32_t) readTMC457(TMC457_SEQ_DAC_SCALE, 3);
    sequencer_configuration_register.driver_status = readTMC457(TMC457_SEQ_DRIVER_STATUS, 1);
    sequencer_configuration_register.chop_clk_div  = readTMC457(TMC457_CHOP_CLK_DIV, 2);
}

void setSynchCondition(bool bMode)
{
    if(bMode == true){
        writeTMC457(TMC457_SEQ_DAC_SCALE, 0x8188, 3);    APC_Delay(SET_DELAY);  // 0x8188, current_op=8, mixdecay_op=1, current_sb=1, mixdecay_sb=1,  0x9090
        writeTMC457(TMC457_VMAX,      _maxVelocity * 7 /10, 4);  APC_Delay(SET_DELAY);
        writeTMC457(TMC457_AMAX_DMAX, _maxAcc*6/10, 3);       APC_Delay(SET_DELAY);  // 0x0C80 (K-one recommend value)
    }
    else{
        writeTMC457(TMC457_SEQ_DAC_SCALE, 0x9090, 3);    APC_Delay(SET_DELAY);  // 0x8188, current_op=8, mixdecay_op=1, current_sb=1, mixdecay_sb=1,  0x9090
        writeTMC457(TMC457_VMAX,      _maxVelocity, 4);  APC_Delay(SET_DELAY);
        writeTMC457(TMC457_AMAX_DMAX, _maxAcc, 3);       APC_Delay(SET_DELAY);  // 0x0C80 (K-one recommend value)
    }
}

void setRampMode(uint8_t ramp_mode)
{
    getRampMode();
    ramp_generator_register.mode.bits.ramp_mode = ramp_mode;
    writeTMC457(TMC457_MODE, ramp_generator_register.mode.all, 2);
    APC_Delay(SET_DELAY);
}

uint8_t getRampMode(void)
{
    ramp_generator_register.mode.all  = (uint16_t) readTMC457(TMC457_MODE, 2);
    return ramp_generator_register.mode.bits.ramp_mode;
}

void setPIDMode(bool bMode)
{
    getRampMode();
    if(bMode == true){
        ramp_generator_register.mode.bits.PID_on = 1;
        ramp_generator_register.mode.bits.PID_base = 1;
    }
    else{
        ramp_generator_register.mode.bits.PID_on = 0;
        ramp_generator_register.mode.bits.PID_base = 0;
    }

    writeTMC457(TMC457_MODE, ramp_generator_register.mode.all, 2);
    APC_Delay(SET_DELAY);
}

void setSolenoidStatus(bool option)
{
#ifdef __REV_MTECH__
#else
    option = APC_HIGH;
    uint8_t data[5] = { 0 };
    data[1] = (option == APC_HIGH) ? 0x01 : 0x00;

    APC_Delay(option == APC_HIGH ? 0 : 500);

    HAL_SRAM_Write_8b(&hsram3, (uint32_t*)(NOR_MEMORY_ADRESS3 + TMC457_SOLENOID_VALVE), data, 5);

    refreshTMCValues();

    APC_Delay(option == APC_HIGH ? 2000 : 2000);
#endif
}

bool getSolenoidStatus()
{
#ifdef __REV_MTECH__
#else
    uint8_t data[5] = { 0 };

    HAL_SRAM_Read_8b(&hsram3, (uint32_t*)(NOR_MEMORY_ADRESS3 + TMC457_SOLENOID_VALVE), data, 5);
    return (data[1] & 0xFF) == 0x01 ? APC_HIGH : APC_LOW;
#endif
}

/**
 * check SOL before moving a target position
 * position : Abs. position in step to move
 */
void setMotorTargetPosition(int position, bool option)
{
    bool sol_status = (option == APC_HIGH) ? getSolenoidStatus() : APC_HIGH;

    if (sol_status == APC_LOW) {
        setSolenoidStatus(APC_HIGH);
        sol_status = getSolenoidStatus();
    }

    if (sol_status == APC_HIGH) {
        writeTMC457(TMC457_XTARGET, position, 4);
        // XXX caller have to get delay
        //APC_Delay(SET_DELAY);
    }
}

int getMotorTargetPosition(void)
{
    ramp_generator_register.x_target = readTMC457(TMC457_XTARGET, 4);
    return ramp_generator_register.x_target;
}

void setMotorPosition(int position)
{
    writeTMC457(TMC457_XACTUAL, position, 4);
    APC_Delay(SET_DELAY);
}

int getMotorPosition()
{
    ramp_generator_register.x_actual = readTMC457(TMC457_XACTUAL, 4);
    return ramp_generator_register.x_actual;
}

void setEncoderPosition(int position)
{
    writeTMC457(TMC457_ENC_X, position, 4);
    APC_Delay(SET_DELAY);
}

int getEncoderPosition()
{
    encoder_register.enc_x = readTMC457(TMC457_ENC_X, 4);
    return encoder_register.enc_x;
}

/*
 * Target velocity
 * The sign determines the direction in velocity mode and hold mode.
 * : ± $7FFF0000 for any a_max [µsteps / t]
 */
void setMotorVelocity(int velocity)
{
    writeTMC457(TMC457_VTARGET, velocity, 4);
    APC_Delay(SET_DELAY);
}

int getMotorVelocity()
{
    ramp_generator_register.v_target = readTMC457(TMC457_VTARGET, 4);
    return ramp_generator_register.v_target;
}

void setMotorPosVelocity(uint32_t velocity)
{
    writeTMC457(TMC457_VMAX, velocity, 4);
    APC_Delay(SET_DELAY);
}

uint32_t getMotorPosVelocity(void)
{
    ramp_generator_register.v_max = readTMC457(TMC457_VMAX, 4);
    return ramp_generator_register.v_max;
}

uint32_t getMotorMaxVelocity(void)
{
    return _maxVelocity;
}

uint32_t getMotorMaxAcc(void)
{
    return _maxAcc;
}

void setMotorPosAcceleration(uint32_t accel)
{
    writeTMC457(TMC457_AMAX_DMAX, accel, 3);
    APC_Delay(SET_DELAY);
}

uint32_t getMotorPosAcceleration(void)
{
    ramp_generator_register.a_max_d_max = readTMC457(TMC457_AMAX_DMAX, 3);
    return ramp_generator_register.a_max_d_max;
}

uint8_t getMotorStatus(void)
{
    ramp_generator_register.status.all =  (uint8_t) readTMC457(TMC457_STATUS,  1);
    return ramp_generator_register.status.all;
}


int readTMC457(uint8_t address, uint8_t length)
{
#ifdef __REV_MTECH__
#else
    uint8_t data[5] = { 0 };
    HAL_SRAM_Read_8b(&hsram3, (uint32_t*)(NOR_MEMORY_ADRESS3 + address), data, length);

    int val = 0;
    if (length == 1) {
        val |= data[0];
    } else if (length == 2) {
        val |= data[0];
        val |= data[1] << 8;
    } else if (length == 3) {
        val |= data[0];
        val |= data[1] << 8;
        val |= data[2] << 16;
    } else if (length == 4) {
        val |= data[0];
        val |= data[1] << 8;
        val |= data[2] << 16;
        val |= data[3] << 24;
    }
    return val;
#endif
}

bool writeTMC457(uint8_t address, int value, uint8_t length)
{
#ifdef __REV_MTECH__
#else
    uint8_t data[5] = { 0 };

    if (length == 1) {
        data[0] = value & 0xFF;
    } else if (length == 2) {
        data[0] = value & 0xFF;
        data[1] = value >> 8;
    } else if (length == 3) {
        data[0] = value & 0xFF;
        data[1] = value >> 8;
        data[2] = value >> 16;
    } else if (length == 4) {
        data[0] = value & 0xFF;
        data[1] = value >> 8;
        data[2] = value >> 16;
        data[3] = value >> 24;
    }
    HAL_StatusTypeDef status = HAL_SRAM_Write_8b(&hsram3, (uint32_t*)(NOR_MEMORY_ADRESS3 + address), data, length);
    return status == HAL_OK ? true : false;
#endif
}

#ifdef __REV_MTECH__
#else
HAL_StatusTypeDef HAL_SRAM_Read_8b_Custom(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint8_t *pDstBuffer, uint32_t BufferSize)
{
  uint32_t size;
  __IO uint8_t *psramaddress = (uint8_t *)pAddress;
  uint8_t *pdestbuff = pDstBuffer;
  HAL_SRAM_StateTypeDef state = hsram->State;

  /* Check the SRAM controller state */
  if ((state == HAL_SRAM_STATE_READY) || (state == HAL_SRAM_STATE_PROTECTED))
  {
    /* Process Locked */
    __HAL_LOCK(hsram);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_BUSY;

    /* Read data from memory */
    for (size = BufferSize; size != 0U; size--)
    {
      *pdestbuff = *psramaddress;
      pdestbuff++;
      psramaddress++;
    }

    /* Update the SRAM controller state */
    hsram->State = state;

    /* Process unlocked */
    __HAL_UNLOCK(hsram);
  }
  else
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

HAL_StatusTypeDef HAL_SRAM_Write_8b_Custom(SRAM_HandleTypeDef *hsram, uint32_t *pAddress, uint8_t *pSrcBuffer, uint32_t BufferSize)
{
  uint32_t size;
  __IO uint8_t *psramaddress = (uint8_t *)pAddress;
  uint8_t *psrcbuff = pSrcBuffer;

  /* Check the SRAM controller state */
  if (hsram->State == HAL_SRAM_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hsram);

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_BUSY;

    /* Write data to memory */
    for (size = BufferSize; size != 0U; size--)
    {
      *psramaddress = *psrcbuff;
      psrcbuff++;
      psramaddress++;
    }

    /* Update the SRAM controller state */
    hsram->State = HAL_SRAM_STATE_READY;

    /* Process unlocked */
    __HAL_UNLOCK(hsram);
  }
  else
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}
#endif
