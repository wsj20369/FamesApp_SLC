/*************************************************************************************
 * �ļ�:    slc/config.c
 *
 * ����:    slc�����ļ�
 *
 * ����:    Jun
 *
 * ʱ��:    2011-2-20
*************************************************************************************/
#define  SLC_CONFIG_C
#include <includes.h>
#include "common.h"

/*------------------------------------------------------------------------------------
 * 
 *          �ļ���
 * 
**----------------------------------------------------------------------------------*/
static char * config_fname = "config.slc";   /* �����ļ���                    */

#define config_magic   0x00434C53L           /* �����ļ���ʶ��"SLC"           */

struct slc_config_s config = {
            config_magic,               /* �ļ���Ч��ʶ */
            0,
            { 0x3F8, 38400L, 7, 1, COM_PARITY_EVEN, 4 }, /* ��1 ���� */
            { 0x2F8, 38400L, 7, 1, COM_PARITY_EVEN, 3 }, /* ��2 ���� */
            { 0x2F8,  9600L, 8, 1, COM_PARITY_NONE, 3 }, /* ���ߴ��� */
            /* ��ѹ��Ĭ�ϲ�����read_config()���� */
};

/*------------------------------------------------------------------------------------
 * ����:    read_config()
 *
 * ����:    ��ȡSLC�����ļ�
 *
 * ����:    ok/fail
 *
 * ע��:    �ڶ�ȡ�ɹ���,���������е�����
**----------------------------------------------------------------------------------*/
BOOL read_config(void)
{
    BOOL retval;
    int  fd = -1;

    retval = fail;
    
    lock_kernel();
    fd=open(config_fname, O_RDONLY|O_BINARY);
    if(fd<0){                                 /* �����ʧ��,��Ҫ���Դ����ļ�     */
        slc_setup_to_default(&config.slc[0]);
        slc_setup_to_default(&config.slc[1]);
        config.slc_used = 1; /* Ĭ��ֻʹ��һ̨�� */
        config.cim_data_delayed = 0;
        config.cim_protocol_type = 0;
        config.slc_start_mode = 0;
        config.language = 0;
        if(save_config()){
            fd=open(config_fname, O_RDONLY|O_BINARY);
        } else {
            retval=fail;                      /* ����ʧ��,�����Ǵ��̿ռ䲻��,�˳�! */
            goto out;
        }        
    }
    if(fd>=0){
        if(read(fd, (void *)&config, sizeof(config)) > 0){
            if(config.magic==config_magic && config.size==sizeof(config)){
                active_config();              /* ��ȡ�ɹ�,�������е�����            */
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
 * ����:    save_config()
 *
 * ����:    ����(�򴴽�)SLC�����ļ�
 *
 * ����:    ok/fail
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
 * ����:    active_config()
 *
 * ����:    ����SLC����
 *
 * ����:    ok/fail
**----------------------------------------------------------------------------------*/
BOOL active_config(void)
{
    slc_descriptor_t * slc;

    #define ____reset(x) MEMSET((INT08S*)&(x), 0, sizeof(x))
    
    /* ��1 */
    slc = &config.slc[0];
    slc->slc_speed = 0;
    ____reset(slc->k_whet);
    ____reset(slc->kl_set);
    ____reset(slc->kl_act);
    ____reset(slc->state);
    ____reset(slc->working);

    /* ��2 */
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
 * ����:    check_config()
 *
 * ����:    ���SLC���ò��������������
 *
 * ����:    ok/fail
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
 * ����:    copy_to_config()
 *
 * ����:    ��cfg������ȫ������config��
 *
 * ����:    ok/fail
 *
 * ˵��:    ���������Ҫ��Ϊ�˱�֤config�е�һЩ��̬���ݲ��ᱻ�ƻ�
**----------------------------------------------------------------------------------*/
BOOL copy_to_config(struct slc_config_s * cfg)
{
    slc_descriptor_t * slc;
    slc_descriptor_t * config_slc;
    FamesAssert(cfg);
    if(!cfg)
        return fail;
    
    lock_kernel();
    /* ��1�Ķ�̬���� */
    slc = &cfg->slc[0];
    config_slc = &config.slc[0];
    slc->slc_speed = config_slc->slc_speed;
    slc->k_whet    = config_slc->k_whet;
    slc->kl_act    = config_slc->kl_act;
    slc->state     = config_slc->state;
    /* ��2�Ķ�̬���� */
    slc = &cfg->slc[1];
    config_slc = &config.slc[1];
    slc->slc_speed = config_slc->slc_speed;
    slc->k_whet    = config_slc->k_whet;
    slc->kl_act    = config_slc->kl_act;
    slc->state     = config_slc->state;
    /* ���� */
    config = (*cfg);
    unlock_kernel();
    
    return ok;
}


/*------------------------------------------------------------------------------------
 * ����:    slc_is_trim_forced()
 *
 * ����:    �Ƿ��趨��ǿ���ޱ�
 *
 * ����:    1=Yes, 0=Auto
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
 * ���ļ�����: slc/config.c
 * 
**==================================================================================*/

