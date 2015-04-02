#ifndef _BILL_API_H
#define _BILL_API_H

#define STATUS_BILL_N_A				0 //未知
#define STATUS_BILL_ENABLE			1 //正常
#define STATUS_BILL_FAULT			2 //故障
#define STATUS_BILL_QURBI			3 //缺币
#define STATUS_BILL_DISABLE			4 //禁能
#define STATUS_BILL_COM				5 //通信故障		

#define CTL_BILL_INIT				1//下发纸币器初始化
#define CTL_BILL_ENABLE				2//下发纸币器使能
#define CTL_BILL_DISABLE			3//下发纸币器禁能	

#define STATUS_BILL_ERR_COM				0x0001 //通信故障
#define STATUS_BILL_ERR_SENSOR			0x0002 //传感器故障
#define STATUS_BILL_ERR_TUBEJAM			0x0004 //出币口卡币
#define STATUS_BILL_ERR_ROM				0x0008 //rom出错
#define STATUS_BILL_ERR_ROUTING			0x0010 //进币通道出错
#define STATUS_BILL_ERR_JAM				0x0020 //投币卡币
#define STATUS_BILL_ERR_REMOVECASH		0x0040 //移除纸币钞箱
#define STATUS_BILL_ERR_DISABLE			0x0080 //禁能
#define STATUS_BILL_ERR_MOTO			0x0100 //马达故障
#define STATUS_BILL_ERR_CASH			0x0200 //纸币钞箱故障
#define STATUS_BILL_ERR_UNKNOW			0x8000 //其他各种故障


#define SET_BILL_ERR(state,errno)  (state |= errno)
#define CLR_BILL_ERR(state)  (state = 0)



typedef struct	_st_dev_bill_{
	unsigned char  pcDisabled;//上次下发纸币器命令  0-enable -disable
	unsigned short  curErrNo;//当前实时故障码
	unsigned char  comerr;//通信故障
	unsigned short errNo; //故障码
	unsigned char  state;//0 未知   1正常 2 禁能 3缺币 4故障
	unsigned char  level; //纸币器等级
	unsigned char  escrowFun;		 //暂存功能
	unsigned short code;		  //国家代码
	unsigned short scale;			  //比例因子
	unsigned short decimal;		  //10^小数位数
	unsigned short stkCapacity;	  //储币容量
	unsigned short security;	//安全等级
	unsigned int   channel[8];	 //通道面值
	unsigned char  IDENTITYBuf[36];  //Identification


}ST_DEV_BILL;


unsigned char billInit(void);
void billDisable(void);
void billEnable(void);
unsigned int billRecv(void);
void billReject(void);
unsigned char billPoll(void);
void billStatusUpdate(void);
unsigned char billTaskPoll(void);

#endif
