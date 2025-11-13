/*
 * APC_CmdListener.h
 *
 *  Created on: 2023. 10. 23.
 *      Author: Yongseok
 */

#ifndef _APC_CMDLISTENER_H_
#define _APC_CMDLISTENER_H_

#include <APC_Define.h>


void initCmdListener(void);
void procCmdListener(uint32_t counter);

void setRemoteTrace(bool option);

#endif /* _APC_CMDLISTENER_H_ */
