/*************************************************************************************
 * �ļ�:    slc/tmp_cfg.c
 *
 * ����:    SLC��ʱ�����ļ�
 *
 * ����:    Jun
 *
 * ʱ��:    2011-4-2
*************************************************************************************/
#define  SLC_TMP_CONFIG_C
#include <includes.h>
#include "common.h"

/*------------------------------------------------------------------------------------
 * 
 *          �ļ���
 * 
**----------------------------------------------------------------------------------*/
static char * tmp_config_fname = "tmp.slc";   /* �����ļ���                    */

struct slc_tmp_config_s tmp_config = {
    0, 0, 0, 0
};

/*------------------------------------------------------------------------------------
 * ����:    read_tmp_config()
 *
 * ����:    ��ȡSLC��ʱ�����ļ�
 *
 * ����:    ok/fail
**----------------------------------------------------------------------------------*/
BOOL read_tmp_config(void)
{
    BOOL retval;
    int  fd = -1;

    retval = fail;
    
    lock_kernel();
    fd=open(tmp_config_fname, O_RDONLY|O_BINARY);
    if(fd<0){                                 /* �����ʧ��,��Ҫ���Դ����ļ�     */
        if(save_tmp_config()){
            fd=open(tmp_config_fname, O_RDONLY|O_BINARY);
        } else {
            retval=fail;                      /* ����ʧ��,�����Ǵ��̿ռ䲻��,�˳�! */
            goto out;
        }        
    }
    if(fd>=0){
        if(read(fd, (void *)&tmp_config, sizeof(tmp_config)) > 0)
            retval = ok;
        close(fd);
    }

out:
    unlock_kernel();
    return retval;
}

/*------------------------------------------------------------------------------------
 * ����:    save_tmp_config()
 *
 * ����:    ����(�򴴽�)SLC��ʱ�����ļ�
 *
 * ����:    ok/fail
**----------------------------------------------------------------------------------*/
BOOL save_tmp_config(void)
{
    BOOL retval;
    int  fd = -1;

    retval = fail;
    
    lock_kernel();
    fd=open(tmp_config_fname, O_WRONLY|O_BINARY|O_CREAT, S_IREAD|S_IWRITE);
    if(fd>=0){
        if(write(fd, (void *)&tmp_config, sizeof(tmp_config)) > 0){
            retval=ok;
        }
        close(fd);
    }
    unlock_kernel();
    
    return retval;
}


/*====================================================================================
 * 
 * ���ļ�����: slc/tmp_cfg.c
 * 
**==================================================================================*/

