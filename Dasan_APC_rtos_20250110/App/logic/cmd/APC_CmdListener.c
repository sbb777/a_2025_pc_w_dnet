/*
 * APC_CmdListener.c
 *
 *  Created on: 2023. 10. 23.
 *      Author: Yongseok
 */

#include <APC_CmdListener.h>

#include <APC_Utils.h>
#include <APC_AccessMode.h>
#include <APC_CmdHandler.h>
#include <APC_Error.h>
#include <APC_LocalPort.h>
#include "APC_RemotePort.h"

#ifdef __REV_MTECH__
extern UART_HandleTypeDef huart4;
#endif

#define ASCII_BS            0x08
#define ASCII_LF            0x0A    // \n
#define ASCII_CR            0x0D    // \r
#define ASCII_COLON         0x3A    // :
#define ASCII_DEL           0x7F

#define ERR_FORMAT          "E:%06d\r\n"
#define CMD_BUF_SIZE        128

typedef struct {
    uint16_t buf_index;
    char     buf[CMD_BUF_SIZE];
} CommandBufferType;

CommandBufferType _local;
CommandBufferType _remote;

static bool _listener_initialized = false;

static bool _trace_option =  false;


void procLocalCmd(void);
void procRemoteCmd(void);

void initLocalListener(void);
void initRemoteListener(void);

void procCmd(uint8_t ch, CommandBufferType buffer);
void printRS232(uint8_t ch, char *err_fmt, uint32_t err_code);

void initCmdListener(void)
{
    initLocalListener();
    initRemoteListener();

    _listener_initialized = true;
}

void initLocalListener(void)
{
    _local.buf_index = 0;
    memset((char*) _local.buf,  0x00, (int) sizeof(_local.buf));

    initLocalPort();
}

void initRemoteListener(void)
{
    _remote.buf_index = 0;
    memset((char*) _remote.buf,  0x00, (int) sizeof(_remote.buf));

    initRemotePort();
}

/*
 * every second called by main
 */
void procCmdListener(uint32_t counter)
{
	if (_listener_initialized != true)   return;

	refreshAccessMode(APC_PORT_INTERNAL, counter);

	if (availableLocalRS232(APC_PORT_LOCAL) > 0)
	{
	    refreshAccessMode(APC_PORT_LOCAL, counter);
	    procLocalCmd();
	    //procCmd(APC_PORT_LOCAL, _local);
	}

	/*
	 * REMOTE RS-232 huart4
	 */
#ifdef __REV_MTECH__
	if(__HAL_UART_GET_FLAG(&huart4, UART_FLAG_RXNE) == SET)
	{
		// ???
    refreshAccessMode(APC_PORT_REMOTE, counter);
    procRemoteCmd();
	}
#else
	if (availableRemoteRS232(APC_PORT_REMOTE) > 0)
	{
        refreshAccessMode(APC_PORT_REMOTE, counter);
        procRemoteCmd();
        //procCmd(APC_PORT_REMOTE, _remote);
	}
#endif
}

void procLocalCmd()
{
    uint8_t rx_data = readLocalRS232Data(APC_PORT_LOCAL);

//    if (rx_data == ASCII_BS)
//    {
//        if (_local.buf_index > 0) {
//            _local.buf_index--;
//            _local.buf[_local.buf_index] = 0x00;
//        }
//    }
    if (rx_data == ASCII_LF)
    {
        if (_local.buf_index < 3)
        {
            if (_local.buf_index == 2)
            {
                if (_local.buf[0] == ASCII_COLON && _local.buf[1] == ASCII_CR)
                {
                    printLocalRS232(APC_PORT_LOCAL, ERR_FORMAT, Format_WrongNumberCharacters);
                }
                else if (_local.buf[0] != ASCII_COLON && _local.buf[1] == ASCII_CR)
                {
                    printLocalRS232(APC_PORT_LOCAL, ERR_FORMAT, Format_ColonMissing);
                }
                else
                {
                    printLocalRS232(APC_PORT_LOCAL, ERR_FORMAT, Format_InvalidCommandCharacter);
                }
            }
        }
        else
        {
            //_local.buf[_local.buf_index] = 0x00;
            if (_local.buf[1] != ASCII_COLON)
            {
                printLocalRS232(APC_PORT_LOCAL, ERR_FORMAT, Format_ColonMissing);
            }
            else if (_local.buf[_local.buf_index-1] == ASCII_CR)
            {
                _local.buf[_local.buf_index-1] = 0x00;

                char rsp_buf[CMD_BUF_SIZE] = { 0x00 };
                int cmd_result = doCmdHandler(APC_PORT_LOCAL, _local.buf, rsp_buf);

                if (cmd_result == CMD_PROCESSING_OK)
                {
                    strcat(rsp_buf, "\r\n");
                    printLocalRS232(APC_PORT_LOCAL, rsp_buf);
                }
                else if (cmd_result > CMD_PROCESSING_OK)
                {
                    printLocalRS232(APC_PORT_LOCAL, ERR_FORMAT, cmd_result);
                }
                else
                {
                    printLocalRS232(APC_PORT_LOCAL, ERR_FORMAT, 999999);
                }
            }
            else
            {
                printLocalRS232(APC_PORT_LOCAL, ERR_FORMAT, Format_TerminationCharactorMissing);
            }
        }

        //initCmdListener();
        for (int i = 0; i < _local.buf_index; i++)
            _local.buf[i] = 0x00;
        _local.buf_index = 0;
    }
    else if (_local.buf_index < (CMD_BUF_SIZE - 1))
    {
        _local.buf[_local.buf_index] = rx_data;
        _local.buf_index++;
    }
    else
    {
        printLocalRS232(APC_PORT_LOCAL, ERR_FORMAT, Format_CommandInterpretationFailure);
        prtLog(0, LL_WARN, __FUNCTION__, "cmd_buf_index=[%d] over. cmd_buf=[%s]\n",
                _local.buf_index, _local.buf);
        initLocalListener();
    }
}

void procRemoteCmd(void)
{
    uint8_t rx_data = readRemoteRS232Data();
    if (rx_data == 0x00)    return;

    if (rx_data == ASCII_LF)
    {
        if (_remote.buf_index < 3)
        {
            if (_remote.buf_index == 2)
            {
                if (_remote.buf[0] == ASCII_COLON && _remote.buf[1] == ASCII_CR)
                {
                    printfRemoteRS232(ERR_FORMAT, Format_WrongNumberCharacters);
                }
                else if (_remote.buf[0] != ASCII_COLON && _remote.buf[1] == ASCII_CR)
                {
                    printfRemoteRS232(ERR_FORMAT, Format_ColonMissing);
                }
                else
                {
                    printfRemoteRS232(ERR_FORMAT, Format_InvalidCommandCharacter);
                }
            }
        }
        else
        {
            //_remote.buf[_remote.buf_index] = 0x00;
            if (_remote.buf[1] != ASCII_COLON)
            {
                printfRemoteRS232(ERR_FORMAT, Format_ColonMissing);
            }
            else if (_remote.buf[_remote.buf_index-1] == ASCII_CR)
            {
                _remote.buf[_remote.buf_index-1] = 0x00;

                char rsp_buf[CMD_BUF_SIZE] = { 0x00 };
                int cmd_result = doCmdHandler(APC_PORT_REMOTE, _remote.buf, rsp_buf);

                if (cmd_result == CMD_PROCESSING_OK)
                {
                    if (_trace_option == true)  printLocalRS232(APC_PORT_LOCAL, _remote.buf);

                    strcat(rsp_buf, "\r\n");
                    printfRemoteRS232(rsp_buf);

                    if (_trace_option == true)  printLocalRS232(APC_PORT_LOCAL, rsp_buf);
                }
                else if (cmd_result > CMD_PROCESSING_OK)
                {
                    printfRemoteRS232(ERR_FORMAT, cmd_result);
                }
                else
                {
                    printfRemoteRS232(ERR_FORMAT, 999999);
                }
            }
            else
            {
                printfRemoteRS232(ERR_FORMAT, Format_TerminationCharactorMissing);
            }
        }

        //initCmdListener();
        for (int i = 0; i < _remote.buf_index; i++)
            _remote.buf[i] = 0x00;
        _remote.buf_index = 0;
    }
    else if (_remote.buf_index < (CMD_BUF_SIZE - 1))
    {
        _remote.buf[_remote.buf_index] = rx_data;
        _remote.buf_index++;
    }
    else
    {
        printfRemoteRS232(ERR_FORMAT, Format_CommandInterpretationFailure);
        initRemoteListener();
    }
}

void procCmd(uint8_t ch, CommandBufferType buffer)
{
    uint8_t rx_data = ch == APC_PORT_LOCAL ? readLocalRS232Data(APC_PORT_LOCAL)
            : (ch == APC_PORT_REMOTE ? readRemoteRS232Data() : 0x00);

    if (rx_data == 0x00)    return;

    if (rx_data == ASCII_LF)
    {
        if (buffer.buf_index < 3)
        {
            if (buffer.buf_index == 2)
            {
                if (buffer.buf[0] == ASCII_COLON && buffer.buf[1] == ASCII_CR)
                {
                    printRS232(ch, ERR_FORMAT, Format_WrongNumberCharacters);
                }
                else if (buffer.buf[0] != ASCII_COLON && buffer.buf[1] == ASCII_CR)
                {
                    printRS232(ch, ERR_FORMAT, Format_ColonMissing);
                }
                else
                {
                    printRS232(ch, ERR_FORMAT, Format_InvalidCommandCharacter);
                }
            }
        }
        else
        {
            //buffer.buf[buffer.buf_index] = 0x00;
            if (buffer.buf[1] != ASCII_COLON)
            {
                printRS232(ch, ERR_FORMAT, Format_ColonMissing);
            }
            else if (buffer.buf[buffer.buf_index-1] == ASCII_CR)
            {
                buffer.buf[buffer.buf_index-1] = 0x00;

                char rsp_buf[CMD_BUF_SIZE] = { 0x00 };
                int cmd_result = doCmdHandler(ch, buffer.buf, rsp_buf);

                if (cmd_result == CMD_PROCESSING_OK)
                {
                    strcat(rsp_buf, "\r\n");
                    if (ch == APC_PORT_LOCAL)   printLocalRS232(APC_PORT_LOCAL, rsp_buf);
                    else                        printfRemoteRS232(rsp_buf);
                }
                else if (cmd_result > CMD_PROCESSING_OK)
                {
                    printRS232(ch, ERR_FORMAT, cmd_result);
                }
                else
                {
                    printRS232(ch, ERR_FORMAT, 999999);
                }
            }
            else
            {
                printRS232(ch, ERR_FORMAT, Format_TerminationCharactorMissing);
            }
        }

        //initCmdListener();
        for (int i = 0; i < buffer.buf_index; i++)
            buffer.buf[i] = 0x00;
        buffer.buf_index = 0;
    }
    else if (buffer.buf_index < (CMD_BUF_SIZE - 1))
    {
        buffer.buf[buffer.buf_index] = rx_data;
        buffer.buf_index++;
    }
    else
    {
        printRS232(ch, ERR_FORMAT, Format_CommandInterpretationFailure);

        prtLog(0, LL_WARN, __FUNCTION__, "cmd_buf_index=[%d] over. cmd_buf=[%s]\n",
                buffer.buf_index, buffer.buf);

        if (ch == APC_PORT_LOCAL)
            initLocalListener();
        else
            initRemoteListener();
    }
}

void printRS232(uint8_t ch, char *err_fmt, uint32_t err_code)
{
    if (ch == APC_PORT_LOCAL)
        printLocalRS232(ch, err_fmt, err_code);
    else
        printfRemoteRS232(err_fmt, err_code);
}

void setRemoteTrace(bool option)
{
    _trace_option = option;
}
