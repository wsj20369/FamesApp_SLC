/*************************************************************************************
 * 文件:    slc/io_card.h
 *
 * 描述:    分压机I/O卡
 *
 * 作者:    Jun
 *
 * 时间:    2011-4-8
*************************************************************************************/
#ifndef SLC_IO_CARD_H
#define SLC_IO_CARD_H


/*------------------------------------------------------------------------------------
 * 
 *      函数声明
 * 
**----------------------------------------------------------------------------------*/
void  IoCardInit(void);
void  IoSet(void);
int   IsIoCardOk(void);
void  K1DownF(void);
void  K1UpF(void);
void  K2DownF(void);
void  K2UpF(void);
void  F1ixOk(void);
void  F1ixCel(void);
void  F2ixOk(void);
void  F2ixCel(void);
void  S1tarton(void);
void  S1tartdff(void);
void  S2tarton(void);
void  S2tartdff(void);


#endif /* #ifndef SLC_IO_CARD_H */

/*====================================================================================
 * 
 * 本文件结束: slc/io_card.h
 * 
**==================================================================================*/

