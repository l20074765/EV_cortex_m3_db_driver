/****************************************Copyright (c)*************************************************
**                      Fujian Junpeng Communicaiton Technology Co.,Ltd.
**                               http://www.easivend.com.cn
**--------------File Info------------------------------------------------------------------------------
** File name:           hopper.h
** Last modified Date:  2014-02-28
** Last Version:        
** Descriptions:        hopper找零算法+驱动                    
**------------------------------------------------------------------------------------------------------
** Created by:          yanbo
** Created date:        2014-02-28
** Version:             V1.0
** Descriptions:        The original version        
********************************************************************************************************/
#include "..\Config.h"

ST_HOPPER sysHopper[3];//初始化读取flash数据

ST_HOPPER stHopper[3];//定义三个hopper斗 结构体数组
static unsigned char  sn=0;


//定义hopper级别 结构体数组 按数组依次划分级别 1 > 2 > 3
//例如 hopper1 hopper2 通道面值 1元 hopper3 通道面值为 5毛 则 hopper1 hopper2
//划分在级别1中,hopper3 划分在级别2中  级别3为空  即
ST_HOPPER_LEVEL stHopperLevel[3];



/*********************************************************************************************************
** Function name:     	hopperInit
** Descriptions:	    hopper设备初始化函数
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void hopperInit(void)
{
	unsigned char i,j,levelNum = 0;
	unsigned int value;

	unsigned char ratio[HOPPER_NUMS] = HOPPER_LEVEL_RATIO;
	unsigned char address[HOPPER_NUMS] = HOPPER_ADDR;
	for(i = 0; i < HOPPER_NUMS;i++)
	{
		memset((void *)&stHopper[i],0,sizeof(ST_HOPPER));
		memset((void *)&stHopperLevel[i],0,sizeof(ST_HOPPER_LEVEL));
	}
	
	//从flash读取通道面值及初始化hopper斗地址
	for(i = 0;i < HOPPER_NUMS;i++)
	{
		stHopper[i].channelValue= stDev.HpValue[i];
		stHopper[i].addr = address[i];
	}

	//级别分配
	for(i = 0;i < HOPPER_NUMS;i++)
	{
		value = 0;
		//查询未分配级别中的最大斗
		for(j = 0;j < HOPPER_NUMS;j++)
		{
			if(!stHopper[j].levelFlag && value < stHopper[j].channelValue)
			{
				value = stHopper[j].channelValue;
			}
		}
		stHopperLevel[i].channelValue = value;
		stHopperLevel[i].ratio = ratio[i];
		if(value)
		{
			for(j = 0;j < HOPPER_NUMS;j++)//查询相同 面值的斗 进行分配
			{
				if(value == stHopper[j].channelValue)
				{
					levelNum = stHopperLevel[i].num ++;
					stHopperLevel[i].hopper[levelNum] = &stHopper[j];
					stHopper[j].levelFlag = 1;//级别分配标志
					
				}
			}
		}
		
		Trace("levelNo=%d num =%d value = %d\r\n",
			i,stHopperLevel[i].num,stHopperLevel[i].channelValue);
		
		
	}

	Trace("hopperInit over..\r\n");
}

/*********************************************************************************************************
** Function name:     	getHopperIndex
** Descriptions:	    根据hopper物理地址号获得 斗号
** input parameters:    
						addr:设备的硬件物理地址
						
** output parameters:   无
** Returned value:      0失败  非零 返回 索引号
*********************************************************************************************************/
unsigned char getHopperIndex(unsigned char addr)
{
	unsigned char i;
	for(i = 0;i < HOPPER_NUMS;i++)
	{
		if(stHopper[i].addr == addr)
			return (i + 1);
	}

	return 0;
}


unsigned char getMinHopperPriceState(void)
{
	unsigned char i,j;
	for(i = HOPPER_NUMS;i > 0;i++)
	{
		if(stHopperLevel[i - 1].channelValue)
		{
			break;
		}
			
	}
	if(i)
	{
		for(j = 0;j < stHopperLevel[i - 1].num;j++)
		{
			if(stHopperLevel[i - 1].hopper[j]->state == 0)
				return 1;
		}	
	}
	
	return 0;
}

/*********************************************************************************************************
** Function name:     	evb_hopper_handle
** Descriptions:	    EVBhopper设备操作控制函数
** input parameters:    cmdType:需要操作控制的设备类型
						address:设备的硬件物理地址
						cmdVlaue:操作内容，如Hopper出币操作的出币数量
** output parameters:   无
** Returned value:      1：收到应答；0：未收到应答即通讯失败
*********************************************************************************************************/
uint8_t evb_hopper_check(uint8_t address)
{
	uint8_t EvbAckMsg[36] = {0};
	uint8_t res,hopperNo=0;
	res = EvbConversation(0,0xED,sn,EVB_HANDLE_HP_QUERY,address,0,EvbAckMsg);
	if(res == 1)
	{
		sn++;
		if(EvbAckMsg[1] == 0x08)//查询返回包
		{
			hopperNo = getHopperIndex(EvbAckMsg[4]);	
	
			EvbAckMsg[3] &= ~(0x01 | (0x01 << 2) | (0x01 << 1) | (0x01 << 7) );//屏蔽非故障位
			if(EvbAckMsg[3]==0x00)//正常
				stHopper[hopperNo -1 ].state= 0x00;
			else if((EvbAckMsg[3] & (0x01 << 5)) || (EvbAckMsg[3] & (0x01 << 3)))//缺币
			{
				if((EvbAckMsg[3] & (0x01 << 4)) || (EvbAckMsg[3] & (0x01 << 6)))//有故障
					stHopper[hopperNo -1 ].state = 0x02;
				else
					stHopper[hopperNo -1 ].state = 0x01;
			}
				
			else
				stHopper[hopperNo -1 ].state = 0x02;	//故障
				
			stHopper[hopperNo -1 ].LastOutPutCount =  INTEG16(EvbAckMsg[6], EvbAckMsg[5]);

			
			return 1;
		}
					
	}	

	return 0;
		
}



/*********************************************************************************************************
** Function name:     	evb_hopper_handle
** Descriptions:	    EVBhopper设备操作控制函数
** input parameters:    cmdType:需要操作控制的设备类型
						address:设备的硬件物理地址
						cmdVlaue:操作内容，如Hopper出币操作的出币数量
** output parameters:   无
** Returned value:      1：收到应答；0：未收到应答即通讯失败
*********************************************************************************************************/
uint8_t evb_hopper_handle(uint8_t cmdType,uint8_t address,unsigned short cmdVlaue)
{
	uint8_t EvbAckMsg[36] = {0},j;
	uint8_t MsgHead,MsgType,MsgAdd;
	uint16_t MsgDate;
	uint8_t res,hopperNo=0;

	if(cmdType == EVB_HANDLE_HP_QUERY)//Hopper查询操作指令	
	{
		MsgHead = 0xED;
		MsgType = EVB_HANDLE_HP_QUERY;
		MsgAdd = address;
		MsgDate = 0;	
	}
	else if(cmdType == EVB_HANDLE_HP_OUTPUT)//Hopper出币操作指令
	{
		MsgHead = 0xED;
		MsgType = EVB_HANDLE_HP_OUTPUT;
		MsgAdd = address;
		MsgDate =  cmdVlaue;
	}
	else
	{
		Trace("EvbDevice_Handle the cmdType is err!\r\n");	
		return 0;
	}
	
	
	for(j=0;j<5;j++)
	{
		//发送操作包，并判断应答包
		//Trace("EvbConversation : %d %d %d \r\n",MsgType,MsgAdd,MsgDate);
		res = EvbConversation(0,MsgHead,sn,MsgType,MsgAdd,MsgDate,EvbAckMsg);
		
		//Trace("res..EvbConversation=%d\r\n",res);
		if(res == 1)
		{
			sn++;
			//出币回应包
			
			if(EvbAckMsg[1] == 0x06 && MsgType == EVB_HANDLE_HP_OUTPUT)
			{
				print_log("hopper%d output cmd...ok\r\n",address + 1);
				return 1;
			}
			else if(EvbAckMsg[1] == 0x08 && MsgType == EVB_HANDLE_HP_QUERY)//查询返回包
			{
				
				hopperNo = getHopperIndex(EvbAckMsg[4]);
				print_log("hopper%d checked state = %x lastCOunt= %d\r\n",
					hopperNo,EvbAckMsg[3],stHopper[hopperNo -1 ].LastOutPutCount);
				EvbAckMsg[3] &= ~((0x01 << 2) | (0x01 << 1) | (0x01 << 7) );//屏蔽非故障位
				if(EvbAckMsg[3]==0x00)//正常
					stHopper[hopperNo -1 ].state= 0x00;
				else if(EvbAckMsg[3] & 0x01)//找零失败
				{
					stHopper[hopperNo -1 ].state = 0x02;
				}
				else if((EvbAckMsg[3] & (0x01 << 5)) || (EvbAckMsg[3] & (0x01 << 3)))//缺币
				{
					if((EvbAckMsg[3] & (0x01 << 4)) || (EvbAckMsg[3] & (0x01 << 6)))//有故障
						stHopper[hopperNo -1 ].state = 0x02;
					else
						stHopper[hopperNo -1 ].state = 0x01;
				}
				else
					stHopper[hopperNo -1 ].state = 0x02;	//故障

					
				stHopper[hopperNo -1 ].LastOutPutCount =  INTEG16(EvbAckMsg[6], EvbAckMsg[5]);

				
				return 1;
			}
			
			
			
		}	
		
	}
	return 0;
}

/*********************************************************************************************************
** Function name:     	getHopperHandleTime
** Descriptions:	    根据找零金额计算 超时等待时间
** input parameters:    
						
** output parameters:   无
** Returned value:      
*********************************************************************************************************/
unsigned short getHopperHandleTime(unsigned int value)
{
	unsigned char i;
	for(i = HOPPER_NUMS;i > 0;i--)
	{
		if(stHopperLevel[i - 1].channelValue)
		{
			return (((value/stHopperLevel[i - 1].channelValue) * 20) + 12000);
		}
	}
	return 12000;
}



/*********************************************************************************************************
** Function name:     	hopperTestOutput
** Descriptions:	    hopper出币测试操作函数
** input parameters:    Count：出币数量
						addr:地址编号
** output parameters:   无
** Returned value:      1：成功出币；0：出币失败
*********************************************************************************************************/
uint8_t hopperTestOutput(unsigned short  count,unsigned char addr)
{
	uint8_t res;
	unsigned char hpIndex;
	if(count<=0)
	{
		Trace("count is 0 ......\r\n");
		return 0;	
	}
	//发送出币指令
	res = evb_hopper_handle(EVB_HANDLE_HP_OUTPUT,addr,count);
	if(res==1)
	{
		OSTimeDly(100);
		//等待出币完成返回应答包
		Timer.HpHandleTimer = 5000 + count * 250;
		while(Timer.HpHandleTimer)
		{
			Trace("HpHandleTimer=%d\r\n",Timer.HpHandleTimer);
			//查询是否出币完成
			res = evb_hopper_handle(EVB_HANDLE_HP_QUERY,addr,0);
			//Trace("evb_hopper_handle res = %d addr = %d lastOutput = %d\r\n",res,addr,stHopper[getHopperIndex(addr) - 1].LastOutPutCount);
			if(res==1)
			{
				hpIndex = getHopperIndex(addr);
				if(hpIndex)
				{	
					Trace("LastOutPutCount = %d  count = %d\r\n",stHopper[hpIndex - 1].LastOutPutCount,count);
					if(stHopper[hpIndex - 1].LastOutPutCount != count)//找零失败
					{
						
						return 0;
					}	
					else
						return 1;			
				}					
			}
			OSTimeDly(2);
		}
		return 0;	
	}
	return 0;
}





/*********************************************************************************************************
** Function name:     	hopperOutput
** Descriptions:	    hopper出币操作函数
** input parameters:    Count：出币数量
						addr:地址编号
** output parameters:   无
** Returned value:      1：成功出币；0：出币失败
*********************************************************************************************************/
uint8_t hopperOutput(unsigned short  count,unsigned char addr)
{
	uint8_t res,i;
	unsigned char hpIndex;
	if(count<=0)
	{
		Trace("count is 0 ......\r\n");
		return 0;	
	}
	//发送出币指令
	for(i = 0;i < 3;i++)
	{
		res = evb_hopper_handle(EVB_HANDLE_HP_OUTPUT,addr,count);
		if(res == 1)
			break;
		print_log("hopper output cmd fail num = %d\r\n",i);
		OSTimeDly(100/5);
	}
	if(res==1)
	{
		OSTimeDly(100);
		//等待出币完成返回应答包
		Timer.HpHandleTimer = 5000 + count * 250;
		while(Timer.HpHandleTimer)
		{
			Trace("HpHandleTimer=%d\r\n",Timer.HpHandleTimer);
			//查询是否出币完成
			res = evb_hopper_handle(EVB_HANDLE_HP_QUERY,addr,0);
			//Trace("evb_hopper_handle res = %d addr = %d lastOutput = %d\r\n",res,addr,stHopper[getHopperIndex(addr) - 1].LastOutPutCount);
			if(res==1)
			{
				hpIndex = getHopperIndex(addr);
				Trace("LastOutPutCount = %d  count = %d\r\n",stHopper[hpIndex - 1].LastOutPutCount,count);
				if(hpIndex)
				{	
					if(hpIndex == 1)
						stTotalTrade.Hopper1DispSum += stHopper[hpIndex - 1].LastOutPutCount;
					else if(hpIndex == 2)
						stTotalTrade.Hopper2DispSum += stHopper[hpIndex - 1].LastOutPutCount;
					else if(hpIndex == 3)
						stTotalTrade.Hopper3DispSum += stHopper[hpIndex - 1].LastOutPutCount;

					print_log("output:hopper1 =%d hopper2 = %d hopper3 = %d\r\n",
						stTotalTrade.Hopper1DispSum,stTotalTrade.Hopper2DispSum,stTotalTrade.Hopper3DispSum);
					if(stHopper[hpIndex - 1].LastOutPutCount != count)//找零失败
					{
						//stHopper[hpIndex - 1].lastPayoutFail = 1;
						return 0;
					}	
					else
					{
						//stHopper[hpIndex - 1].lastPayoutFail = 0;
						return 1;	
					}			
				}					
			}
			OSTimeDly(2);
		}
		return 0;	
	}
	return 0;
}



/*********************************************************************************************************
** Function name:     	checkHopperAndUpdate
** Descriptions:	    检查hopper并更新状态和 结构体
** input parameters:    
						
** output parameters:   
** Returned value:      
*********************************************************************************************************/

void checkHopperAndUpdate(void)
{
	unsigned char i;
	for(i = 0;i < HOPPER_NUMS;i++)
	{
		if(stHopper[i].channelValue)
		{
			evb_hopper_check(stHopper[i].addr);	
		}
		else
			 stHopper[i].state = 3;

	} 
	
}



/*********************************************************************************************************
** Function name:     	hopperOutputByLevel
** Descriptions:	    hopper设备兑零操作函数
** input parameters:    changeCount——需要兑零的数量
** output parameters:   remainCount——兑零失败后剩余的数量
						levelNo :选择哪个级别斗 找零
** Returned value:      1：兑币成功；0：失败
*********************************************************************************************************/
unsigned char  hopperOutputByLevel(unsigned int changeCount,unsigned int *remainCount,
				unsigned char levelNo)
{
	unsigned char i,addr,cycleFlag = 0,res;
	unsigned int changedCount = 0;//已找零金额
	unsigned char rcx = 2;
	if(changeCount == 0)
	{
		*remainCount = 0;
		return 1;
	}
	//断言 hopper斗号是否符合
	if(levelNo > HOPPER_NUMS)
	{
		*remainCount = changeCount;
		Trace("the levelNo = %d > the best no %d \r\n",levelNo,HOPPER_NUMS);
		return 0;
	}
	if(!stHopperLevel[levelNo - 1].num )//没有可用的斗
	{
		Trace("the levelNo = %d useAbleNum = is 0 \r\n",levelNo);
		*remainCount = changeCount;
		return 0;
	}
	//查询是否有循环斗
	for(i = 0; i < stHopperLevel[levelNo - 1].num;i++)
	{
		if(stHopperLevel[levelNo - 1].hopper[i]->isCycle && stHopperLevel[levelNo - 1].hopper[i]->state == 0)
		{
			cycleFlag = 1;
			break;
		}		
	}
	if(cycleFlag)//有循环斗 优先让循环斗出币
	{
		addr = stHopperLevel[levelNo - 1].hopper[i]->addr;
		res = hopperOutput(changeCount,addr);
		if(res)
		{
			*remainCount  = 0;
			return 1;
		}
		else
		{
			changedCount += stHopperLevel[levelNo - 1].hopper[i]->LastOutPutCount;//循环斗找零失败 再找其他斗	
		}
	}
	//没有循环斗 则 随便选择一个可用的斗 出币
	for(i = 0; i < stHopperLevel[levelNo - 1].num;i++)
	{	
		if(stHopperLevel[levelNo - 1].hopper[i]->specialFault)//检查设备不可用跳出
			continue;
		addr = stHopperLevel[levelNo - 1].hopper[i]->addr;	
		for(rcx = 0;rcx < 2;rcx++)
		{
			res = hopperOutput(changeCount - changedCount,addr);
			Trace("selcet-- levelNo = %d addr= %d res =%d\r\n",levelNo,addr,res);
			if(res)
			{	
				*remainCount  = 0;
				return 1;
			}
			else //失败了 继续找下一个 同级别的斗 出币
			{
				changedCount += stHopperLevel[levelNo - 1].hopper[i]->LastOutPutCount;
				Trace("changeCount = %d,hopper[%d]->LastOutPutCount =%d,changedCount =%d\r\n",
					changeCount,i,stHopperLevel[levelNo - 1].hopper[i]->LastOutPutCount,changedCount);				
			}
		}
		//连续循环三次找币，如果还是找不出来的话
		//则软件认为 hopper 特殊故障并标记			
		stHopperLevel[levelNo - 1].hopper[i]->specialFault = 1;//故障
						
	}
	//查询本级别所有可用的斗 出币后的最终的失败情况
	*remainCount = changeCount - changedCount;
	return 0;
		
	
	
}


/*********************************************************************************************************
** Function name:     	hopperChangerHandle
** Descriptions:	    hopper设备兑零操作函数
** input parameters:    changeCount——需要兑零的金额
** output parameters:   remainCount——兑零失败后剩余的金额
						
** Returned value:      1：兑币成功；0：失败
*********************************************************************************************************/
unsigned char  hopperChangerHandle(unsigned int changeMoney,unsigned int *remainMoney)
{
	unsigned int changedCount,remainCount;
	unsigned char res,i;
	//Trace("hopperChangerHandle:changeMoney = %d  \r\n",changeMoney);
	//按级别 从大到小 开始找零
	for(i = 0; i < HOPPER_NUMS;i++)
	{
		//Trace("stHopperLevel[%d].channelValue = %d\r\n",i,stHopperLevel[i].channelValue);
		if(!stHopperLevel[i].channelValue)//此级别不存在
			continue;
		changedCount = changeMoney / stHopperLevel[i].channelValue; //本级别斗 需要找零数
		changeMoney  = changeMoney - (changedCount * stHopperLevel[i].channelValue);
		res = hopperOutputByLevel(changedCount,&remainCount,(i + 1));
		if(res)//找零成功
		{
			//Trace("changeMoney .... %d %d %d\r\n",changeMoney,changedCount,stHopperLevel[i].channelValue);
			if(changeMoney == 0)//总找零成功
			{
				*remainMoney = 0;
				return 1;
			}
		}
		else
		{
			changeMoney  = changeMoney + remainCount * stHopperLevel[i].channelValue;	
			//Trace("changeMoney remain .... %d  %d %d\r\n",changeMoney,(changedCount - remainCount),stHopperLevel[i].channelValue);
		}
		
		
	}
	//所有级别斗都查找了 直接返回剩余金额
	*remainMoney = changeMoney;
	Trace("all level is checked....\r\n");
	return 0;
		
		
	
	
}

/*********************************************************************************************************
** Function name:     	evb_device_hp_dispence
** Descriptions:	    EVB_Hp设备兑零操作函数
** input parameters:    ChangeMoney——需要兑零的金额
** output parameters:   SurplusMoney——兑零失败后剩余的金额
						Hp1OutNum——00地址Hopper的出币个数
						Hp2OutNum——01地址Hopper的出币个数
						Hp3OutNum——10地址Hopper的出币个数
** Returned value:      1：兑币成功；0：失败
*********************************************************************************************************/
uint32_t evb_device_hp_dispence(uint32_t ChangeMoney,uint32_t *SurplusMoney,uint8_t *Hp1OutNum,uint8_t *Hp2OutNum,uint8_t *Hp3OutNum)
{
	unsigned char mErrFlag = 3,err;
	MAIN_DEV_TASK_MSG *HpBackMsg;
	unsigned int remainVlaue;
	unsigned short timeOut;
	while(mErrFlag)
	{
		
		task_msg_main_to_dev.hopper.value32 = ChangeMoney;
		//发送邮箱给DevieTask让其操作Hopper做出币操作
		task_msg_main_to_dev.subType = OBJ_HOPPER;
		mbox_post_main_to_dev(TASK_CHANGER);
		//等待DeviceTask任务返回应答邮箱，得到指定Hopper的出币情况
		
		timeOut = getHopperHandleTime(ChangeMoney);//按照最小级别斗找零计算超时时间

		HpBackMsg = OSMboxPend(g_msg_dev_to_main,timeOut,&err);
		if(err == OS_NO_ERR)
		{
			remainVlaue = HpBackMsg->hopper.value32;
			//Trace("hopper mboxBack remainVlaue=%d\r\n",remainVlaue);
			*SurplusMoney = remainVlaue;
			*Hp1OutNum = stHopper[0].LastOutPutCount;
			*Hp2OutNum = stHopper[1].LastOutPutCount;
			*Hp3OutNum = stHopper[2].LastOutPutCount;
			
			return (remainVlaue) ? 0 : 1;
			
		}
		else
		{
			mErrFlag--;
			continue;
		}
	}

	return 0;
}




/*********************************************************************************************************
//算法预处理hopper 有两个斗 面值 2元 和面值 1元
1.纸币兑零按照 5元(2*2 + 1*1)的方式进行兑零
纸币投一张立马兑零

硬币投入的一枚面值大于等于  hopper最大面值认为是兑小
此时投一枚立即找零

硬币投入的一枚面值小于hopper最大面值任务是兑大 默认按高一级的面值找零

2.硬币兑零大兑小 2元 硬币 兑换 2*1元
3.硬币兑零 小兑大 一旦满足兑零要求 立即兑换硬币
	例如 投 50分 20分20分 10分后 立马兑零1元*1
	例如投 1元1元 立马找零 1*2元  

*********************************************************************************************************/


/*********************************************************************************************************
** Function name:     	vmcBillChanger
** Descriptions:	     纸币兑零算法
** input parameters:   
** output parameters:   
** Returned value:      1：兑币成功；0：失败
*********************************************************************************************************/
unsigned char  vmcBillChanger(unsigned int changeAmount,unsigned int *remainAmount)
{
	char i,j,res;
	unsigned int remainCount = 0,changeCount = 0;	
	//收纸币找零算法
	if(!stDev.hpBaseChange)
	{
		print_log("stDev.hpBaseChange is 0 can't change.....\r\n");
		return 0;
	}
	for(i = 0;i < HOPPER_NUMS;i++)
	{		
		if(!stHopperLevel[i].channelValue ||  !stHopperLevel[i].ratio)
		{
			break;
		}
	}	
	
	*remainAmount = changeAmount % HOPPER_BASE_CHANGED;

	if(changeAmount / HOPPER_BASE_CHANGED)
	{
		//按比例先进行一次一轮循环找零
		for(j = 0;j < i;j++)
		{
			changeCount = (changeAmount / HOPPER_BASE_CHANGED) * stHopperLevel[j].ratio;
			if(!hopperOutputByLevel(changeCount, &remainCount, j+1))
			{
				*remainAmount += remainCount * stHopperLevel[j].channelValue;//不成功累计剩余金额
				print_log("vbillChanged[%d] *remainAmount = %d\r\n",j,*remainAmount);
			}

		}
	}
	
	//还有未找零的金额
	if(*remainAmount)
	{
		changeAmount = *remainAmount;
		//可以按照收货机找零方式进行找零
		print_log("remain.....changeAmount = %d\r\n",changeAmount);
		res = hopperChangerHandle(changeAmount,remainAmount);
		
		if(res)
		{
			return 1;
		}
		else
			return 0;
	}
	
	return 1;
}



/*********************************************************************************************************
** Function name:     	vmcCheckBillChanger
** Descriptions:	     检查纸币是否可以兑零
** input parameters:   
** output parameters:   
** Returned value:      1：兑币成功；0：失败
*********************************************************************************************************/

unsigned char vmcCheckBillChanger(unsigned short changeAmount)
{
	unsigned char i,j;
	for(i= 0;i < HOPPER_NUMS;i++)
	{
		if(changeAmount >= stHopperLevel[i].channelValue)
			break;
	}
	if((i < HOPPER_NUMS) && stHopperLevel[i].channelValue)
	{
		for(;i < HOPPER_NUMS;i++)
		{
			if(!stHopperLevel[i].channelValue)
				break;
			for(j =0;j < stHopperLevel[i].num;j++)
			{
				if(!stHopperLevel[i].hopper[j]->state)
					return 1;
			}
		}
	}
	return 0;
	
}


/*********************************************************************************************************
** Function name:     	vmcCheckCoinChanger
** Descriptions:	     检查硬币是否可以兑零
** input parameters:   
** output parameters:   
** Returned value:      1：兑币成功；0：失败
*********************************************************************************************************/

unsigned char vmcCheckCoinChanger(unsigned short changeAmount)
{
	unsigned char i,j;
	for(i= 0;i < HOPPER_NUMS;i++)
	{
		if(stHopperLevel[i].channelValue)
		{
			if(changeAmount >= stHopperLevel[i].channelValue)
			{	
				break;
			}
		}
		else
			break;
	}

	for(;i < HOPPER_NUMS;i++)
	{
		if(!stHopperLevel[i].channelValue)
			break;
		for(j =0;j < stHopperLevel[i].num;j++)
		{
			if(!stHopperLevel[i].hopper[j]->state)
				return 1;
		}
	}
	return 0;

	
}



/*********************************************************************************************************
** Function name:     	vmcCoinChanger
** Descriptions:	    硬币兑零算法
** input parameters:   
** output parameters:   
** Returned value:      1：兑币成功；0：失败
*********************************************************************************************************/

unsigned char vmcCoinChanger(unsigned short changeAmount,unsigned int *remainAmount)
{
	unsigned char i;
	unsigned int remainCount = 0,changeCount = 0;

	*remainAmount = changeAmount;
	//硬币器找零算法
	for(i = HOPPER_NUMS;i > 0;i--)
	{
		if(stHopperLevel[i - 1].channelValue)
		{
			*remainAmount  = changeAmount % stHopperLevel[i - 1].channelValue;
			 changeCount = changeAmount / stHopperLevel[i - 1].channelValue;
			 print_log("coin level[%d].changeCount = %d \r\n",i-1,changeCount);
			
			 if(hopperOutputByLevel(changeCount, &remainCount,i))
			 {
				if(*remainAmount == 0)
					return 1;		
			 }
			 changeAmount = *remainAmount  + remainCount*stHopperLevel[i - 1].channelValue;
			 *remainAmount = changeAmount;
		}
		
	}
	return (*remainAmount == 0) ? 1 : 0;
	
 
	
}


/*********************************************************************************************************
** Function name:     	hopperChanger
** Descriptions:	    兑零邮箱
** input parameters:   
** output parameters:   
** Returned value:      1：兑币成功；0：失败
*********************************************************************************************************/

uint32_t hopperChanger(uint32_t changedAmount,uint32_t *remainAmount,unsigned char type)
{
	unsigned char mErrFlag = 3,err;
	MAIN_DEV_TASK_MSG *HpBackMsg;
	unsigned int remainVlaue;
	unsigned short timeOut;
	while(mErrFlag)
	{
		task_msg_main_to_dev.hopper.value32 = changedAmount;
		task_msg_main_to_dev.subType = OBJ_HOPPER;
		//发送邮箱给DevieTask让其操作Hopper做出币操作
		mbox_post_main_to_dev(TASK_CHANGER);
		//等待DeviceTask任务返回应答邮箱，得到指定Hopper的出币情况
		
		timeOut = getHopperHandleTime(changedAmount);//按照最小级别斗找零计算超时时间
		HpBackMsg = OSMboxPend(g_msg_dev_to_main,timeOut,&err);
		if(err == OS_NO_ERR)
		{
			
			remainVlaue = HpBackMsg->hopper.value32;
			*remainAmount = remainVlaue;
			return (remainVlaue) ? 0 : 1;
			
		}
		else
		{
			mErrFlag--;
			continue;
		}
	}

	return 0;
}




/*********************************************************************************************************
** Function name:     	hopperChangerFailCheck
** Descriptions:	    查询hopper找零失败原因
** input parameters:   
** output parameters:   
** Returned value:      1：兑币成功；0：失败
*********************************************************************************************************/
unsigned char hopperChangerFailCheck(const unsigned int remainAmount)
{
	unsigned char i;

	for(i = HOPPER_NUMS;i > 0;i--)
	{
		if(stHopperLevel[i - 1].channelValue)
			break;
	}

	print_log("check the mix hopper calue is %d remainAmount is %d\r\n",
		stHopperLevel[i - 1].channelValue,remainAmount);

	if(i && (remainAmount < stHopperLevel[i - 1].channelValue))//找零金额小于最小面值斗
		return 1;
	else
		return 0;
}

/*********************************************************************************************************
** Function name:       hopperIsEmpty
** Descriptions:        hopper缺币 
** input parameters:    
** output parameters:   无
** Returned value:      1 缺币  0 正常
*********************************************************************************************************/
uint8_t hopperIsEmpty(void)
{
	return ((hopperErrNo & HP_ERR_NO_HOPPER1_EMPTY) && (hopperErrNo & HP_ERR_NO_HOPPER2_EMPTY));

}


unsigned char HopperDispence(unsigned int DispenceMoney,unsigned int *iouMoney)
{
	uint8_t res = 0;
	uint32_t SurplusMoney=0;
	uint8_t CHp1Out=0,CHp2Out=0,CHp3Out=0;
	*iouMoney = DispenceMoney;

	res = evb_device_hp_dispence(DispenceMoney,&SurplusMoney,&CHp1Out,&CHp2Out,&CHp3Out);
	Trace("Dispence=%d Sur=%d cHp1=%d cHp2=%d cHp3=%d\r\n",
		DispenceMoney,SurplusMoney,CHp1Out,CHp2Out,CHp3Out);
	if(res==1) //兑零成功
	{	
		stTotalTrade.DispenceSum += DispenceMoney;	
	}
	else//兑零失败
	{
		stTotalTrade.DispenceSum += (DispenceMoney-SurplusMoney);
		*iouMoney = SurplusMoney;
	}
	
	if(CHp1Out)
	{
		stTotalTrade.Hopper1DispSum += CHp1Out;
	}
	if(CHp2Out)
	{	
		stTotalTrade.Hopper2DispSum += CHp2Out;
	}
	if(CHp3Out)
	{
		stTotalTrade.Hopper3DispSum += CHp3Out;	
	}


	
	return res;
}


 
