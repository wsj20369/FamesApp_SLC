/*************************************************************************************
 * 文件:    slc/io_card.c
 *
 * 描述:    分压机I/O卡
 *
 * 作者:    Jun
 *
 * 时间:    2011-4-8
 *
 * 说明:    此文件拷贝自旧版程序(实际上已经不再需要了, -- Jun, 2011-4-8)
*************************************************************************************/
#define  SLC_IO_CARD_C
#include <includes.h>
#include "common.h"

/**********************************************************
**********************************************************/
#define  PAD1A      0x190           /* 8255 */
#define  PAD1B      0x191
#define  PAD1C      0x192
#define  CONTROL1   0x193
#define  PAD2A      0x194
#define  PAD2B      0x195
#define  PAD2C      0x196
#define  CONTROL2   0x197
#define  PAD3A      0x198
#define  PAD3B      0x199
#define  PAD3C      0x19A
#define  CONTROL3   0x19B
#define  PCNC1      0x80
#define  PCNC2      0x89
#define  PCNC3      0x89

#define  MODE1      0x89
#define  MODE2      0x80
#define  MODE3      0x93

INT08U  ioA_port  =  0xff;
INT08U  ioB_port  =  0xff;

#define ___wait_io_ops() do { int a,b=1; a = 1000; while(a--)b=a*b; } while(0)

/**********************************************************
**********************************************************/
void IoCardInit(void)
{
    outportb(CONTROL1,MODE1);
    outportb(PAD1A,0xff);
    outportb(PAD1B,0xff);
    outportb(CONTROL2,MODE1);
    outportb(PAD2A,0xff);
    outportb(PAD2B,0xff);
    ___wait_io_ops();
}

/**********************************************************
**********************************************************/
void IoSet(void)
{
    outportb(CONTROL1,MODE1);
    outportb(CONTROL2,MODE1);
    ___wait_io_ops();
}

/**********************************************************
**********************************************************/
int IsIoCardOk(void)
{
    unsigned char  Read_port, Read_word;
    unsigned char  XPASSWORD = 0xb0;

    /* Read Password    */
    outportb( CONTROL3, MODE3 );
    outportb( PAD3C, XPASSWORD );
    ___wait_io_ops();
    Read_port = inportb( PAD3C );
    Read_word = ( Read_port & 0x0f );
    if ( Read_word == 0x09 )    /*         */
        return(1);
    else
        return(0);
}

/**********************************************************
**********************************************************/
void K1DownF(void)
{
    unsigned char  temp_port;

    temp_port = ioA_port & 0xdf ;
    ioA_port = temp_port;
    outportb( PAD1A,  temp_port );
    ___wait_io_ops();
}

/**********************************************************
**********************************************************/
void K1UpF(void)
{
    unsigned char  temp_port;

    temp_port = ioA_port | 0x20 ;
    ioA_port = temp_port;
    outportb( PAD1A,  temp_port );
    ___wait_io_ops();
}

/**********************************************************
**********************************************************/
void K2DownF(void)
{
    unsigned char  temp_port;

    temp_port = ioB_port & 0xdf ;
    ioB_port = temp_port;
    outportb( PAD1B,  temp_port );
    ___wait_io_ops();
}

/**********************************************************
**********************************************************/
void K2UpF(void)
{
    unsigned char  temp_port;

    temp_port = ioB_port | 0x20 ;
    ioB_port = temp_port;
    outportb( PAD1B,  temp_port );
    ___wait_io_ops();
}

/**********************************************************
**********************************************************/
void F1ixOk(void)
{
    unsigned char  temp_port;

    temp_port = ioA_port & 0xbf ;
    ioA_port = temp_port;
    outportb( PAD1A,  temp_port );
    ___wait_io_ops();
}

/**********************************************************
**********************************************************/
void F1ixCel(void)
{
    unsigned char  temp_port;

    temp_port = ioA_port | 0x40 ;
    ioA_port = temp_port;
    outportb( PAD1A,  temp_port );
    ___wait_io_ops();
}

/**********************************************************
**********************************************************/
void F2ixOk(void)
{
    unsigned char  temp_port;

    temp_port = ioB_port & 0xbf ;
    ioB_port = temp_port;
    outportb( PAD1B,  temp_port );
    ___wait_io_ops();
}

/**********************************************************
**********************************************************/
void F2ixCel(void)
{
    unsigned char  temp_port;

    temp_port = ioB_port | 0x40 ;
    ioB_port = temp_port;
    outportb( PAD1B,  temp_port );
    ___wait_io_ops();
}

/**********************************************************
**********************************************************/
void S1tarton(void)
{
    unsigned char  temp_port;

    temp_port = ioA_port & 0xef ;
    ioA_port = temp_port;
    outportb( PAD1A,  temp_port );
    ___wait_io_ops();
}

/**********************************************************
**********************************************************/
void S1tartdff(void)
{
    unsigned char  temp_port;

    temp_port = ioA_port | 0x10 ;
    ioA_port = temp_port;
    outportb( PAD1A,  temp_port );
    ___wait_io_ops();
}

/**********************************************************
**********************************************************/
void S2tarton(void)
{
    unsigned char  temp_port;

    temp_port = ioB_port & 0xef ;
    ioB_port = temp_port;
    outportb( PAD1B,  temp_port );
    ___wait_io_ops();
}

/**********************************************************
**********************************************************/
void S2tartdff(void)
{
    unsigned char  temp_port;

    temp_port = ioB_port | 0x10 ;
    ioB_port = temp_port;
    outportb( PAD1B,  temp_port );
    ___wait_io_ops();
}


/*====================================================================================
 * 
 * 本文件结束: slc/io_card.c
 * 
**==================================================================================*/

