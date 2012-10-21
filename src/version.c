/*************************************************************************************
 * 文件:    slc/version.c
 *
 * 说明:    SLC版本号
 *
 * 作者:    Jun
 *
 * 创建:    2011-4-8
*************************************************************************************/
#define  SLC_VERSION_C
#include <FamesOS.h>
#include "common.h"

/*------------------------------------------------------------------------------------
 *                  SLC版本号(VERSION)
 * 
 * 说明: 版本号由两部分组成:
 *
 *       1) major:  主版本号, 主版本号的变化代表了较大的或者较本质的改变
 *       2) minor:  次版本号, 次版本号的变化代表了在部分功能上有所加强或改变
**----------------------------------------------------------------------------------*/
char  ___NAME[]      =      "SLC";
char  ___AUTHOR[]    =      "Jun";

#define VersionMajor         0
#define VersionMinor         9

#define VersionString       "0.9.5" /* 不可超过32个字节 */

INT16U get_version(void)
{
    INT16U ver = 0;

    ver |=(INT16U)VersionMajor&0xFF;
    ver<<=8L;
    ver |=(INT16U)VersionMinor&0xFF;

    return ver;
}

STRING get_version_string(void)
{
    return VersionString;
}

/*
**      下面为版本记录
*-------------------------------------------------------------------------------------
*
*  从2011-1-24日开始创建, 到2011-4-25日完成, 共历时三个月
*
*  1) 2012-8-19     开始继续开发, 加入注册机制
*  2) 2012-9-02     注册机制基本完成, 现可以使用, 将来可继续完善.
*  3) 2012-10-18    为韩超, 谢辉彬现场测试而修改的版本
*
*
*
*-------------------------------------------------------------------------------------
** 
*/


/*====================================================================================
 * 
 * 本文件结束: slc/version.c
 * 
**==================================================================================*/


