#include <stdio.h>
#include <string.h>
#include "netx_drv.h"


#define NEWLINE "\r\n"

static int console_kbhit(void);
static int console_getchar(void);

static uint32_t terminal_Read(void);

uint32_t Terminal_GetCommand(char* pszCmd, uint32_t ulCmdBufferSize);

static char   abKeyboardBuffer[256];
static int    iKeyboardBufferIdxWrite = 0;

extern DRV_UART_HANDLE_T g_tUART;


/**************************************************************************************/
uint32_t Terminal_GetCommand(char* pszCmd, uint32_t ulCmdBufferSize)
{
  uint32_t  ulCmdSize = 0;

  ulCmdSize = terminal_Read();
  if (0 != ulCmdSize)
  {
    if ((NULL != pszCmd) && (ulCmdSize <= ulCmdBufferSize))
    {
      memcpy(pszCmd, abKeyboardBuffer, ulCmdSize);
    }
    else
    {
      ulCmdSize = 0;
    }
  }

  return ulCmdSize;
}


/**************************************************************************************/
static uint32_t terminal_Read(void)
{
  uint32_t ulCmdSize = 0;
  int      iKey      = 0;

  if(console_kbhit())
  {
    iKey = console_getchar();

    switch (iKey)
    {
      case '\r':
      case '\n':
      {
        abKeyboardBuffer[iKeyboardBufferIdxWrite++] = 0x00;
        ulCmdSize = iKeyboardBufferIdxWrite;
        iKeyboardBufferIdxWrite = 0;
        printf(NEWLINE);
      }
      break;

      case '\b':
      {
         if (0 != iKeyboardBufferIdxWrite)
         {
           iKeyboardBufferIdxWrite -= 1;
           /* move cursor back and delete last char in console */
           printf("\b \b");
         }
      }
      break;

      default:
      {
        abKeyboardBuffer[iKeyboardBufferIdxWrite++] = iKey;
        putchar(iKey);    /* echo character */

        if ( sizeof(abKeyboardBuffer) == (iKeyboardBufferIdxWrite+1))
        { /* buffer is full start from beginning */
          iKeyboardBufferIdxWrite = 0;
        }
      }
      break;
    }
  }

  return ulCmdSize;
}


/*****************************************************************************/
/*! Tests if any data in Terminal Uart FIFO
 *   \return   0 -- FIFO is empty
 1 -- data in FIFO                                                           */
/*****************************************************************************/
static int console_kbhit(void)
{
  DRV_UART_STATE_E eState;

  while(DRV_OK != DRV_UART_GetState(&g_tUART, &eState))
    ;

  if((eState & DRV_UART_STATE_RXFE) != 0) /* check if there is data in the FIFO  */
    return 0; /* no, FIFO is empty  */
  else
    return 1; /* yes, data in FIFO  */
}

/**************************************************************************************/
/*! Grab a character from the keyboard
 *  This will block if no character is in the terminal/keyboard input buffer
 *  until a key is pressed.
 *
 *  \return The key read from the keyboard buffer                                     */
/**************************************************************************************/
static int console_getchar(void)
{
  return getchar();
}


