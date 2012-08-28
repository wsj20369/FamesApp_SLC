/*************************************************************************************
 * 文件:    slc/test/type.h
 *
 * 说明:    SLC测试
 *
 * 作者:    Jun
 *
 * 时间:    2010-12-17
*************************************************************************************/
#ifndef FAMES_SLC_TEST_TYPE_H
#define FAMES_SLC_TEST_TYPE_H

/*------------------------------------------------------------------------------------
 * 
 *      当需要与FamesOS一起编译时, 将USE_FAMESOS_TYPE定义为1
 *
 *      否则定义为0
 * 
**----------------------------------------------------------------------------------*/

#define USE_FAMESOS_TYPE 1       

#if USE_FAMESOS_TYPE == 1

#include <FamesOS.h>

#else

#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <assert.h>
#include <alloc.h>
#include <string.h>

typedef unsigned long INT32U;
typedef unsigned int  INT16U;
typedef int           BOOL;
typedef int           __int;
typedef char          INT08S;
typedef unsigned char INT08U;

#define ok    1
#define fail  0

#define __far far

#define lock_kernel() 
#define unlock_kernel();

#define FamesAssert(x) assert(x)

#define allocate_buffer(name, type, nbytes, statement_on_failure)  \
            do{ name = (type)farmalloc((nbytes));                  \
                if(name == NULL){ statement_on_failure; }          \
            } while(0) 

#define MEMSET(a,b,c)  memset(a,b,c)

#define in_atomic()  disable()
#define out_atomic() enable();
#define prepare_atomic()

#endif /* USE_FAMESOS_TYPE */

#endif /* #ifndef FAMES_SLC_TEST_TYPE_H */

/*====================================================================================
 * 
 * 本文件结束: slc/test/type.h
 * 
**==================================================================================*/

