
#ifndef _PCCONAPI_H_
#define _PCCONAPI_H_

//add by yoc
#define LED_DB    		(1ul << 26)//P3.26×ª½Ó°å ÓÃ»§LEDµÆ	
#define SET_ULED_ON()	do{FIO3DIR |= LED_DB; FIO3CLR |= LED_DB;}while(0)
#define SET_ULED_OFF()	do{FIO3DIR |= LED_DB; FIO3SET |= LED_DB;}while(0)

#define PCD_HEAD  0xE5
#define DPC_HEAD  0xE6

#define print_err(...)



//Ö½Ó²±ÒÆ÷Çý¶¯°å×´Ì¬½á¹¹Ìå
typedef struct _bcdb_st_{

	unsigned char status;//ÕûÌå×´Ì¬0¿ÕÏÐ 1Ã¦
	unsigned char billType;//Ö½±ÒÆ÷ÀàÐÍ2 MDB
	unsigned char coinType;//Ó²±ÒÆ÷ÀàÐÍ2 MDB
	unsigned char billStatus;//Ö½±ÒÆ÷×´Ì¬0 ²»´æÔÚ 1Õý³£ 2½ûÄÜ 3¹ÊÕÏ 
	unsigned char coinStatus;//Ó²±ÒÆ÷×´Ì¬0²»´æÔÚ 1Õý³£ 2½ûÄÜ 3¹ÊÕÏ 
	unsigned int  billAmount;//Ö½±ÒÆ÷µ±Ç°ÊÕ±Ò½ð¶î
	unsigned int  coinAmount;//Ó²±ÒÆ÷µ±Ç°ÊÕ±Ò½ð¶î
	unsigned int  billChange;//Ö½±ÒÆ÷ÉÏ´ÎÕÒÁã½ð¶î
	unsigned int  coinChange;//Ó²±ÒÆ÷ÉÏ´ÎÕÒÁã½ð¶î
	unsigned char payout;//ÍË±Ò°´Å¥±êÖ¾

}BCDB_ST;






void pcCon(void);
void pcdCoinEscrow(void);




#endif
