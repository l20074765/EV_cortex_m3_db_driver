/****************************************Copyright (c)*************************************************
**                      Fujian Junpeng Communicaiton Technology Co.,Ltd.
**                               http://www.easivend.com.cn
**--------------File Info------------------------------------------------------------------------------
** File name:           MDB.h
** Last modified Date:  2013-01-08
** Last Version:         
** Descriptions:        MDB通讯协议                     
**------------------------------------------------------------------------------------------------------
** Created by:          sunway
** Created date:        2013-01-08
** Version:             V0.1
** Descriptions:        The original version       
********************************************************************************************************/
#ifndef __MDB_H 
#define __MDB_H

void MdbBusHardwareReset(void);	//未测试,暂不要用
uint8_t MdbConversation(uint8_t Dev,uint8_t *SeDat,uint8_t SenDatLen,uint8_t *RevDat,uint8_t *RevLen);	


#endif
/**************************************End Of File*******************************************************/
