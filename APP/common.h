/****************************************Copyright (c)*************************************************
**                      Fujian Junpeng Communicaiton Technology Co.,Ltd.
**                               http://www.easivend.com.cn
**--------------File Info------------------------------------------------------------------------------
** File name:           common.h
** Last modified Date:  2013-01-06
** Last Version:        No
** Descriptions:        系统参数定义及通用函数部分                     
**------------------------------------------------------------------------------------------------------
** Created by:          sunway
** Created date:        2013-01-06
** Version:             V0.1
** Descriptions:        The original version        
********************************************************************************************************/
#ifndef __COMMON_H 
#define __COMMON_H


#include "..\API\billApi.h"
#include "..\API\coinApi.h"

#define print_log(...)	




//==============================定时器计数变量结构体========================================================

typedef struct _timer_st_{

	volatile unsigned char secTimer;//定义秒级 定时

//==============================10毫秒级定时器变量=====================================================================
	volatile unsigned short printTimer;
	volatile unsigned short pcm_handle_timeout;
	volatile unsigned short checkDeviceTimeout;
	volatile unsigned short HpHandleTimer;
	volatile unsigned int bill_comunication_timeout;//MDB通讯超时时间
	volatile unsigned short led_paoma_timer;
	volatile unsigned char  user_led_green;

//==============================秒级定时器变量=====================================================================
	volatile unsigned short sec_usr_op_timer;//用户操作超时	
	volatile unsigned short sec_changer_timer;
	volatile unsigned short sec_hopper_state_timer;//hopper  定时复位
	volatile unsigned short sec_pccon_timer;//处理PC命令超时
	volatile unsigned char	PayoutTimer;//出币等待定时器


}TIMER_ST;
extern TIMER_ST Timer;


/************************************************************************************
*系统参数结构体
***************************************************************************************/
typedef struct _system_para_{

	//硬币器类型: 高四位表示硬币找零器 0-无 1-hopper 2-MDB
	//			  低四位表示硬币接收器 0无  1-串行   2-MDB  3-并行
	unsigned char coinType;

	//纸币器类型: 高四位表示纸币找零器 0-无 1-预留 2-MDB
	//			  低四位表示纸币接收器 0无  1-预留 2-MDB  
	unsigned char billType; 
	unsigned char traceFlag;//调试开关
			


}SYSTEM_PARA;
extern SYSTEM_PARA sysPara;



/************************************************************************************
*设备 结构体
***************************************************************************************/
typedef struct _st_dev_{
	ST_DEV_BILL bill;//纸币器结构体
	ST_DEV_COIN coin;//硬币器结构体
	uint32_t hpBaseChange;//hopper 结构体
	uint16_t HpValue[3];

}ST_DEV;

extern ST_DEV stDev;


/************************************************************************************
*总交易数据结构体
***************************************************************************************/
typedef struct _st_total_trade_sum_
{	
	unsigned int BillMoneySum;//收入纸币总金额	
	unsigned int CoinMoneySum;//收入硬币总金额	
	unsigned int DispenceSum;//找零总金额	
	unsigned int Hopper1DispSum;//00地址Hopper找零总数量	
	unsigned int Hopper2DispSum;//01地址Hopper找零总数量	
	unsigned int Hopper3DispSum;//10地址Hopper找零总数量
	unsigned int tradePageNo;//交易页号 0 表示没有交易记录 
	unsigned int tradeNums;//总交易数量
	unsigned int iouAmount;//欠费金额
	unsigned int tradeAmount;//交易总金额


}ST_TOTAL_TRADE_SUM;

extern ST_TOTAL_TRADE_SUM stTotalTrade;


/************************************************************************************
*邮箱结构体
***************************************************************************************/
//纸币器任务通讯结构体
typedef struct _task_msg_bill_{

	 unsigned char type;
	 unsigned char status;
	 unsigned int  amount;
	 unsigned int  change;

}TASK_MSG_BILL;

//硬币器任务通讯结构体
typedef struct _task_msg_coin_{

	 unsigned char type;
	 unsigned char status;
	 unsigned int  amount;
	 unsigned int  change;


}TASK_MSG_COIN;


//hopper 任务通讯结构体
typedef struct _task_msg_hopper_{

   	 unsigned char value8;
	 unsigned short value16;
	 unsigned int  value32;

}TASK_MSG_HOPPER;
//定义主任务与设备任务之间通信的结构体
typedef struct
{
    unsigned char   type;				//命令类型
	unsigned char   subType;		   //子集命令类型
	TASK_MSG_BILL 	bill;
	TASK_MSG_COIN 	coin;
	TASK_MSG_HOPPER hopper;

	
} MAIN_DEV_TASK_MSG;




//任务通信邮箱的对发邮箱句柄
extern OS_EVENT *g_msg_main_to_dev;
extern OS_EVENT *g_msg_dev_to_main;
extern MAIN_DEV_TASK_MSG task_msg_main_to_dev;
extern MAIN_DEV_TASK_MSG task_msg_dev_to_main;


//纸币器邮箱
extern OS_EVENT *g_billIN;
typedef struct _bill_recv_msg_{
	unsigned char channel;
	unsigned int  value;
	
}BILL_RECV_MSG;
#define G_BILL_IN_SIZE    20  
extern BILL_RECV_MSG bill_recv_msg[G_BILL_IN_SIZE];


//硬币器通信消息队列
#define G_COIN_IN_SIZE    100
extern OS_EVENT *g_CoinIn;
extern unsigned char  CoinIn[G_COIN_IN_SIZE];


//初始化任务信号量
extern OS_EVENT *g_InitSem;


/*---------------------------------------------------------
任务间通信设备任务接受的操作指令集合
-----------------------------------------------------------*/
#define TASK_DISABLE           	1 //主任务发送禁能 请求

#define TASK_ENABLE            	3 //主任务发送使能 请求
 	 
#define TASK_HOPPER         	5 //主任务发送hopper 请求

#define TASK_INIT		    	7//主任务发送初始化请求

#define TASK_CHANGER			9 //找零请求

#define TASK_OVER				0xAA
#define TASK_NOT_ACK			0x0A
//请求对象
#define OBJ_BILL					0x0001
#define OBJ_COIN					0x0002
#define OBJ_HOPPER					0x0004

#define OBJ_ALL						0xFFFF



//================================系统故障码======================================================

#define SYS_ERR_NO_NORMAL  			(unsigned short)(0)  //正常
#define SYS_ERR_NO_HOPPER  			(unsigned short)(0x01 << 0)  //Hopper故障
#define SYS_ERR_NO_BILL    			(unsigned short)(0x01 << 1)  //纸币器故障
#define SYS_ERR_NO_HUODAO_PRICE   	(unsigned short)(0x01 << 2)  //货道自检全部货道单价为零
#define SYS_ERR_NO_HUODAO_EMPTY   	(unsigned short)(0x01 << 3)  //货道系统参数中的储货量全为0
#define SYS_ERR_NO_HUODAO_FAULT  	(unsigned short)(0x01 << 4)  //货道自检全部货道不可用
#define SYS_ERR_NO_COIN				(unsigned short)(0x01 << 5)  //硬币器故障
#define SYS_ERR_NO_CHANGER_FAULT 	(unsigned short)(0x01 << 6)  //非hopper找零器故障

//================================hopper故障码======================================================

#define HP_ERR_NO_NORMAL  		 ((unsigned short)(0))//正常
#define HP_ERR_NO_HOPPER1_FAULT  (unsigned short)(0x01 << 0)//hopper1故障
#define HP_ERR_NO_HOPPER1_EMPTY  (unsigned short)(0x01 << 1)//hopper1缺币
#define HP_ERR_NO_HOPPER2_FAULT  (unsigned short)(0x01 << 2)//hopper2故障
#define HP_ERR_NO_HOPPER2_EMPTY  (unsigned short)(0x01 << 3)//hopper2缺币
#define HP_ERR_NO_HOPPER3_FAULT  (unsigned short)(0x01 << 4)//hopper3故障
#define HP_ERR_NO_HOPPER3_EMPTY  (unsigned short)(0x01 << 5)//hopper3缺币
#define HP_ERR_NO_HOPPER4_FAULT  (unsigned short)(0x01 << 6)//hopper4故障
#define HP_ERR_NO_HOPPER4_EMPTY  (unsigned short)(0x01 << 7)//hopper4缺币



//hopper故障码
extern unsigned int hopperErrNo;
//售货机故障码
extern uint16_t HardWareErr;


//======================================================================================
unsigned char XorCheck(unsigned char *pstr,unsigned short len);
unsigned short CrcCheck(unsigned char *msg, unsigned short len);
void msleep(unsigned int msec);
unsigned char mbox_post_main_to_dev(unsigned char type);
unsigned char mbox_post_dev_to_main(unsigned char type);
void CreateCommonMBox(void);



#endif
/**************************************End Of File*******************************************************/
