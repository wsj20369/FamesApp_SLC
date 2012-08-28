/******************************************************************************************
 * 文件:    slc/preview.h
 *
 * 描述:    订单预览控件
 *
 * 作者:    Jun
******************************************************************************************/
#ifndef SLC_PREVIEW_H
#define SLC_PREVIEW_H


/*-----------------------------------------------------------------------------------------
 * 
 *      PREVIEW风格
 * 
**---------------------------------------------------------------------------------------*/
#define PREVIEW_STYLE_BORDER           0x0001       /* 边框         */
#define PREVIEW_STYLE_SUBSIDE          0x0002       /* 下沉         */
#define PREVIEW_STYLE_CLIENT_BDR       0x0004       /* CLIENT边框   */
#define PREVIEW_STYLE_MODAL_FRAME      0x0008       /* 模式框       */

#define PREVIEW_PAPER_COLOR            20           /* 预览默认颜色 */

/*-----------------------------------------------------------------------------------------
 * 
 *      函数声明
 * 
**---------------------------------------------------------------------------------------*/
BOOL guical preview_init_private(gui_widget * preview);
BOOL guical preview_set_order(gui_widget * preview, order_struct * order);
BOOL guical preview_set_param(gui_widget * preview,
                              int   ruler_font, COLOR ruler_color,
                              int   ruler_unit_font, COLOR ruler_unit_color,
                              int   order_info_font, COLOR order_info_color,
                              COLOR preview_paper_color,
                              int   preview_marker_font, COLOR preview_marker_color
                             );
int         preview_initialize(void);



#endif /* #ifndef SLC_PREVIEW_H */

/*=========================================================================================
 * 
 * 本文件结束: slc/preview.h
 * 
**=======================================================================================*/


