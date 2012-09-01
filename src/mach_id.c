/*************************************************************************************
 * �ļ�:    slc/mach_id.c
 *
 * ����:    ��ȡ����ID��(����ע��)
 *
 * ����:    Jun
 *
 * ʱ��:    2012-9-2
 *
 * ˵��:    Ŀǰֻ�ܶ�ȡӲ��ID(ֻ�ܼ���1��IDE�豸(IDE0))
*************************************************************************************/
#define  MACHINE_ID_C
#include <includes.h>
#include "common.h"

#define IDE_SN_START_WORD 10
#define IDE_SN_WORD_COUNT 10

void __do_read_id(unsigned char __BUF * buf, int count)
{
    unsigned int dd[256]; /* DiskData */
    unsigned int dd_off; /* DiskData offset */
    unsigned char ide_sn_string[64];
    int  length, i, j;
    long wait;

    FamesAssert(buf != NULL);

    /* ��Ӳ�����к� */
    lock_kernel();
    wait = 20000L;
    while (inportb(0x1F7) != 0x50) { /* Wait for controller not busy */
        wait--;
        if (wait <= 0L)
            break;
    }
    outportb(0x1F6, 0xA0); /* Get first/second drive */
    outportb(0x1F7, 0xEC); /* Get drive info data */
    wait = 20000L;
    while (inportb(0x1F7) != 0x58) { /* Wait for data ready */
        wait--;
        if (wait <= 0L)
            break;
    }
    for (dd_off = 0; dd_off != 256; dd_off++) /* Read "sector" */
        dd[dd_off] = inport(0x1F0);
    unlock_kernel();

    /* �����кŷ������, ����һ���ַ��� */
    for (i = 0, j = 0; i < IDE_SN_WORD_COUNT; i++) {
        int tmp;
        tmp = dd[IDE_SN_START_WORD+i];
        ide_sn_string[j++] = (unsigned char)(tmp >> 8); /* Get High byte */
        ide_sn_string[j++] = (unsigned char)(tmp & 0xff); /* Get Low byte */
    }
    length = STRLEN((INT08S *)ide_sn_string);

    /* ������ַ���д���������ȥ */
    MEMSET((INT08S *)buf, 0, count);
    for (i = 0, j = 0; i < length; i++) {
        buf[j] = (buf[j] << 1) + ide_sn_string[i];
        j++;
        j %= count;
    }
}

int machine_id_get(unsigned char __BUF * buf, int buf_len)
{
    static int readed = 0;
    static unsigned char ____machine_hard_id[12];

    if (!readed) {
        __do_read_id(____machine_hard_id, 8); /* Ӳ��IDʵ����ֻ��8λ */
    }

    readed = 1;
    MEMSET((INT08S *)buf, 0, buf_len);
    if (buf_len > sizeof(____machine_hard_id))
        buf_len = sizeof(____machine_hard_id);
    MEMCPY((INT08S *)buf, (INT08S *)____machine_hard_id, buf_len);

    return ok;
}


/*====================================================================================
 * 
 * ���ļ�����: slc/mach_id.c
 * 
**==================================================================================*/

