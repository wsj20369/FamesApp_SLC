/*************************************************************************************
 * �ļ�:    slc/yx_dat.c
 *
 * ����:    ��дѹ���ļ�yx.dat
 *
 * ����:    Jun
 *
 * ʱ��:    2011-4-4
*************************************************************************************/
#define  SLC_YX_DAT_C
#include <includes.h>
#include "common.h"


/*------------------------------------------------------------------------------------
 * 
 *          �ڲ�����
 * 
**----------------------------------------------------------------------------------*/
static char * __yx_string[YX_MAX_NR] = { NULL, };

static int    __yx_types = 0;


#if  0  /* ʹ��"yx.dat"��Ϊѹ�������ļ� */
/*------------------------------------------------------------------------------------
 * 
 *          �ļ���
 * 
**----------------------------------------------------------------------------------*/
static char * yx_dat_fname = "yx.dat";    /* ѹ�������ļ� */

static char   __yx_dat_image[64] = "����\r\nƽ��\r\n˫��\r\n";


/*------------------------------------------------------------------------------------
 * ����:    read_yx_dat()
 *
 * ����:    ��ȡ�ļ�"yx.dat"
 *
 * ����:    ok/fail
 *
 * ˵��:    �˺���ֻ�ܱ�����һ��(�����ڳ�ʼ��ʱ)
**----------------------------------------------------------------------------------*/
BOOL read_yx_dat(void)
{
    BOOL retval;
    int  fd = -1;
    int  i, j;
    int  skip, s;

    retval = fail;
    
    lock_kernel();
    fd=open(yx_dat_fname, O_RDONLY|O_BINARY);
    if(fd<0){                                 /* �����ʧ��,��Ҫ���Դ����ļ�     */
        fd=open(yx_dat_fname, O_WRONLY|O_BINARY|O_CREAT, S_IREAD|S_IWRITE);
        if(fd>=0){
            write(fd, (void *)&__yx_dat_image, STRLEN(__yx_dat_image));
            close(fd);
            fd=open(yx_dat_fname, O_RDONLY|O_BINARY);
        } else {
            retval=fail;                      /* ����ʧ��,�����Ǵ��̿ռ䲻��,�˳�! */
            __yx_types = 3;
            goto out;
        }        
    }
    if(fd>=0){
        i = read(fd, (void *)&__yx_dat_image, (sizeof(__yx_dat_image)-4));
        if( i > 0){ /* ��������ȡ���е�ѹ���ַ��� */
            __yx_dat_image[i] = 0;
            /* ���������ַ� */
            for(i=0; __yx_dat_image[i]; i++){
                if(__yx_dat_image[i] & 0xE0) /* >= 0x20 */
                    break;
            }
            skip = 0;
            __yx_string[0] = &__yx_dat_image[i];
            __yx_types = 1;
            for(j=0; __yx_dat_image[i]; i++){
                if(skip && (__yx_dat_image[i] & 0xE0)){
                    __yx_string[j] = &__yx_dat_image[i];
                    skip = 0;
                    __yx_types++;
                }
                if(!skip && __yx_dat_image[i] == '\n'){
                    skip = 1;
                    __yx_dat_image[i] = 0;
                    s = STRLEN(__yx_string[j]);
                    if(s > 8)  /* ��󳤶ȹ̶�Ϊ8���ַ� */
                        __yx_string[j][8] = 0;
                    j++;
                }
                if(!(__yx_dat_image[i] & 0xE0)){
                    __yx_dat_image[i] = 0;
                }
                if(j >= YX_MAX_NR)
                    break;
            }
            retval = ok;
        } else {
            sys_print("file yx.dat is empty!");
            ExitApplication();
        }
        close(fd);
    }

out:
    unlock_kernel();
    return retval;
}

#else  /* ʹ��load_string����ѹ�������ַ���, ��֧����Ӣ���л� */

/*------------------------------------------------------------------------------------
 * ����:    read_yx_dat()
 *
 * ����:    ��ȡѹ�������ַ���(ͨ��load_string())
 *
 * ����:    ok/fail
 *
 * ˵��:    �˺���ֻ�ܱ�����һ��(�����ڳ�ʼ��ʱ)
**----------------------------------------------------------------------------------*/
static char __yx_str_buf[YX_MAX_NR][12] = {
                "����", "ƽ��", "˫��",  /* ǰ3��ѹ�͵�Ĭ��ֵ */
};

BOOL read_yx_dat(void)
{
    int i;
    char ___s[32];

    for(i = 3; i < YX_MAX_NR; i++){
        __yx_str_buf[i][0] = 0;
    }
    for(i = 0; i < YX_MAX_NR; i++){
        sprintf(___s, "press_type_%d", (i+1));
        load_string(__yx_str_buf[i], 9, ___s); /* ���8���ֽ�(9-1) */
        if(__yx_str_buf[i][0]){ /* �ַ�����Ч */
            __yx_types++;
            __yx_string[i] = __yx_str_buf[i];
        } else {
            __yx_string[i] = NULL;
            break;
        }
    }

    return ok;
}

#endif


/*------------------------------------------------------------------------------------
 * ����:    get_yx_types()
 *
 * ����:    ����ѹ������
 *
 * ����:    ok/fail
**----------------------------------------------------------------------------------*/
int get_yx_types(void)
{
    return __yx_types;
}


/*------------------------------------------------------------------------------------
 * ����:    get_yx_string()
 *
 * ����:    ����(�򴴽�)SLC��ʱ�����ļ�
 *
 * ����:    ok/fail
**----------------------------------------------------------------------------------*/
char * get_yx_string(int yx)
{
    char * t = NULL;

    yx--;  /* ѹ�ʹ����Ǵ�1��ʼ�� */

    if(yx >= 0 && yx < YX_MAX_NR){
        t = __yx_string[yx];
    }
    
    if(t)
        return t;
    else
        return "-/-";
}


/*====================================================================================
 * 
 * ���ļ�����: slc/yx_dat.c
 * 
**==================================================================================*/

