/******************************************************************************************
 * �ļ�:    slc/slc_whet.c
 *
 * ����:    ĥ������
 *
 * ����:    Jun
******************************************************************************************/
#define  SLC_WHET_C
#include <FamesOS.h>
#include "common.h"


extern struct slc_config_s config;

/*-----------------------------------------------------------------------------------------
 * 
 *      ����ĥ����SLC���
 * 
**---------------------------------------------------------------------------------------*/
static int __current_slc_to_whet = 0; /* 0/1 */


/*-----------------------------------------------------------------------------------------
 *          
 *      ĥ�������еĿؼ�����������
 * 
**---------------------------------------------------------------------------------------*/
static gui_widget * whet_screen  = NULL;          /* ĥ����������ؼ�      */
static gui_widget * status_bar   = NULL;          /* ĥ�������״̬��      */
static gui_widget * knife_view   = NULL;          /* ĥ��״̬����ʾ        */

#define nr_buttons 6                              /* ĥ�������еİ�ť����  */
static gui_widget * buttons[nr_buttons];          /* ĥ�������еİ�ť      */
static char * buttons_caption_zh[nr_buttons] = {  /* ��ť����              */
    "F1 ȷ���趨����", "F2 ȷ��ĥ������", "F3 ����ۼ�����",
    "F4 ȷ�ϸ�������", "F5 ȷ�ϸ���ʱ��", "F6 ���Ϳ����л�"
};
static char * buttons_caption_en[nr_buttons] = {  /* ��ť����              */
    "F1 Confirm Set",  "F2 Confirm Whet", "F3 Clear Total",
    "F4 Confirm Oil",  "F5 Cfm Oil Time", "F6 Oil On/Off"
};
static char ** buttons_caption = NULL;
 
extern BMPINFO  icon;                             /* ͼ��                  */


BOOL init_knife_view_private(int knives, int first_call);
/*-----------------------------------------------------------------------------------------
 *          
 *      ĥ������Ķ���(���ʼ��)
 * 
**---------------------------------------------------------------------------------------*/
gui_widget * init_whet_screen(void)
{
    int i, x, y, width, height;

    x = 78;
    y = 196;
    width = 832;
    height = 426;
    
    /* ������   */
    whet_screen = gui_create_widget(GUI_WIDGET_FORM, x, y, width, height, 0, 0, 1, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE);
    if(!whet_screen)
        goto some_error;
    gui_form_init_private(whet_screen, 128);
    gui_form_set_icon(whet_screen, &icon);
    gui_form_set_caption(whet_screen, "");
    
    /* ������   */
    status_bar = gui_create_widget(GUI_WIDGET_LABEL, 5, (height-35), (width-11), 30, 0, 0, 1, LABEL_STYLE_CLIENT_BDR);
    if(!status_bar)
        goto some_error;
    gui_widget_link(whet_screen, status_bar);
    gui_label_init_private(status_bar, 100);
    gui_label_set_text(status_bar, "");

    /* ĥ������ */
    #define __knife_view_widget_width  790 /*(width-20)*/
    knife_view = gui_create_widget(GUI_WIDGET_VIEW, 21, 41, __knife_view_widget_width, 241, 0, 0, 1, 0xB0);
    if(!knife_view)
        goto some_error;
    gui_widget_link(whet_screen, knife_view);
    if(!init_knife_view_private(SLC_K_MAX_NR, 1))
        goto some_error;

    /* ���ܰ�ť */
    x = 73; y = (height-134);
    width = 180; height = 42;
    buttons_caption = pick_string(buttons_caption_zh, buttons_caption_en);
    for(i=0; i<nr_buttons; i++){
        buttons[i] = gui_create_widget(GUI_WIDGET_BUTTON, x, y, width, height, 0, 0, 1, BUTTON_STYLE_CLIENT_BDR);
        if(!buttons[i])
            goto some_error;
        gui_widget_link(whet_screen, buttons[i]);
        gui_button_init_private(buttons[i], 32);
        gui_button_set_caption(buttons[i], buttons_caption[i]);
        x += (width+70);
        if(i == 2){
            x = 73;
            y += 48;
        }
    }

    return whet_screen;

some_error:
    sys_print("init_whet_screen(): failed to create widgets for slc monitor screen!\n");
    ExitApplication();
    return NULL;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      ��ʾĥ�����ݵ���ͼ����
 * 
**---------------------------------------------------------------------------------------*/
static int knife_view_get_max(void)
{
    return (6-1);
}

static BOOL is_writable(int index, int field_id, INT16U option)
{
    option = option;

    if(index == 0 || index == 3)
        return fail;
    if(field_id < 0)
        return fail;
   
    return  ok;
}

static char * __whet_captions_zh[] = { 
    "��������", "�趨����", "ĥ������", "�ۼ�����",
    "��������", "����ʱ��"
};
static char * __whet_captions_en[] = { 
    "Run", "Set", "Whet", "Total",
    "Oil", "Oil Time"
};
static char ** __whet_captions = __whet_captions_zh;

static BOOL get_item(int index, int field_id, char * buf, int buf_len, INT16U option)
{
    INT16U chg_flag;
    BOOL retval;
    slc_descriptor_t * slc;
    
    FamesAssert(buf);
    if(!buf)
        return fail;

    option  = option;
    buf_len = buf_len;  /* ���ﲻ��, ����Ϊ������Ա�֤���ᳬ��������� */

    if(index > knife_view_get_max())
        return fail;

    slc = &(config.slc[__current_slc_to_whet]);

    chg_flag = CHG_OPT_END|CHG_OPT_DEC;

    retval = ok;

    if(field_id < 0){
        __whet_captions = pick_string(__whet_captions_zh, __whet_captions_en);
        STRCPY(buf, __whet_captions[index]);
    } else {
        switch(index){
            case 0: /* �������� */
                INT16toSTR(buf, slc->k_whet.whet_run[field_id], chg_flag);
                break;
            case 1: /* �趨���� */
                INT16toSTR(buf, slc->k_whet.whet_set[field_id], chg_flag);
                break;
            case 2: /* ĥ������ */
                INT16toSTR(buf, slc->k_whet.whet_wht[field_id], chg_flag);
                break;
            case 3: /* �ۼ����� */
                INT32toSTR(buf, slc->k_whet.whet_acc[field_id], chg_flag);
                break;
            case 4: /* �������� */
                INT16toSTR(buf, slc->k_whet.oil_leng[field_id], chg_flag);
                break;
            case 5: /* ����ʱ�� */
                INT16toSTR(buf, slc->k_whet.oil_time[field_id], chg_flag);
                break;
            default:
                break;
        }
    }
    
    return  retval;
}

static BOOL set_item(int index, int field_id, char * buf, int buf_len, KEYCODE key, INT16U option)
{
    INT16U chg_flag;
    int  temp;
    slc_descriptor_t * slc;
    
    FamesAssert(buf);
    if(!buf)
        return fail;

    option  = option;
    buf_len = buf_len;  /* ���ﲻ��, ����Ϊ������Ա�֤���ᳬ��������� */

    if(index > knife_view_get_max())
        return fail;

    slc = &(config.slc[__current_slc_to_whet]);

    chg_flag = CHG_OPT_DEC;
    temp = STRtoINT16(buf, chg_flag);
    if(temp < 0)
        temp = 0;

    if(field_id >= 0){
        switch(index){
            case 0: /* �������� */
                break;
            case 1: /* �趨���� */
                slc->k_whet.whet_set[field_id] = temp;
                break;
            case 2: /* ĥ������ */
                slc->k_whet.whet_wht[field_id] = temp;
                break;
            case 3: /* �ۼ����� */
                break;
            case 4: /* �������� */
                slc->k_whet.oil_leng[field_id] = temp;
                break;
            case 5: /* ����ʱ�� */
                slc->k_whet.oil_time[field_id] = temp;
                break;
            default:
                key = key;
                break;
        }
    }
    
    return  ok;
}

BOOL init_knife_view_private(int knives, int first_call)
{
    static view_fields_t * knife_fields = NULL;
    static char (* knife_captions)[8] = NULL;
    int  i, fields, width;


    FamesAssert(knives > 2);
    if(knives <= 2)
        return fail;

    if(!knife_fields){
        knife_fields = mem_alloc(sizeof(view_fields_t) * (SLC_K_MAX_NR+3));
    }
    if(!knife_captions){
        knife_captions = mem_alloc(8 * SLC_K_MAX_NR);
    }
    if(!knife_fields || !knife_captions)
        return fail;

    fields = knives;

    width = (__knife_view_widget_width-88)/fields;
    width -= 8;
    width /= 8;

    knife_fields[0].caption = pick_string("==��==", "== K ==");
    knife_fields[0].id = -1;
    knife_fields[0].bytes = 9;
    knife_fields[0].bytes_for_width = 9;
    knife_fields[0].style = 0;
    knife_fields[0].draw_style = DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER;
    knife_fields[0].comment = "";
    for(i=1; i<=knives; i++){
        sprintf(knife_captions[i-1], pick_string("��%d", "K%d"), i);
        knife_fields[i].caption = knife_captions[i-1];
        knife_fields[i].id = i-1;
        knife_fields[i].bytes = 8;
        knife_fields[i].bytes_for_width = width;
        knife_fields[i].style = 0;
        knife_fields[i].draw_style = DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER;
        knife_fields[i].comment = "";
    }
    knife_fields[i].caption = NULL;
    
    gui_view_init_private( knife_view,
                           knife_fields,
                           knife_view_get_max,
                           get_item,
                           set_item,
                           is_writable,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           COLOR_WHITE,
                           17,
                           34,
                           first_call
                         );
    gui_view_set_dashed(knife_view, COLOR_YELLOW, 0);
    return ok;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      ĥ�������µĲ���
 * 
**---------------------------------------------------------------------------------------*/
void setup_whet(int machine)
{
    KEYCODE key;
    slc_descriptor_t * slc;
    char ___s[64];
    int i, slc_index, oil_on;
    INT32S tmp32[SLC_K_MAX_NR];
    extern PLC * slc_plc[];

    FamesAssert(machine == 1 || machine == 2);

    slc_index = machine;

    machine--;

    if(!(machine == 0 || machine == 1))
        return;

    lock_kernel();
    __current_slc_to_whet = machine;
    unlock_kernel();

    slc = &(config.slc[machine]);

    if(!whet_screen)
        return;
    
    if(!slc_plc[machine])
        return;

    plc_action_id_set_number(slc_plc[machine], id_whet_run, slc->k_number);
    plc_action_id_set_number(slc_plc[machine], id_whet_acc, slc->k_number);
    plc_action_enable_id(slc_plc[machine], id_whet_run);
    plc_action_enable_id(slc_plc[machine], id_whet_acc);
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_R, PLC_ADDR_WHET_SET, slc->k_whet.whet_set, slc->k_number);
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_R, PLC_ADDR_WHET_WHT, slc->k_whet.whet_wht, slc->k_number);
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_R, PLC_ADDR_OIL_LENG, slc->k_whet.oil_leng, slc->k_number);
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_R, PLC_ADDR_OIL_TIME, slc->k_whet.oil_time, slc->k_number);
    oil_on = 0;
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_M, PLC_ADDR_OIL_ON,   &oil_on, 1);
    if(oil_on){
        gui_set_widget_bkcolor(buttons[5], COLOR_GREEN);
    } else {
        gui_set_widget_bkcolor(buttons[5], 0);
    }

    sprintf(___s, pick_string("ĥ���趨 - ��%d", "Whet Setup - M%d"), (machine+1));
    gui_form_set_caption(whet_screen, ___s);
    init_knife_view_private(slc->k_number, 0);
    gui_view_goto_top(knife_view);
    gui_show_widget(whet_screen);

    for(;;){
        key = gui_view_editing(knife_view, 0);
        gui_label_set_text(status_bar, "");
        switch(key){
            case ESC:
                plc_action_disable_id(slc_plc[machine], id_whet_run);
                plc_action_disable_id(slc_plc[machine], id_whet_acc);
                gui_hide_widget(whet_screen);
                return;
            case UP:
                gui_view_move_up(knife_view);
                break;
            case DOWN:
                gui_view_move_down(knife_view);
                break;
            case F1:  /* ȷ���趨���� */
                gui_set_widget_bkcolor(buttons[0], COLOR_RED);
                ___slc_plc_rw(slc_index, FATEK_PLC_WRITE_R, PLC_ADDR_WHET_SET, slc->k_whet.whet_set, slc->k_number);
                TaskDelay(100L);
                gui_set_widget_bkcolor(buttons[0], 0);
                gui_label_set_text(status_bar, pick_string("ȷ���趨���� - ���", "Confirm Set - Done"));
                break;
            case F2:  /* ȷ��ĥ������ */
                gui_set_widget_bkcolor(buttons[1], COLOR_RED);
                ___slc_plc_rw(slc_index, FATEK_PLC_WRITE_R, PLC_ADDR_WHET_WHT, slc->k_whet.whet_wht, slc->k_number);
                TaskDelay(100L);
                gui_set_widget_bkcolor(buttons[1], 0);
                gui_label_set_text(status_bar, pick_string("ȷ��ĥ������ - ���", "Confirm Whet - Done"));
                break;
            case F3:  /* ����ۼ����� */
                gui_set_widget_bkcolor(buttons[2], COLOR_RED);
                for(i=0; i<slc->k_number; i++)
                    tmp32[i] = 0L;
                ___slc_plc_rw(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_WHET_ACC, tmp32, slc->k_number);
                TaskDelay(100L);
                gui_set_widget_bkcolor(buttons[2], 0);
                gui_label_set_text(status_bar, pick_string("����ۼ����� - ���", "Clear Total - Done"));
                break;
            case F4:  /* ȷ�ϸ������� */
                gui_set_widget_bkcolor(buttons[3], COLOR_RED);
                ___slc_plc_rw(slc_index, FATEK_PLC_WRITE_R, PLC_ADDR_OIL_LENG, slc->k_whet.oil_leng, slc->k_number);
                TaskDelay(100L);
                gui_set_widget_bkcolor(buttons[3], 0);
                gui_label_set_text(status_bar, pick_string("ȷ�ϸ������� - ���", "Confirm Oil - Done"));
                break;
            case F5:  /* ȷ�ϸ���ʱ�� */
                gui_set_widget_bkcolor(buttons[4], COLOR_RED);
                ___slc_plc_rw(slc_index, FATEK_PLC_WRITE_R, PLC_ADDR_OIL_TIME, slc->k_whet.oil_time, slc->k_number);
                TaskDelay(100L);
                gui_set_widget_bkcolor(buttons[4], 0);
                gui_label_set_text(status_bar, pick_string("ȷ�ϸ���ʱ�� - ���", "Confirm Oil Time - Done"));
                break;
            case F6:  /* ���Ϳ����л� */
                oil_on = !oil_on;
                ___slc_plc_rw(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_OIL_ON, &oil_on, 1);
                ___slc_plc_rw(slc_index, FATEK_PLC_READ_M,  PLC_ADDR_OIL_ON, &oil_on, 1);
                if(oil_on){
                    gui_set_widget_bkcolor(buttons[5], COLOR_GREEN);
                } else {
                    gui_set_widget_bkcolor(buttons[5], 0);
                }
                gui_label_set_text(status_bar, pick_string("���Ϳ����л� - ���", "Oil Switch - Done"));
                break;
            default:
                break;
        }
    }
}


/*=========================================================================================
 * 
 * ���ļ�����: slc/slc_whet.c
 * 
**=======================================================================================*/


