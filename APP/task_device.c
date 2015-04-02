#include "task_device.h"



  



/*********************************************************************************************************
** Function name:     	initDeviecByfile
** Descriptions:	   根据配置文件初始化设备
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void initDeviecByfile(void)
{	

	sysPara.billType = VMC_BILL_TYPE;
	sysPara.coinType= VMC_COIN_TYPE;
	sysPara.traceFlag = VMC_TRACE_FLAG;
	
}




/*********************************************************************************************************
** Function name:       SystemParaInit
** Descriptions:        系统参数初始化
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void SystemParaInit()
{
	//统一从flash中读取所有有关售货机的参数
	readSystemParaFromFlash();//changed by yoc 2014.2.19
	initDeviecByfile();

	
}




/*********************************************************************************************************
** Function name:       SystemDevInit
** Descriptions:        系统设备初始化
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void SystemDevInit()
{	
#if 0
	//硬币器初始化	
	if(SYSPara.CoinType == VMC_COIN_PARALLEL)
	{
		InitParallelPluseCoinAcceptor();
	}	
	else if(SYSPara.CoinType == VMC_COIN_SERIAL)
	{
		InitSerialPluseCoinAcdeptor();
	}	
	msleep(1000);
	
	//纸币器初始化
	if(SYSPara.BillType)
	{
		BillDevInit();
	}	
	
	//找零器初始化
	if(SYSPara.DispenceType)
	{
		hopperInit();
	}
	msleep(1000);
		

	//使能键盘
	EnableKeyBoard();

	//使能串行、并行硬币器
	if(SYSPara.CoinType == VMC_COIN_PARALLEL)
		DisableParallelPluseCoinAcceptor();
	else if(SYSPara.CoinType == VMC_COIN_SERIAL)
		DisableSerialPluseCoinAcceptor();
#endif	
}


/*********************************************************************************************************
** Function name:       TASK_Device
** Descriptions:        设备轮训任务
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/

void TASK_Device(void *pdata)
{
	uint8_t err;
	unsigned char subType = 0;
	MAIN_DEV_TASK_MSG *nMsgPack;

	SystemParaInit();//系统参数初始化
	SystemDevInit();	//系统设备初始化	
	OSSemPost(g_InitSem);
	Trace("device init over.....\r\n");
	SET_ULED_ON();
	OSTimeDly(100);
   	
	

	//设备任务流程
	while(1)
	{
		//1.轮训纸币器	
		billTaskPoll();	
		Trace("billStatus=%04x\r\n",stDev.bill.errNo);
				
		//2.轮训硬币器
		coinTaskPoll();	
		Trace("billStatus=%04x\r\n",stDev.coin.errNo);
		//接收设备邮件
		nMsgPack = OSMboxPend(g_msg_main_to_dev,10,&err);
		if(err == OS_NO_ERR)
		{	
			Trace("task_msg_dev_to_main PEND:%d\r\n",nMsgPack->type);
			if(nMsgPack->type == TASK_INIT)	
			{
				subType = nMsgPack->subType;
				
				if(subType & OBJ_BILL) //初始化纸币器
				{
					sysPara.billType = nMsgPack->bill.type;
					billInit();
					task_msg_dev_to_main.bill.type = sysPara.billType;
					task_msg_dev_to_main.bill.status = stDev.bill.state;
					task_msg_dev_to_main.bill.amount = 0;
					task_msg_dev_to_main.bill.change = 0;
					
				}
				if(subType & OBJ_COIN)//初始化硬币器
				{
					sysPara.coinType = nMsgPack->coin.type;
					coinInit();
					task_msg_dev_to_main.coin.type = sysPara.coinType;
					task_msg_dev_to_main.coin.status = stDev.coin.state;
					task_msg_dev_to_main.coin.amount = 0;
					task_msg_dev_to_main.coin.change = 0;
				}

			}
			else if(nMsgPack->type == TASK_CHANGER)//找零
			{
				subType = nMsgPack->subType;
				if(subType & OBJ_BILL)//
				{	
					subType = subType;	//预备
					task_msg_dev_to_main.bill.change = 0;
				}
				if(subType & OBJ_COIN)//硬币器找零
				{
					coinChange(nMsgPack->coin.change,&task_msg_dev_to_main.coin.change);
				}
				if(subType & OBJ_HOPPER)//hopper 找零
				{
					hopperChangerHandle(nMsgPack->hopper.value32,&task_msg_dev_to_main.hopper.value32);
				}
				
			}
			else if(nMsgPack->type == TASK_ENABLE)
			{
				subType = nMsgPack->subType;
				if(subType & OBJ_BILL) //使能纸币器
				{
					stDev.bill.pcDisabled = 0;
					billEnable();
				    task_msg_dev_to_main.bill.status = stDev.bill.state;
				}
				if(subType & OBJ_COIN)//使能硬币器
				{
					stDev.coin.pcDisabled = 0;
					coinEnable();
					task_msg_dev_to_main.coin.status = stDev.coin.state;
				}
				
			}
			else if(nMsgPack->type == TASK_DISABLE)
			{	
				subType = nMsgPack->subType;
				if(subType & OBJ_BILL) //禁能纸币器
				{
					stDev.bill.pcDisabled = 1;
					billDisable();
					task_msg_dev_to_main.bill.status = stDev.bill.state;
				}
				if(subType & OBJ_COIN)//禁能硬币器
				{
					stDev.coin.pcDisabled = 1;
					coinDisable();
					task_msg_dev_to_main.coin.status = stDev.coin.state;
				}
			}
			//将出币结果发送给交易任务
			task_msg_dev_to_main.subType = nMsgPack->subType;
			mbox_post_dev_to_main(nMsgPack->type);
			OSTimeDly(10);
			
		}
		else
		{
			OSTimeDly(10);
		}
			
	}	
}












