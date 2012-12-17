/**************************************************************************************
 * �ļ�:    slc/gui.c
 *
 * ˵��:    SLCͼ���û�����
 *
 * ����:    Jun
 *
 * ʱ��:    2011-3-13
**************************************************************************************/
#define  SLC_GUI_C
#include <FamesOS.h>
#include "common.h"

gui_widget * about_system_root;
gui_widget * sys_monitor;

extern gui_widget * welcome_root;

void slc_init_gui(void)
{
    early_loads();

    about_system_root = gui_create_widget(GUI_WIDGET_FORM, 6, 6, 1012, 756, 3, 0, 0, FORM_STYLE_TRANSPARENT|FORM_STYLE_NO_BORDER);  
    if(!about_system_root)return;
    gui_form_init_private(about_system_root, 32);
    gui_form_set_caption(about_system_root, "About System");
    
    sys_monitor = gui_create_widget(GUI_WIDGET_SYS_MNTR, 100, 86, 1, 1, COLOR_YELLOW, CLRSCR_COLOR, 0, SYS_MNTR_STYLE_NO_BORDER);
    if(!sys_monitor)return;
    gui_sys_mntr_init_private(sys_monitor);
        
    gui_widget_link(about_system_root, sys_monitor);

    #if 0
    init_welcome_screen();
    #endif
    #if 0
    //FIXME: ��Ϊ�ڴ治����, ��ʱ�Ȳ�Ҫ��ʾ��ɫ��
    show_palette_init();
    #endif
    init_main_screen();
    init_send_screen();      /* �������ͻ��� */

    StartGUI();
    return;
}


/*=====================================================================================
 * 
 * ���ļ�����: slc/gui.c
 * 
**===================================================================================*/

