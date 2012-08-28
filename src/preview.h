/******************************************************************************************
 * �ļ�:    slc/preview.h
 *
 * ����:    ����Ԥ���ؼ�
 *
 * ����:    Jun
******************************************************************************************/
#ifndef SLC_PREVIEW_H
#define SLC_PREVIEW_H


/*-----------------------------------------------------------------------------------------
 * 
 *      PREVIEW���
 * 
**---------------------------------------------------------------------------------------*/
#define PREVIEW_STYLE_BORDER           0x0001       /* �߿�         */
#define PREVIEW_STYLE_SUBSIDE          0x0002       /* �³�         */
#define PREVIEW_STYLE_CLIENT_BDR       0x0004       /* CLIENT�߿�   */
#define PREVIEW_STYLE_MODAL_FRAME      0x0008       /* ģʽ��       */

#define PREVIEW_PAPER_COLOR            20           /* Ԥ��Ĭ����ɫ */

/*-----------------------------------------------------------------------------------------
 * 
 *      ��������
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
 * ���ļ�����: slc/preview.h
 * 
**=======================================================================================*/


