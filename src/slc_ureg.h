/******************************************************************************************
 * 文件:    slc/slc_reg.h
 *
 * 描述:    注册对话框
 *
 * 作者:    Jun
******************************************************************************************/
#ifndef SLC_REGISTER_H
#define SLC_REGISTER_H

/*-----------------------------------------------------------------------------------------
 * 
 *      对话框指针
 * 
**---------------------------------------------------------------------------------------*/
#ifndef SLC_REGISTER_C

extern gui_widget * const register_dialog;

#endif

#define ID_MAX_LEN    32
#define SN_MAX_LEN    32

/*-----------------------------------------------------------------------------------------
 * 
 *      函数声明
 * 
**---------------------------------------------------------------------------------------*/
void init_register_dialog(void);
void check_register_dialog(int x, int y);


#endif /* #ifndef SLC_REGISTER_H */

/*=========================================================================================
 * 
 * 本文件结束: slc/slc_reg.h
 * 
**=======================================================================================*/


