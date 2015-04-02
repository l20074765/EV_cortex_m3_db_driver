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
#ifndef _HOPPER_H_
#define _HOPPER_H_

//当前只做Hopper得指令，后续还有货道的操作指令等
#define EVB_HANDLE_HP_QUERY				(0x51)				//Hopper的查询操作指令
#define EVB_HANDLE_HP_OUTPUT			(0x50)				//Hopper的出币操作指令
#define HOPPER1_ADDR					0x00
#define HOPPER2_ADDR					0x01
#define HOPPER3_ADDR					0x02


#define HOPPER_NUMS                   3  //hopper个数
#define HOPPER_ADDR					{HOPPER1_ADDR,HOPPER2_ADDR,HOPPER3_ADDR}

//定义hopper斗 结构体
typedef struct __st_hopper_{

    unsigned char addr;//hopper 物理地址
    unsigned char no;//hopper编号
    unsigned char state;//hopper状态  0 正常 1 缺币 2 故障
    unsigned char channelValue;//通道面值
	unsigned char LastOutPutCount;//上次操作出币数量
	unsigned char levelFlag;//级别分配标志
	unsigned char isCycle;//循环斗标志
	unsigned char specialFault;//特殊故障标志
	


}ST_HOPPER;

extern ST_HOPPER stHopper[HOPPER_NUMS];//定义三个hopper斗 结构体数组

typedef struct _st_hopper_level_{

    unsigned char num;//本级别hopper个数
    //unsigned char useAbleNum;//本级别可用的hopper个数
    ST_HOPPER *hopper[HOPPER_NUMS];//定义hopper斗结构体指针数组
    unsigned char channelValue;//本级别通道面值
    unsigned char ratio;//对币比例因子


}ST_HOPPER_LEVEL;


//定义hopper级别 结构体数组 按数组依次划分级别 1 > 2 > 3
//例如 hopper1 hopper2 通道面值 1元 hopper3 通道面值为 5毛 则 hopper1 hopper2
//划分在级别1中,hopper3 划分在级别2中  级别3为空  即

extern ST_HOPPER_LEVEL stHopperLevel[HOPPER_NUMS];



void hopperInit(void);
void checkHopperAndUpdate(void);
unsigned char  hopperChangerHandle(unsigned int changeMoney,unsigned int *remainMoney);
uint8_t hopperOutput(unsigned short  count,unsigned char addr);
unsigned int evb_device_hp_dispence(unsigned int ChangeMoney,unsigned int *SurplusMoney,
		unsigned char *Hp1OutNum,unsigned char  *Hp2OutNum,unsigned char *Hp3OutNum)
;
unsigned short getHopperHandleTime(unsigned int value);
unsigned char vmcChangerHandle(unsigned int changedMoney,unsigned int *remainMoney,unsigned char changedType);

unsigned char  vmcBillChanger(unsigned int changeAmount,unsigned int *remainAmount);
unsigned char vmcCoinChanger(unsigned short changeAmount,unsigned int *remainAmount);
unsigned char vmcCheckCoinChanger(unsigned short changeAmount);

unsigned char vmcCheckBillChanger(unsigned short changeAmount);
uint32_t hopperChanger(uint32_t changedAmount,uint32_t *remainAmount,unsigned char type);
uint8_t evb_hopper_check(uint8_t address);
unsigned char hopperChangerFailCheck(const unsigned int remainAmount);
unsigned char getMinHopperPriceState(void);
uint8_t hopperTestOutput(unsigned short  count,unsigned char addr);
uint8_t hopperIsEmpty(void);
unsigned char HopperDispence(unsigned int DispenceMoney,unsigned int *iouMoney);


#endif

