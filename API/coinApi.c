#include "..\config.h"

static unsigned char coinIndex = 0;

void coinStatusUpdate(void);


/*********************************************************************************************************
** Function name:       coin_send
** Descriptions:         硬币器命令发送 
** input parameters:    Dev:设备号(高5位地址及低三位命令)；
**					    *sd:要发送的数据slen：要发送数据的长度
** output parameters:   *rd:接收到的数据rlen:接收到数据的长度
** Returned value:      0:失败 1通讯成功2通讯超时
*********************************************************************************************************/
static unsigned char coin_send(uint8_t dev,uint8_t *sd,uint8_t slen,uint8_t *rd,uint8_t *rlen)
{
	unsigned char res,i;
	for(i = 0;i < 8;i++)
	{
		res = MdbConversation(dev,sd,slen,rd,rlen);
		if(res == 1)
			break;
		msleep(500);
	}
	stDev.coin.comerr = (res == 1) ? 0 : 1;
	return res;
}


/*********************************************************************************************************
** Function name:       coinDisable
** Descriptions:        禁能硬币器
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void coinDisable(void)
{
	uint8_t CoinValue[4]={0x00,0x00,0xFF,0xFF},rbuf[36],rlen,res;
   	res = coin_send(0x0C,CoinValue,4,&rbuf[0],&rlen);
	if(res == 1)
		stDev.coin.state = STATUS_COIN_DISABLE;
}


/*********************************************************************************************************
** Function name:       coinEnable
** Descriptions:        使能硬币器
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void coinEnable(void)
{
	unsigned char wbuf[4]={0x00,0x00,0xFF,0xFF},rbuf[36],rlen,i,res;	
	for(i = 0;i < 8;i++)
	{
		if(stDev.coin.channel[i] != 0)
			wbuf[1] |= (0x01 << i);			
	}	
	//Enable
	res = coin_send(0x0C,wbuf,4,rbuf,&rlen);
	if(res == 1)
		stDev.coin.state = STATUS_COIN_ENABLE;	
}



/*********************************************************************************************************
** Function name:       coinInit
** Descriptions:        硬币器设备初始化
** input parameters:    无
** output parameters:   无
** Returned value:      0失败,1成功
*********************************************************************************************************/
unsigned char coinInit(void)
{
	unsigned char res,rbuf[36],rlen,i;
	memset((void *)&stDev.coin,0,sizeof(stDev.coin));
	res = coin_send(0x08,NULL,0x00,&rbuf[0],&rlen); //Reset
	if(res != 1)
		return 0;

	msleep(100);
	res = coin_send(0x09,NULL,0x00,&rbuf[0],&rlen); //Setup
	if(res != 1)
		return 0;		
	stDev.coin.level = rbuf[0];
	stDev.coin.code = INTEG16(rbuf[1],rbuf[2]);
	stDev.coin.decimal = 100;
	for(i = 0; i < rbuf[4]; i++) 
	{
	   stDev.coin.decimal /= 10;
	}
	stDev.coin.scale = rbuf[3] * stDev.coin.decimal;
	stDev.coin.routing	= INTEG16(rbuf[5],rbuf[6]);
	Trace("level:%d,code=%d,decimal:%d,scale:%d,routing:%d\r\n",
		stDev.coin.level,stDev.coin.code,stDev.coin.decimal,stDev.coin.scale,
		stDev.coin.routing);

	for(i=0;i<8;i++)
	{
		if(rbuf[7+i] == 0xFF)
		     break;
		stDev.coin.channel[i] = (uint32_t)rbuf[7+i] * stDev.coin.scale;
		Trace("coinChannel[%d]=%d\r\n",i,stDev.coin.channel[i]);
	}

	msleep(100);
	res = coin_send(0x0B,NULL,0x00,&rbuf[0],&rlen); //Poll
	if(res != 1)
		return 0;

	msleep(100);		
	res = coin_send(0x0F,0x00,1,&rbuf[0],&rlen); //Identification
	if(res != 1)
		return 0;	
	for(i = 0; i < rlen; i++)
		stDev.coin.identity[i] = rbuf[i];
	
	msleep(100);
	res = coin_send(0x0A,NULL,0,&rbuf[0],&rlen); 
	if(res != 1)
		return 0;

	msleep(100);
	coinEnable();

	coinStatusUpdate();
	return 1;
}


/*********************************************************************************************************
** Function name:       coinEscrow
** Descriptions:        硬币器退币按钮触发
** input parameters:    无
** output parameters:   
** Returned value:      有硬币收入返回1，无返回0
*********************************************************************************************************/
void coinEscrow(void)
{
	Buzzer();
	pcdCoinEscrow();
} 

/*********************************************************************************************************
** Function name:       coin_0x0B_response
** Descriptions:        硬币器0x0B指令应答解析
** input parameters:    无
** output parameters:   
** Returned value:      有硬币收入返回1，无返回0
*********************************************************************************************************/
void  coin_0x0B_response(unsigned char *buf,unsigned char len)
{
	unsigned char i,temp,temp1,temp2;
	if(len == 0)
	{
		CLR_COIN_ERR(stDev.coin.curErrNo);
		return;
	}
	for(i = 0;i < len;)
	{
		temp1 = buf[i++];
		if(temp1 & (0x01 << 7))//coin despensed manually
		{
			temp2 = buf[i++];
			temp2 = temp2;
			Trace("Dispensed Manually \r\n");
		}
		else if(temp1 & (0x01 << 6)) //Coins Deposited	  有收币
		{
			temp2 = buf[i++];
			//the rbuf[1]	is The number of coins in the tube for the coin type accepted
			temp = (temp1 >> 4) & 0x03;
			Trace("Coin_recv:temp:%x coin:%d index:%d\r\n",temp,temp1 & 0x0F,coinIndex);
			if(temp == 0x00 || temp == 0x01)
			{
				CoinIn[coinIndex] =  (temp1 & 0x0F) + 1;
				OSQPost(g_CoinIn,(void *)&CoinIn[coinIndex]);
				coinIndex = ++coinIndex % G_COIN_IN_SIZE;
			}
		}
		else if(temp1 & (0x01 << 5))	 //Slug 
		{
			Trace("Slug");
		}
		else  //status 
		{
			temp = temp1 & 0x0F;	
			switch(temp)
			{
				case 0x01:
					coinEscrow(); 	
					break;							 //escrow request		
				case 0x02:	break;			                 //changer pay out busy					
			    case 0x03:	break;			                 //no credit //不能进入对应通道 			   
			    case 0x04:			                 //defective tube sensor//传感器故障
					SET_COIN_ERR(stDev.coin.curErrNo,STATUS_COIN_ERR_SENSOR);					   	   	
					break;
			    case 0x05:	break;		                 //double arriver	   	   
			    case 0x06:	break;		                 //acceptor unplugged						   				   
			    case 0x07:			                 //tube jam	//出币卡币
					SET_COIN_ERR(stDev.coin.curErrNo,STATUS_COIN_ERR_TUBEJAM);				   
				    break;
			    case 0x08:			                 //rom chksum err  //ROM出错				   
				  	SET_COIN_ERR(stDev.coin.curErrNo,STATUS_COIN_ERR_ROM);
				    break;
			    case 0x09:			                 //coin routing err	//进币通道出错				   
				   	SET_COIN_ERR(stDev.coin.curErrNo,STATUS_COIN_ERR_ROUTING);
				   	break;
			    case 0x0A: break;			                 //changer busy   
			    case 0x0B: break;			                 //changer was reset//刚复位				   		   
			   	case 0x0C:			                 //coin jam	//投币卡币
			   		SET_COIN_ERR(stDev.coin.curErrNo,STATUS_COIN_ERR_JAM);				   
				   	break;
			   	case 0x0D:		              	     //possible credited coin removal  //试图移除硬币		   
				   	SET_COIN_ERR(stDev.coin.curErrNo,STATUS_COIN_ERR_REMOVETUBE);
				   	break;
			   	default:	
			   	   	CLR_COIN_ERR(stDev.coin.curErrNo);
				   	break;
			}
			
		}
	}
}

/*********************************************************************************************************
** Function name:       coinStatusUpdate
** Descriptions:        硬币器状态更新
** input parameters:    无
** output parameters: 
** Returned value:      无
*********************************************************************************************************/
void coinStatusUpdate(void)
{
	if(stDev.coin.comerr == 1)
	{
		 stDev.coin.state = STATUS_COIN_COM;
		 stDev.coin.errNo = 0;
		 return;
	}
	  
	  	 
	if(stDev.coin.curErrNo == 0)//么有故障 
	{
		if(stDev.coin.pcDisabled == 1)
			stDev.coin.state = STATUS_COIN_DISABLE;
		else
		{
			if(stDev.coin.quebi == 1)//有缺币
				stDev.coin.state = STATUS_COIN_QURBI;
			else
				stDev.coin.state = STATUS_COIN_ENABLE;
			stDev.coin.errNo = 0;	
		}
	}
	else 
	{
		if(stDev.coin.curErrNo & (~STATUS_COIN_ERR_DISABLE)) //除了禁能还有其他故障
		{
			stDev.coin.state = STATUS_COIN_FAULT;
			stDev.coin.errNo = stDev.coin.curErrNo;
		}
						
	}



}

/*********************************************************************************************************
** Function name:       coinPoll
** Descriptions:        硬币器轮训
** input parameters:    无
** output parameters:   
** Returned value:      有硬币收入返回1，无返回0
*********************************************************************************************************/
unsigned char coinPoll(void)
{
	unsigned char res,rbuf[36],rlen;
	if(stDev.coin.state == STATUS_COIN_COM || stDev.coin.state == STATUS_COIN_N_A)//通信故障 禁止轮训
		return 0;
	if(sysPara.coinType == VMC_COIN_PARALLEL) //并行脉冲
	{
		CoinIn[coinIndex] = ReadParallelCoinAcceptor();
		if(CoinIn[coinIndex])
		{
			OSQPost(g_CoinIn,(void *)&CoinIn[coinIndex]);	
			coinIndex = ++coinIndex % G_COIN_IN_SIZE;
			return 1;
		}

	}
	else if(sysPara.coinType == VMC_COIN_SERIAL)//串行脉冲
	{	
		CoinIn[coinIndex] = ReadSerialPluseCoinAcceptor();
		if(CoinIn[coinIndex])
		{
			OSQPost(g_CoinIn,(void *)&CoinIn[coinIndex]);
			coinIndex = ++coinIndex % G_COIN_IN_SIZE;
			return 1;
		}	
	}
	else if(sysPara.coinType == VMC_COIN_MDB)
	{
		res = coin_send(0x0B,NULL,0x00,&rbuf[0],&rlen);
		if(res != 1) return 0;
		
		coin_0x0B_response(rbuf,rlen);
			
		if(stDev.coin.errNo != 0)
		{
			if(stDev.coin.state != STATUS_COIN_DISABLE)
				stDev.coin.state = STATUS_COIN_FAULT;	
		}
		else
		{
			if(stDev.coin.state != STATUS_COIN_DISABLE && stDev.coin.state != STATUS_COIN_QURBI)
			{
				stDev.coin.state = STATUS_COIN_ENABLE;			
			}		
		}
		
		return 1;
			
	}
	return 0;
}

/*********************************************************************************************************
** Function name:       coinRecv
** Descriptions:        硬币器收币
** input parameters:    无
** output parameters:   
** Returned value:      有硬币收入，无返回0
*********************************************************************************************************/

unsigned int coinRecv(void)
{
	unsigned int money = 0;
	unsigned char count = 0,*pcount,err;
	if(sysPara.coinType == VMC_COIN_PARALLEL)
	{
		count = ReadParallelCoinAcceptor();
		if(count)
		{
			money += stDev.coin.channel[count-1];
		}
	}
	else if(sysPara.coinType == VMC_COIN_SERIAL)
	{		
		count = ReadSerialPluseCoinAcceptor();
		if(count)
			money += stDev.coin.channel[count-1];
	
		
	}
	else if(sysPara.coinType == VMC_COIN_MDB)
	{
		pcount = (unsigned char *)OSQPend(g_CoinIn,2,&err);
		if(err == OS_NO_ERR && *pcount)
		{
			money += stDev.coin.channel[*pcount-1];
			Trace("coin_recv = %d\r\n",*pcount);
		}	
	}
	return money;
}


/*********************************************************************************************************
** Function name:       coinGetNums
** Descriptions:        MDB硬币器余币量
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void coinGetNums(void)
{
	uint8_t rbuf[36],rlen,res,j;
	unsigned int temp = 0;		
   	res = coin_send(0x0A,NULL,0x00,&rbuf[0],&rlen);
	if(res != 1) return;

	for(j = 0; j < 8; j++) 
	{
		stDev.coin.chNum[j] = rbuf[2+j];
		temp += rbuf[2+j];				 
		Trace("chNum[%d]=%d\r\n", j,stDev.coin.chNum[j]);
	}
	stDev.coin.quebi = (temp < 5) ? 1 : 0;	
}


/*********************************************************************************************************
** Function name:       ChangePayoutProcessLevel3
** Descriptions:        level3找零操作
** input parameters:    PayMoney——需要找出的硬币金额
** output parameters:   
                        PayoutNum--各通道实际出币数量						
** Returned value:      有硬币找出返回1，无返回0
*********************************************************************************************************/
unsigned char ChangePayoutProcessLevel3(unsigned int PayMoney,unsigned char *PayoutNum)
{
	unsigned char CoinRdBuff[36],CoinRdLen,ComStatus,VMCValue[2]={0},VMCPoll[1]={0};
	uint8_t coinscale,dispenseValue;
	uint8_t  i;	
	coinGetNums();
	OSTimeDly(OS_TICKS_PER_SEC / 10);

	coinscale = stDev.coin.scale;
	dispenseValue = PayMoney / coinscale;//发送找零基准数量			
	Trace("\r\nDrvChangescale = %d,%d", coinscale,dispenseValue);
	VMCValue[0] = 0x02;
	VMCValue[1] = dispenseValue;
	//1发送找币指令
	ComStatus = MdbConversation(0x0F,VMCValue,2,&CoinRdBuff[0],&CoinRdLen);
	if(ComStatus == 1)
	{
		Timer.PayoutTimer = dispenseValue/2 + 20;
		while(Timer.PayoutTimer)
		{					
			//2发送扩展poll指令，检测找币是否完成
			VMCPoll[0] = 0x04;
			ComStatus = MdbConversation(0x0F,VMCPoll,1,&CoinRdBuff[0],&CoinRdLen);
			Trace("\r\nDrvChangesend = %d,%d,%d,%d\r\n",ComStatus,CoinRdBuff[0],CoinRdBuff[1],CoinRdLen);
			//找零进行时，CoinRdLen=1 找零完成后，CoinRdLen = 0
			if( CoinRdLen == 0 )
			{
				//CoinRdLen = 0;
				//3发送扩展指令，检测本次找币各通道找多少枚
				VMCPoll[0] = 0x03;
				ComStatus = MdbConversation(0x0F,VMCPoll,1,&CoinRdBuff[0],&CoinRdLen);
				Trace("\r\nDrvChange s = %d,%d,%d,%d\r\n",ComStatus,CoinRdBuff[0],CoinRdBuff[1],CoinRdLen);
				if( CoinRdLen > 0 )
				{
					Trace("\r\nDrvChangeOut=%d,%d,%d,%d",CoinRdBuff[0],CoinRdBuff[1],CoinRdBuff[2],CoinRdBuff[3]);
					for(i = 0;i < 8;i++)
					{
						PayoutNum[i] = CoinRdBuff[i];
					}
					break;
				}
			}
			msleep(10);
		}
	}			
			
	return 0;
}


/*********************************************************************************************************
** Function name:       ChangePayoutProcessLevel2
** Descriptions:        level2找零操作
** input parameters:    PayType——出币通道
**                      PayNum ——出币数量
** output parameters:   PayoutNum--各通道实际出币数量							
** Returned value:      有硬币找出返回1，无返回0
*********************************************************************************************************/
unsigned char ChangePayoutProcessLevel2(unsigned char PayType,unsigned char PayNum, unsigned char PayoutNum[8])
{
	unsigned char ComStatus,VMCPoll[1]={0},CoinRdBuff[36],CoinRdLen;
	
	//5出币
	VMCPoll[0]  =  PayType&0x0f;
	VMCPoll[0] |= ((PayNum<<4)&0xf0);
	ComStatus = MdbConversation(0x0D,VMCPoll,1,&CoinRdBuff[0],&CoinRdLen);
	//6上传通道的出币枚数
	if(ComStatus == 1)
	{
		PayoutNum[PayType] = PayNum;	
		Trace("\r\nDrvChange nu=%d",PayoutNum[PayType]);
	}	
	return 0;
}


/*********************************************************************************************************
** Function name:       coinChange
** Descriptions:        找零操作
** input parameters:    PayMoney——需要找出的硬币金额
** output parameters:   remainMoney--剩余未找零金额						
** Returned value:      有硬币找出返回1，无返回0
*********************************************************************************************************/
unsigned char coinChange(unsigned int payMoney,unsigned int *changeMoney)
{	
	unsigned char i;
	*changeMoney = 0;
	if(payMoney == 0)
		return 1;
	ChangePayoutProcessLevel3(payMoney,stDev.coin.payoutNum);
	
	for(i = 0; i < 8; i++)
	{
		*changeMoney +=  stDev.coin.payoutNum[i] * stDev.coin.channel[i];		
	}

	return 1;
}


/*********************************************************************************************************
** Function name:       CoinDevProcessExpanse
** Descriptions:        扩展指令检查硬币器状态
** input parameters:    无
** output parameters:   
** Returned value:      有打开返回2，其他情况返回1,正常返回0
*********************************************************************************************************/
void CoinDevProcessExpanse(void)
{
    uint8_t CoinRdBuff[36],CoinRdLen,ComStatus,VMCdata[1]={0x05};
    unsigned char manCode,subCode;
	
    ComStatus = MdbConversation(0x0F,VMCdata,1,&CoinRdBuff[0],&CoinRdLen);
	if(ComStatus == 1 && CoinRdLen >= 2)	
    {  		
    	manCode = CoinRdBuff[0];
		subCode = CoinRdBuff[1];
		Trace("\r\nDrvCoin=%#02x,%#02x",manCode,subCode);
		if(	(manCode <= 0x05)||((manCode == 0x11)&&(subCode == 0x10)))//正常
		{
			Trace("\r\nDrvCoin ok");
			CLR_COIN_ERR(stDev.coin.curErrNo);
		}
		else if(manCode == 0x06)//被vmc禁能
		{
			Trace("\r\nDrvCoin disable");
			SET_COIN_ERR(stDev.coin.curErrNo,STATUS_COIN_ERR_DISABLE);
		}
		else//其他各种故障
		{
			Trace("\r\nDrvCoin unknow=%#02x",CoinRdBuff[0]);
			//SET_COIN_ERR(stDev.coin.curErrNo,STATUS_COIN_ERR_UNKNOW);
		}
		

    } 	  
}

/*********************************************************************************************************
** Function name:       coinTaskPoll
** Descriptions:        硬币器任务轮训
** input parameters:    无
** output parameters:   
** Returned value:      
*********************************************************************************************************/
unsigned char coinTaskPoll(void)
{
	static unsigned char flush = 0;
	if(stDev.coin.state == STATUS_COIN_COM || stDev.coin.state == STATUS_COIN_N_A)//通信故障 禁止轮训
		return 0;

	switch(flush)
	{
		case 0:case 2:
			coinPoll();
			break;
		case 1:case 3:
			CoinDevProcessExpanse();
			coinStatusUpdate();
			break;
		default:
			flush = 0;
			coinGetNums();//获取硬币器余量
			break;
	}
	flush++;
	
	return 0;
}



