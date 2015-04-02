#ifndef _COIN_API_H
#define _COIN_API_H


#define STATUS_COIN_N_A					0 //未知
#define STATUS_COIN_ENABLE				1 //正常
#define STATUS_COIN_FAULT				2 //故障
#define STATUS_COIN_QURBI				3 //缺币
#define STATUS_COIN_DISABLE				4 //软件禁能
#define STATUS_COIN_COM					5 //通信故障
#define CTL_COIN_INIT				1//下发硬币器初始化
#define CTL_COIN_ENABLE				2//下发硬币器使能
#define CTL_COIN_DISABLE			3//下发硬币器禁能
#define CTL_COIN_PAYOUT				4//下发硬币器找零

#define STATUS_COIN_ERR_COM				0x0001 //通信故障
#define STATUS_COIN_ERR_SENSOR			0x0002 //传感器故障
#define STATUS_COIN_ERR_TUBEJAM			0x0004 //出币口卡币
#define STATUS_COIN_ERR_ROM				0x0008 //rom出错
#define STATUS_COIN_ERR_ROUTING			0x0010 //进币通道出错
#define STATUS_COIN_ERR_JAM				0x0020 //投币卡币
#define STATUS_COIN_ERR_REMOVETUBE		0x0040 //硬币斗移开
#define STATUS_COIN_ERR_DISABLE			0x0100 //软件禁能
#define STATUS_COIN_ERR_UNKNOW			0x8000 //硬币其他各种故障


#define SET_COIN_ERR(state,errno)  	do{state |= errno; }while(0)
#define CLR_COIN_ERR(state)  		(state = 0)
 



//硬币器相关参数
typedef struct _st_dev_coin_{
	
	unsigned char  pcDisabled;//上次下发的硬币器 0-使能  1-禁能
	unsigned short curErrNo;
	unsigned char  quebi;//缺币故障
	unsigned char  comerr;//通信故障
	unsigned char  state;//
	unsigned short errNo;				//故障码		
	unsigned char  level;				//硬币器等级
	unsigned char  scale;			 	//比例因子
	unsigned short code;			 	//国家代码
	unsigned short decimal;		 		//10^小数位数
	unsigned short routing;		 		//Bit is set to indicate a coin type can be routed to the tube
	unsigned short chNum[8];	     	//各个通道存币数量
	unsigned char  payoutNum[8];	    //一次找零命令各个通道找币数量
	unsigned char  identity[36];		//Identification
	unsigned int   channel[8];	  		//各个通道面值


}ST_DEV_COIN;


unsigned char coinPoll(void);
unsigned int coinRecv(void);
void coinGetNums(void);
void coinDisable(void);
void coinEnable(void);
unsigned char coinInit(void);
unsigned char coinChange(unsigned int payMoney,unsigned int *changeMoney);
unsigned char coinTaskPoll(void);


#endif
