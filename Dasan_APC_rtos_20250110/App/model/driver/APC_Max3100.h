/*
 * APC_Max3100.h
 *
 *  Created on: 2023. 11. 30.
 *      Author: Yongseok
 */

#ifndef _APC_MAX3100_H_
#define _APC_MAX3100_H_

#include <APC_Define.h>

typedef struct
{
    uint8_t B0B3;     // Baud-Rate Divisor Select Bits. (4-bit baud clock value)
    bool CTS;         // Clear-to-Send-Input. (CTS bit = 0 implies CTS_ pin = logic high.)
    uint8_t D0tD7t;   // Transmit-Buffer Register.
                      // Eight data bits written into the transmit-buffer register. D7t is ignored when L=1.
    uint8_t D0rD7r;   // Eight data bits read from the receive FIFO or the receive register.
                      // all 0s when the receive FIFO or the receive registers are empty. When L=1, D7r is always 0.
    bool FEN_;        // FIFO Enable.
    bool IR;          // IrDA bit.
    bool L;           // Bit for setting the word length of the transmitted or received data.
                      // L=0 results in 8-bit words(9-bit words if PE=1). L=1 results in 7-bit words (8-bit words if PE=1).
    bool Pt;          // Transmit-Parity Bit. This bit is treated as an extra bit that will be transmitted if PE=1.
                      // To be useful in 9-bit networks, the MAX3100 does not calculate parity.
    bool Pr;          // Receive-Parity Bit. This bit is the extra bit received if PE=1.
                      // PE=1 results in 9-bit transmissions (L=0). If PE=0, then Pr is set to 0.
    bool PE;          // Parity-Enable Bit.
    bool PM_;         // Mask for Pr bit.
    bool R;           // Receive Bit or FIFO Not Empty Flag.
                      // R=1 means new data is available to be read from the receive register or FIFO.
    bool RM_;         // Mask for R bit. IRQ_ is asserted if RM_=1 and R=1
    bool RAM_;        // Mask for RA/FE bit. IRQ_ is asserted if RAM_=1 and RA/FE=1
    bool RTS;         // Request-to-Send Bit. Controls the state of the RTS_ output.
                      // This bit is reset on power-up (RTS bit = 0 sets the RTS_ pin = logic high).
    bool RA_FE;       // Receiver-Activity/Framing-Error Bit.
                      //
    bool SHDNi;       // Software-Shutdown Bit. Enter software shutdown with a write configuration where SHDNi = 1.
                      // Software shutdown takes effect after CS goes high, and causes the oscillator to stop as soon as the transmitter becomes idle.
    bool SHDNo;       // Shutdown Read-Back Bit. The read-configuration register outputs SHDNo = 1 when the UART is in shutdown.
                      //
    bool ST;          // Transmit-Stop Bit.
                      // One stop bit will be transmitted when ST=0. Two stop bits will be transmitted when ST=1. The receiver only requires one stop bit.
    bool T;           // Transmit-Buffer-Empty Flag.
                      // T=1 means that the transmit buffer is empty and ready to accept another data word.
    bool TE_;         // Transmit-Enable Bit.
                      // If TE_=1, then only the RTS_ pin will be updated on CS_’s rising edge.
                      // The contents of RTS_, Pt, and D0t–D7t transmit on CS_’s rising edge when TE_=0.
    bool TM_;         // Mask for T bit. IRQ_ is asserted if TM_=1 and T=1.
} Max3100ConfigType;

bool initMax3100(uint32_t b_rate, uint8_t d_length, uint8_t stop_bit);

bool availableMax3100Transmit(void);
bool availableMax3100Receive(void);

void     setMax3100Baudrate(uint32_t rate);
uint32_t getMax3100Baudrate(void);

int  readMax3100Data(void);
void writeMax3100Data(uint8_t data);

void     setMax3100Config(uint16_t config);
uint16_t getMax3100Config(void);
void     setMax3100Data(uint16_t data);
uint16_t getMax3100Data(void);

#endif /* _APC_MAX3100_H_ */
