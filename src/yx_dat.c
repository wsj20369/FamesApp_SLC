/*************************************************************************************
 * 文件:    slc/yx_dat.c
 *
 * 描述:    读写压型文件yx.dat
 *
 * 作者:    Jun
 *
 * 时间:    2011-4-4
*************************************************************************************/
#define  SLC_YX_DAT_C
#include <includes.h>
#include "common.h"


/*------------------------------------------------------------------------------------
 * 
 *          内部变量
 * 
**----------------------------------------------------------------------------------*/
static char * __yx_string[YX_MAX_NR] = { NULL, };

static int    __yx_types = 0;


#if  0  /* 使用"yx.dat"作为压型描述文件 */
/*------------------------------------------------------------------------------------
 * 
 *          文件名
 * 
**----------------------------------------------------------------------------------*/
static char * yx_dat_fname = "yx.dat";    /* 压型配置文件 */

static char   __yx_dat_image[64] = "凹尖\r\n平尖\r\n双尖\r\n";


/*------------------------------------------------------------------------------------
 * 函数:    read_yx_dat()
 *
 * 描述:    读取文件"yx.dat"
 *
 * 返回:    ok/fail
 *
 * 说明:    此函数只能被调用一次(比如在初始化时)
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
    if(fd<0){                                 /* 如果打开失败,则要尝试创建文件     */
        fd=open(yx_dat_fname, O_WRONLY|O_BINARY|O_CREAT, S_IREAD|S_IWRITE);
        if(fd>=0){
            write(fd, (void *)&__yx_dat_image, STRLEN(__yx_dat_image));
            close(fd);
            fd=open(yx_dat_fname, O_RDONLY|O_BINARY);
        } else {
            retval=fail;                      /* 创建失败,可能是磁盘空间不足,退出! */
            __yx_types = 3;
            goto out;
        }        
    }
    if(fd>=0){
        i = read(fd, (void *)&__yx_dat_image, (sizeof(__yx_dat_image)-4));
        if( i > 0){ /* 分析并提取其中的压型字符串 */
            __yx_dat_image[i] = 0;
            /* 跳过控制字符 */
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
                    if(s > 8)  /* 最大长度固定为8个字符 */
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

#else  /* 使用load_string加载压型描述字符串, 可支持中英文切换 */

/*------------------------------------------------------------------------------------
 * 函数:    read_yx_dat()
 *
 * 描述:    读取压型描述字符串(通过load_string())
 *
 * 返回:    ok/fail
 *
 * 说明:    此函数只能被调用一次(比如在初始化时)
**----------------------------------------------------------------------------------*/
static char __yx_str_buf[YX_MAX_NR][12] = {
                "凹尖", "平尖", "双尖",  /* 前3个压型的默认值 */
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
        load_string(__yx_str_buf[i], 9, ___s); /* 最多8个字节(9-1) */
        if(__yx_str_buf[i][0]){ /* 字符串有效 */
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
 * 函数:    get_yx_types()
 *
 * 描述:    返回压型种类
 *
 * 返回:    ok/fail
**----------------------------------------------------------------------------------*/
int get_yx_types(void)
{
    return __yx_types;
}


/*------------------------------------------------------------------------------------
 * 函数:    get_yx_string()
 *
 * 描述:    保存(或创建)SLC临时配置文件
 *
 * 返回:    ok/fail
**----------------------------------------------------------------------------------*/
char * get_yx_string(int yx)
{
    char * t = NULL;

    yx--;  /* 压型代码是从1开始的 */

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
 * 本文件结束: slc/yx_dat.c
 * 
**==================================================================================*/

