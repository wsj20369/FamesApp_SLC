/******************************************************************************************
 * �ļ�:    slc/slc_malf.c
 *
 * ����:    SLC���ϼ��(malfunction)
 *
 * ����:    Jun
 *
 * ʱ��:    2011-4-18
******************************************************************************************/
#define  SLC_MALFUNCTION_C
#include <includes.h>
#include "common.h"


/*-----------------------------------------------------------------------------------------
 *          
 *      �ؼ�����������
 * 
**---------------------------------------------------------------------------------------*/
static gui_widget * main_form         = NULL;     /* ���ϻ�������ؼ�       */
static gui_widget * status_bar        = NULL;     /* ״̬��                 */
static gui_widget * malfunction_view  = NULL;     /* ��ʾ���ϵ�VIEW�ؼ�     */

#define nr_buttons 2                              /* ��ť����               */
static gui_widget * buttons[nr_buttons];          /* ��ť                   */
static char * buttons_caption_zh[nr_buttons] = {  /* ��ť����               */
    "F1 ��1", "F2 ��2"
};
static char * buttons_caption_en[nr_buttons] = {  /* ��ť����               */
    "F1 SLC-1", "F2 SLC-2"
};
static char ** buttons_caption = NULL;

extern BMPINFO  icon;                             /* ͼ��                   */

static int   current_slc_to_mntr = 0;

extern INT32S slc_malfunction[];                  /* SLC����λͼ            */

BOOL init_malfunction_view_private(void);
/*-----------------------------------------------------------------------------------------
 *          
 *      ����Ķ���(���ʼ��)
 * 
**---------------------------------------------------------------------------------------*/
gui_widget * init_malfunction_screen(void)
{
    int i, x, y, width, height;

    x = 97;
    y = 202;
    width = 766;
    height = 390;
    
    /* ������   */
    main_form = gui_create_widget(GUI_WIDGET_FORM, x, y, width, height, 0, 0, 1, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE);
    if(!main_form)
        goto some_error;
    gui_form_init_private(main_form, 128);
    gui_form_set_icon(main_form, &icon);
    gui_form_set_caption(main_form, pick_string("SLC���ϼ��", "SLC Malfunctions View"));
    
    /* ������   */
    status_bar = gui_create_widget(GUI_WIDGET_LABEL, 5, (height-35), (width-11), 30, 0, 0, 1, LABEL_STYLE_CLIENT_BDR);
    if(!status_bar)
        goto some_error;
    gui_widget_link(main_form, status_bar);
    gui_label_init_private(status_bar, 100);
    gui_label_set_text(status_bar, pick_string("-���ϼ��-", "-Malfunction-"));


    /* ������� */
    #define ___view_widget_width  722 /*(width-20)*/
    malfunction_view = gui_create_widget(GUI_WIDGET_VIEW, 23, 41, ___view_widget_width, (height-135), 0, 0, 1, VIEW_STYLE_FIELDS_TITLE|VIEW_STYLE_NONE_FIRST);
    if(!malfunction_view)
        goto some_error;
    gui_widget_link(main_form, malfunction_view);
    if(!init_malfunction_view_private())
        goto some_error;

    /* ���ܰ�ť */
    x = 160; y = (height-85);
    width = 160; height = 42;
    buttons_caption = pick_string(buttons_caption_zh, buttons_caption_en);
    for(i=0; i<nr_buttons; i++){
        buttons[i] = gui_create_widget(GUI_WIDGET_BUTTON, x, y, width, height, 0, 0, 1, BUTTON_STYLE_CLIENT_BDR);
        if(!buttons[i])
            goto some_error;
        gui_widget_link(main_form, buttons[i]);
        gui_button_init_private(buttons[i], 32);
        gui_button_set_caption(buttons[i], buttons_caption[i]);
        x += (width+120);
    }

    return main_form;

some_error:
    sys_print("init_malfunction_screen(): failed to create widgets!\n");
    ExitApplication();
    return NULL;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      ��ʾ���ϵ�VIEW�ؼ�
 * 
**---------------------------------------------------------------------------------------*/
enum slc_malfunction_view_id {
    __id_sn,
    __id_code,
    __id_comment,
};

static view_fields_t slc_malfunction_view_fields[] =
{
#define ____style        0
#define ____draw_style   DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER
    { "���",     __id_sn,       4,   6,  ____style,  ____draw_style,  "", },
    { "����",     __id_code,     5,   7,  ____style,  ____draw_style,  "", },
    { "== ˵ �� ==",   
                  __id_comment, 72,  72,  ____style,  DRAW_OPT_FIL_BG, "", },
    { NULL, }
#undef ____style
#undef ____draw_style
};
    
static int get_max(void)
{
    #if 1
    INT32U malf_code;
    int i, cntr;
    char ___s[32];
    lock_kernel();
    malf_code = (INT32U)slc_malfunction[current_slc_to_mntr];
    unlock_kernel();
    for(i=0, cntr=0; i<32; i++){
        if(malf_code & 0x1L)
            cntr++;
        malf_code >>= 1;
    }
    sprintf(___s, pick_string("��%d����: %d ��", "M%d Malfunctions: %d"), (current_slc_to_mntr+1), cntr);
    gui_label_set_text(status_bar, ___s);
    #endif

    return 31; /* ��32�ֹ��� */
}

static BOOL get_item(int index, int field_id, char * buf, int buf_len, INT16U option)
{
    INT16U chg_flag;
    INT32U malf_code;
    int i, j;
    
    FamesAssert(buf);
    if(!buf)
        return fail;

    option  = option;

    if(index > get_max())
        return fail;

    chg_flag = CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_SIG;

    lock_kernel();
    malf_code = (INT32U)slc_malfunction[current_slc_to_mntr];
    unlock_kernel();

    for(i=0, j=index; i<=get_max(); i++){
        if(malf_code & 0x1L){
            if(j == 0)
                break;
            j--;
        }
        malf_code >>= 1;
    }

    if(i <= get_max()){
        switch(field_id){
            case __id_sn:
                INT16toSTR(buf, (index+1), chg_flag);
                break;
            case __id_code:
                INT16toSTR(buf, (i+1), chg_flag);
                break;
            case __id_comment:
                slc_malfunction_get_comment(buf, buf_len, (i+1));
                break;
            default:
                break;
        }
    } else {
        STRCPY(buf, "");
    }

    return  ok;
}

static BOOL init_malfunction_view_private(void)
{
    slc_malfunction_view_fields[0].caption = pick_string("���", "S/N");
    slc_malfunction_view_fields[1].caption = pick_string("����", "Code");
    slc_malfunction_view_fields[2].caption = pick_string("== ˵ �� ==", "== Description ==");
    
    gui_view_init_private( malfunction_view, 
                           slc_malfunction_view_fields, 
                           get_max, 
                           get_item, 
                           NULL, 
                           NULL,
                           NULL, 
                           "", 
                           NULL,
                           NULL,
                           COLOR_WHITE, 
                           17,
                           28,
                           1
                         );
    return ok;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      �鿴���й���
 * 
**---------------------------------------------------------------------------------------*/
void slc_malfunction_monitor(void)
{
    KEYCODE key;

    #define ____bkcolor  144

    if(!malfunction_view)
        return;

    switch(current_slc_to_mntr){
        case 0:
            gui_set_widget_color(buttons[0], COLOR_WHITE);
            gui_set_widget_bkcolor(buttons[0], ____bkcolor);
            gui_set_widget_color(buttons[1], 0);
            gui_set_widget_bkcolor(buttons[1], 0);
            break;
        case 1:
            gui_set_widget_color(buttons[0], 0);
            gui_set_widget_bkcolor(buttons[0], 0);
            gui_set_widget_color(buttons[1], COLOR_WHITE);
            gui_set_widget_bkcolor(buttons[1], ____bkcolor);
            break;
        default:
            FamesAssert(!"current_slc_to_mntr has a invalid value!");
            break;
    };
    for(;;){
        key = waitkey(0L);
        switch(key){
            case ESC:
                return;
            case F1:
                current_slc_to_mntr = 0;
                gui_set_widget_color(buttons[0], COLOR_WHITE);
                gui_set_widget_bkcolor(buttons[0], ____bkcolor);
                gui_set_widget_color(buttons[1], 0);
                gui_set_widget_bkcolor(buttons[1], 0);
                break;
            case F2:
                current_slc_to_mntr = 1;
                gui_set_widget_color(buttons[0], 0);
                gui_set_widget_bkcolor(buttons[0], 0);
                gui_set_widget_color(buttons[1], COLOR_WHITE);
                gui_set_widget_bkcolor(buttons[1], ____bkcolor);
                break;
            case UP:
                gui_view_move_up(malfunction_view);
                break;
            case DOWN:
                gui_view_move_down(malfunction_view);
                break;
            case PGUP:
                gui_view_page_up(malfunction_view);
                break;
            case PGDN:
                gui_view_page_down(malfunction_view);
                break;
            case CTRL_HOME:
                gui_view_goto_top(malfunction_view);
                break;
            case CTRL_END:
                gui_view_goto_bottom(malfunction_view);
                break;
            default:
                break;
        }
    }
}

/*-----------------------------------------------------------------------------------------
 *          
 *      ���ع���˵���ַ���
 * 
**---------------------------------------------------------------------------------------*/
static char __comments[32][32]; /* ����˵�� */

void slc_malfunction_get_comment(char * buf, int buf_len, int malf_code)
{
    static int first = 1;

    lock_kernel();
    if(first){
        int i;
        char ___s[32];
        first = 0;
        for(i=0; i<32; i++){
            STRCPY(__comments[i], pick_string("δ֪����!", "Unknown!"));
            sprintf(___s, "fault_%d", (i+1));
            load_string(__comments[i], sizeof(__comments[i]), ___s);
        }
    }
    unlock_kernel();
    
    FamesAssert(buf);
    FamesAssert(malf_code > 0 && malf_code <= 32)
    if(!buf || !(malf_code > 0 && malf_code <= 32))
        return;

    buf_len = buf_len;

    sprintf(buf, __comments[malf_code-1]);
}


/*=========================================================================================
 * 
 * ���ļ�����: slc/slc_malf.c
 * 
**=======================================================================================*/


