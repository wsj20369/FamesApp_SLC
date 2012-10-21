/*************************************************************************************
 * 文件:    slc/config.c
 *
 * 描述:    slc配置文件
 *
 * 作者:    Jun
 *
 * 时间:    2011-2-20
*************************************************************************************/
#define  SLC_CONFIG_C
#include <includes.h>
#include "common.h"

/*------------------------------------------------------------------------------------
 * 
 *          文件名
 * 
**----------------------------------------------------------------------------------*/
static char * config_fname = "config.slc";   /* 配置文件名                    */

#define config_magic   0x00434C53L           /* 配置文件标识字"SLC"           */

struct slc_config_s config = {
            config_magic,               /* 文件有效标识 */
            0,
            { 0x3F8, 38400L, 7, 1, COM_PARITY_EVEN, 4 }, /* 机1 串口 */
            { 0x2F8, 38400L, 7, 1, COM_PARITY_EVEN, 3 }, /* 机2 串口 */
            { 0x2F8,  9600L, 8, 1, COM_PARITY_NONE, 3 }, /* 连线串口 */
            /* 分压机默认参数由read_config()设置 */
};

/*------------------------------------------------------------------------------------
 * 函数:    read_config()
 *
 * 描述:    读取SLC配置文件
 *
 * 返回:    ok/fail
 *
 * 注意:    在读取成功后,激活了其中的设置
**----------------------------------------------------------------------------------*/
BOOL read_config(void)
{
    BOOL retval;
    int  fd = -1;

    retval = fail;
    
    lock_kernel();
    fd=open(config_fname, O_RDONLY|O_BINARY);
    if(fd<0){                                 /* 如果打开失败,则要尝试创建文件     */
        slc_setup_to_default(&config.slc[0]);
        slc_setup_to_default(&config.slc[1]);
        config.slc_used = 1; /* 默认只使用一台机 */
        config.cim_data_delayed = 0;
        config.cim_protocol_type = 0;
        config.slc_start_mode = 0;
        config.language = 0;
        if(save_config()){
            fd=open(config_fname, O_RDONLY|O_BINARY);
        } else {
            retval=fail;                      /* 创建失败,可能是磁盘空间不足,退出! */
            goto out;
        }        
    }
    if(fd>=0){
        if(read(fd, (void *)&config, sizeof(config)) > 0){
            if(config.magic==config_magic && config.size==sizeof(config)){
                active_config();              /* 读取成功,激活其中的设置            */
                retval = ok;
            } else {
                sys_print("Error: config.slc has been corruptted!!!\n\n"
                          "  remove it manually, then it will be auto-generated!\n\n"
                          "Any key to exit\n");
                getch();
                ExitApplication();
            }
        }
        close(fd);
    }

out:
    unlock_kernel();
    return retval;
}

/*------------------------------------------------------------------------------------
 * 函数:    save_config()
 *
 * 描述:    保存(或创建)SLC配置文件
 *
 * 返回:    ok/fail
**----------------------------------------------------------------------------------*/
BOOL save_config(void)
{
    BOOL retval;
    int  fd = -1;

    retval = fail;
    
    lock_kernel();
    fd=open(config_fname, O_WRONLY|O_BINARY|O_CREAT, S_IREAD|S_IWRITE);
    if(fd>=0){
        check_config();
        if(write(fd, (void *)&config, sizeof(config)) > 0){
            retval=ok;
        }
        close(fd);
    }
    unlock_kernel();
    
    return retval;
}

/*------------------------------------------------------------------------------------
 * 函数:    active_config()
 *
 * 描述:    激活SLC设置
 *
 * 返回:    ok/fail
**----------------------------------------------------------------------------------*/
BOOL active_config(void)
{
    slc_descriptor_t * slc;

    #define ____reset(x) MEMSET((INT08S*)&(x), 0, sizeof(x))
    
    /* 机1 */
    slc = &config.slc[0];
    slc->slc_speed = 0;
    ____reset(slc->k_whet);
    ____reset(slc->kl_set);
    ____reset(slc->kl_act);
    ____reset(slc->state);
    ____reset(slc->working);

    /* 机2 */
    slc = &config.slc[1];
    slc->slc_speed = 0;
    ____reset(slc->k_whet);
    ____reset(slc->kl_set);
    ____reset(slc->kl_act);
    ____reset(slc->state);
    ____reset(slc->working);

    return ok;

    #undef ____reset
}

/*------------------------------------------------------------------------------------
 * 函数:    check_config()
 *
 * 描述:    检测SLC配置并更正错误的设置
 *
 * 返回:    ok/fail
**----------------------------------------------------------------------------------*/
BOOL check_config(void)
{
    lock_kernel();
    if(config.magic!=config_magic){
        config.magic=config_magic;
    }
    if(config.size!=sizeof(config)){
        config.size=sizeof(config);
    }
    unlock_kernel();
    return ok;
}

/*------------------------------------------------------------------------------------
 * 函数:    copy_to_config()
 *
 * 描述:    将cfg拷贝到全局配置config中
 *
 * 返回:    ok/fail
 *
 * 说明:    这个函数主要是为了保证config中的一些动态数据不会被破坏
**----------------------------------------------------------------------------------*/
BOOL copy_to_config(struct slc_config_s * cfg)
{
    slc_descriptor_t * slc;
    slc_descriptor_t * config_slc;
    FamesAssert(cfg);
    if(!cfg)
        return fail;
    
    lock_kernel();
    /* 机1的动态数据 */
    slc = &cfg->slc[0];
    config_slc = &config.slc[0];
    slc->slc_speed = config_slc->slc_speed;
    slc->k_whet    = config_slc->k_whet;
    slc->kl_act    = config_slc->kl_act;
    slc->state     = config_slc->state;
    /* 机2的动态数据 */
    slc = &cfg->slc[1];
    config_slc = &config.slc[1];
    slc->slc_speed = config_slc->slc_speed;
    slc->k_whet    = config_slc->k_whet;
    slc->kl_act    = config_slc->kl_act;
    slc->state     = config_slc->state;
    /* 拷贝 */
    config = (*cfg);
    unlock_kernel();
    
    return ok;
}


/*------------------------------------------------------------------------------------
 * 函数:    slc_is_trim_forced()
 *
 * 描述:    是否设定了强制修边
 *
 * 返回:    1=Yes, 0=Auto
**----------------------------------------------------------------------------------*/
int slc_is_trim_forced(void)
{
    int trim_flag = 0;

    if (((config.slc_used & 1) && (config.slc[0].slc_flag & SLC_FLAG_TRIM)) ||
        ((config.slc_used & 2) && (config.slc[1].slc_flag & SLC_FLAG_TRIM)))
        trim_flag = 1;

    return trim_flag;
}


/*====================================================================================
 * 
 * 本文件结束: slc/config.c
 * 
**==================================================================================*/

