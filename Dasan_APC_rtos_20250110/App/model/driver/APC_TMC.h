/*
 * APC_TMC.h
 *
 *  Created on: 2024. 1. 2.
 *      Author: Yongseok
 */

#ifndef _APC_TMC_H_
#define _APC_TMC_H_

#include <APC_Define.h>

// TMC457 Register Set
#define TMC457_MODE                 0x00    //0x00
#define TMC457_XACTUAL              0x02    //0x01
#define TMC457_VACTUAL              0x06    //0x02
#define TMC457_VMAX                 0x0A    //0x03
#define TMC457_VTARGET              0x0E    //0x04
#define TMC457_AMAX                 0x12    //0x05
#define TMC457_DMAX                 0x15    //0x06
#define TMC457_DSTOP                0x18    //0x07
#define TMC457_BOWMAX               0x1B    //0x08
#define TMC457_XTARGET              0x1C    //0x09
#define TMC457_XCOMPARE             0x20    //0x0A
#define TMC457_STATUS               0x24    //0x0B
#define TMC457_AACTUAL              0x25    //0x0C
#define TMC457_SDSCALE              0x29    //0x0D
#define TMC457_AMAX_DMAX            0x2B    //0x0F

#define TMC457_ENC_CONST            0x30    //0x10
#define TMC457_ENC_X                0x34    //0x11
#define TMC457_ENC_MODE             0x38    //0x12
#define TMC457_ENC_STATUS           0x3A    //0x13
#define TMC457_ENC_LATCH            0x3B    //0x14
#define TMC457_XLATCH               0x3F    //0x15
#define TMC457_ENC_WARNDIST         0x43    //0x16
#if 0
#define TMC457_VENC_US_CONST        0x17    // not defined
#define TMC457_VENC_US_POS          0x18    // not defined
#define TMC457_VENC_US_SEL          0x19    // not defined
#endif
#define TMC457_PID_P                0x46    //0x20
#define TMC457_PID_I                0x49    //0x21
#define TMC457_PID_D                0x4C    //0x22
#define TMC457_PID_ICLIP            0x4F    //0x23
#define TMC457_PID_ISUM             0x51    //0x24
#define TMC457_PID_DCLKDIV          0x55    //0x25
#define TMC457_PID_DV_CPU           0x26    // not defined
#define TMC457_PID_DV_CLIP          0x56    //0x27
#define TMC457_PID_E                0x5A    //0x28
#define TMC457_PID_VACTUAL          0x5E    //0x29
#define TMC457_PID_TOLERANCE        0x62    //0x2A
#if 0
#define TMC457_PULSE_MAX            0x30    // not defined
#define TMC457_PULSE_XSTEP_DIV      0x31    // not defined
#define TMC457_STEP_DIR_MODE        0x32    // not defined
#define TMC457_MICROSTEP_POS        0x33    // not defined
#define TMC457_STDBY_DELAY          0x34    // not defined
#define TMC457_PULSE_LENGTH         0x35    // not defined
#endif
#define TMC457_SWITCH_MODE          0x65    //0x40
#define TMC457_SWITCH_STATUS        0x67    //0x41
#define TMC457_POSLIM_LEFT          0x68    //0x42
#define TMC457_POSLIM_RIGHT         0x6C    //0x43

#define TMC457_SEQ_MODE             0x70    //0x54
#define TMC457_SEQ_DAC_SCALE        0x72    //0x55
#define TMC457_SEQ_STALL_THRESHOLD  0x56    // not defined
#define TMC457_SEQ_DRIVER_STATUS    0x75    //0x57
#define TMC457_CHOP_CLK_DIV         0x76    //0x59

#define TMC457_VERSION              0x78    //0x60
#define TMC457_INT_MASK             0x7B    //0x61
#define TMC457_INT_FLAGS            0x7C    //0x62
#define TMC457_WAVETAB              0x7F    // not defined
#define TMC457_SOLENOID_VALVE       0x80    // new

// Write Bit (to be combined with any register address for write access)
#define TMC457_WRITE 0x80

// TMC457 modes (for TMC457_MODE register)
#define TMC457_RM_POSITION  0x00000000
#define TMC457_RM_RESERVED  0x00000001
#define TMC457_RM_VELOCITY  0x00000002
#define TMC457_RM_HOLD      0x00000003
#define TMC457_RM_SHAFT     0x00000010  // to be combined with the other modes
#define TMC457_RM_PID       0x00000100  // to be combined with the other modes
#define TMC457_RM_PID_BASE  0x00000200  // to be combined with the other modes

// Status bits (TMC457_STATUS register)
#define TMC457_ST_X_REACHED  0x00000001
#define TMC457_ST_V_REACHED  0x00000002
#define TMC457_ST_V_ZERO     0x00000004
#define TMC457_ST_ENC_WARN   0x00000010

// Interrupt bits (TMC457_INT_MASK and TMC457_INT_FLAGS registers)
#define TMC457_IRQ_TARGET     0x00000001
#define TMC457_IRQ_DEVIATION  0x00000002
#define TMC457_IRQ_ENCN       0x00000004
#define TMC457_IRQ_STOP       0x00000008
#define TMC457_IRQ_DRVSTATUS  0x00000010
#define TMC457_IRQ_REFL       0x00000020
#define TMC457_IRQ_REFR       0x00000040
#define TMC457_IRQ_XCOMP      0x00000080

// Driver status bits (TMC457_SEQ_DRIVER_STATUS register)
#define TMC457_DRV_ERROR  0x00000001
#define TMC457_DRV_OTPW   0x00000002
#define TMC457_DRV_STALL  0x00000004


struct _mode {
    uint16_t ramp_mode       : 2;
    uint16_t step_dir_enable : 1;
    uint16_t                 : 1;
    uint16_t shaft           : 1;
    uint16_t                 : 3;
    uint16_t PID_on          : 1;
    uint16_t PID_base        : 1;
};
union TMC457_mode {
    uint16_t            all;
    struct _mode        bits;
};

struct _status {
    uint8_t target_pos_reached : 1;
    uint8_t target_v_reached   : 1;
    uint8_t v_is_zero          : 1;
    uint8_t                    : 1;
    uint8_t enc_warn_dist_status : 1;
};
union TMC457_status {
    uint8_t             all;
    struct _status      bits;
};

struct _sd_scale {
    uint16_t    sd_scaler     : 15;
    uint16_t    sd_scale_sign : 1;
};
union TMC457_sd_scale {
    uint16_t            all;
    struct _sd_scale    bits;
};

struct _enc_mode {
    uint16_t        pol_A : 1;
    uint16_t        pol_B : 1;
    uint16_t        pol_N : 1;
    uint16_t    ignore_AB : 1;
    uint16_t    clr_cont  : 1;
    uint16_t    clr_once  : 1;
    uint16_t    pos_edge  : 1;
    uint16_t    neg_edge  : 1;
    uint16_t    clr_enc_x : 1;
    uint16_t              : 3;
    uint16_t x_comp_sel_enc  : 1;
    uint16_t enc_sel_decimal : 1;
};
union TMC457_enc_mode {
    uint16_t            all;
    struct _enc_mode    bits;
};

struct _dacscale_icntl {
    uint32_t    current_op  : 5;
    uint32_t                : 2;
    uint32_t    mixdecay_op : 1;
    uint32_t    current_sb  : 5;
    uint32_t                : 2;
    uint32_t    mixdecay_sb : 1;
};
union TMC457_dacscale_icntl {
    uint32_t                all;
    struct _dacscale_icntl  bits;
};



typedef struct Ramp_Generator_Register_Set {
    union TMC457_mode mode;
    int         x_actual;
    int         v_actual;
    uint32_t    v_max;
    int         v_target;
    uint32_t    a_max;
    uint32_t    d_max;
    uint32_t    d_stop;
    uint8_t     bow_max;
    int         x_target;
    int         x_compare;
    union TMC457_status status;
    int         a_actual;
    union TMC457_sd_scale sd_scale;
    uint32_t    a_max_d_max;
} Ramp_Generator_Register ;

typedef struct Encoder_Register_Set {
    int         enc_const;
    int         enc_x;
    union TMC457_enc_mode enc_mode;
    uint8_t     enc_status;
    uint32_t    enc_latch;
    uint32_t    x_act_latch;
    uint32_t    enc_warn_dist;
} Encoder_Register;

typedef struct Sequencer_Configuration_Register_Set {
    uint16_t    seq_mode;
    union TMC457_dacscale_icntl dacscale_icntl;
    uint8_t     driver_status;
    uint16_t    chop_clk_div;
} Sequencer_Configuration_Register ;


bool initTMC(uint8_t valve_type, uint16_t valve_size);

void refreshTMCValues(void);

void setSynchCondition(bool bMode);
void    setRampMode(uint8_t ramp_mode);
uint8_t getRampMode(void);
void setPIDMode(bool bMode);

void setSolenoidStatus(bool option);
bool getSolenoidStatus(void);

int  getMotorTargetPosition(void);
void setMotorTargetPosition(int posisiton, bool option);

void setMotorPosition(int position);
int  getMotorPosition();

void     setMotorPosVelocity(uint32_t velocity);
uint32_t getMotorPosVelocity(void);
uint32_t getMotorMaxVelocity(void);
uint32_t getMotorMaxAcc(void);

void setMotorVelocity(int velocity);
int  getMotorVelocity();

void     setMotorPosAcceleration(uint32_t accel);
uint32_t getMotorPosAcceleration(void);

void setEncoderPosition(int position);
int  getEncoderPosition(void);

uint8_t getMotorStatus(void);

#endif /* _APC_TMC_H_ */
