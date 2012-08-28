/******************************************************************************************
 * 文件:    slc/slc_para.h
 *
 * 描述:    分压机参数设置
 *
 * 作者:    Jun
******************************************************************************************/
#ifndef SLC_PARAMETER_H
#define SLC_PARAMETER_H


/*-----------------------------------------------------------------------------------------
 * 
 *      函数声明
 * 
**---------------------------------------------------------------------------------------*/
gui_widget * init_slc_param_screen(void);
void enter_slc_param_setup(int machine);

gui_widget * init_slc_global_param_dlg(void);
BOOL setup_slc_global_param(void);


#endif /* #ifndef SLC_PARAMETER_H */

/*=========================================================================================
 * 
 * 本文件结束: slc/slc_para.h
 * 
**=======================================================================================*/


