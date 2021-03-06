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

#define RS_EXIT_KEY 0xE010

/*-----------------------------------------------------------------------------------------
 * 
 *      函数声明
 * 
**---------------------------------------------------------------------------------------*/
void init_register_dialog(void);
void check_register_dialog(int x, int y, int need_quit);


#endif /* #ifndef SLC_REGISTER_H */

/*=========================================================================================
 * 
 * 本文件结束: slc/slc_reg.h
 * 
**=======================================================================================*/


