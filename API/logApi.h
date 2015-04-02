/****************************************Copyright (c)*************************************************
**                      Fujian Junpeng Communicaiton Technology Co.,Ltd.
**                               http://www.easivend.com.cn
**--------------File Info------------------------------------------------------------------------------
** File name:           common.h
** Last modified Date:  2013-01-06
** Last Version:        No
** Descriptions:        系统参数定义及通用函数部分                     
**------------------------------------------------------------------------------------------------------
** Created by:          yanbo
** Created date:        20134-02-18
** Version:             V0.1
** Descriptions:        The original version        
********************************************************************************************************/
#ifndef __LOG_API_H 
#define __LOG_API_H



#define TRADE_MAX_PAGE   100 //交易记录最大页

void clearTradeInfo(void);

unsigned int amountTurnTocents(unsigned int value);
unsigned int amountTurnByPoint(unsigned int value);

void saveTradeInfo(void);

unsigned char readTradeInfo(void);
void saveDetailTradeInfo(void);
void saveOneTradeInfo(unsigned char columnNo,unsigned int price,unsigned char result);
unsigned char saveSystemParaFromFlash(void);
unsigned char readSystemParaFromFlash(void);



#endif
