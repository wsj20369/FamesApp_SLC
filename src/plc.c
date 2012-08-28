/***********************************************************************************************
 * 文件:    slc/plc.c
 *
 * 说明:    PLC控制部分
 *
 * 作者:    Jun
 *
 * 时间:    2010-12-18
***********************************************************************************************/
#define  SLC_PLC_C
#include <includes.h>
#include "common.h"

#include "plc_def.h"

PLC    * slc_plc[2]  = { NULL, NULL };    /* 两个机器的PLC */

int      plc_connected[2] = { 1, 1 };     /* PLC的连接状态 */

int      plc_tick[2] = { 0, };            /* PLC时钟 */
int    * plc_io_x[2] = { NULL, };
int    * plc_io_y[2] = { NULL, };
INT32S   slc_malfunction[2] = { 0L, 0L }; /* SLC故障位图 */

void slc_open_plc(void)
{
    int used;
    extern struct slc_config_s config;

    lock_kernel();
    used = config.slc_used;
    unlock_kernel();

    if(used & 0x1){
        if(!m1_start_plc()){
            lock_kernel();
            sys_print("SLC-1: failed to start plc!\n");
            getch();
            unlock_kernel();
            quit();
        }
    }
    if(used & 0x2){
        if(!m2_start_plc()){
            lock_kernel();
            sys_print("SLC-2: failed to start plc!\n");
            getch();
            unlock_kernel();
            quit();
        }
    }
}

void slc_close_plc(void)
{
    if(!m1_stop_plc()){
        /* ... on failed */
    }
    if(!m2_stop_plc()){
        /* ... on failed */
    }
}

void ___slc_plc_rw(int slc_index, int cmd, void * addr, void * associated, int number)
{
    FamesAssert(slc_index == 1 || slc_index == 2);
    FamesAssert(addr);
    FamesAssert(associated);
    FamesAssert(number > 0);

    if(!(slc_index == 1 || slc_index == 2) || !addr || !associated || number <= 0)
        return;

    slc_index--;

    if(slc_plc[slc_index] && plc_connected[slc_index])
        plc_rw(slc_plc[slc_index], cmd, addr, associated, number);
}

void ___slc_plc_rw_ensure(int slc_index, int cmd, void * addr, void * associated, int number)
{
    FamesAssert(slc_index == 1 || slc_index == 2);
    FamesAssert(addr);
    FamesAssert(associated);
    FamesAssert(number > 0);

    if(!(slc_index == 1 || slc_index == 2) || !addr || !associated || number <= 0)
        return;

    slc_index--;

    if(slc_plc[slc_index] && plc_connected[slc_index])
        plc_rw_ensure(slc_plc[slc_index], cmd, addr, associated, number);
}


/*==============================================================================================
 * 
 * 本文件结束: slc/plc.c
 * 
**============================================================================================*/

