/******************************************************************************************
 * 文件:    plc_test.c
 *
 * 描述:    PLC暂存器读写及I/O点监控
 *
 * 作者:    Jun
 *
 * 时间:    2011-4-12
******************************************************************************************/
#define  SLC_PLC_TEST_C
#include <includes.h>
#include "common.h"

/*-----------------------------------------------------------------------------------------
 *
 *    下面为PLC监控部分
 *
**---------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
 *          
 *      控件及其它定义
 * 
**---------------------------------------------------------------------------------------*/
static gui_widget * plc_screen      = NULL;         /* PLC监控画面的主控件    */
static gui_widget * plc_status_bar  = NULL;         /* 状态条                 */
static gui_widget * plc_view        = NULL;         /* PLC监控的VIEW控件      */

#define nr_buttons 4                                /* 按钮个数               */
static gui_widget * buttons[nr_buttons];            /* 按钮                   */
static char * buttons_caption_zh[nr_buttons] = {    /* 按钮标题               */
    "F1 机1", "F2 机2", "F10 确认输入", "ESC 返回"
};
static char * buttons_caption_en[nr_buttons] = {    /* 按钮标题               */
    "F1 SLC-1", "F2 SLC-2", "F10 Confirm", "ESC Return"
};
static char ** buttons_caption = NULL;

extern BMPINFO  icon;                               /* 图标                   */


#define  PLC_ITEM_NR  24                            /* 同时可监控24个单元     */
PLC_ACTION  * plc_action[2] = { NULL, NULL };
INT08S        plc_addr[2][PLC_ITEM_NR][8];
int           plc_value[2][PLC_ITEM_NR] = { {0, }, {0, } };
int           plc_input[2][PLC_ITEM_NR] = { {0, }, {0, } };
static  int   current_plc_to_mntr = 0;

extern  PLC * slc_plc[];

BOOL init_plc_view_private(void);
/*-----------------------------------------------------------------------------------------
 *          
 *      画面的定义(或初始化)
 * 
**---------------------------------------------------------------------------------------*/
gui_widget * init_plc_monitor_screen(void)
{
    int i, x, y, width, height;
    int size;
    BOOL retval;

    /* 
    ** PLC-ACTION 初始化 -----------------------------------------------------------------
    */
    size = PLC_ITEM_NR * sizeof(PLC_ACTION);

    /* 机1 PLC  */
    plc_action[0] = mem_alloc((INT32U)(INT32S)size);
    if(!plc_action[0])
        return NULL;
    MEMSET((INT08S *)plc_action[0], 0, size);
    for(i = 0; i < PLC_ITEM_NR; i++){
        retval = plc_set_action(plc_action[0]+i,
                                0, 
                                FATEK_PLC_READ_R, 
                                plc_addr[0][i], 
                               &plc_value[0][i], 
                                1, 
                                0,
                                NULL, 
                                1);
        if(!retval){
            /* actions on fail */
        }
    }

    /* 机2 PLC  */
    plc_action[1] = mem_alloc((INT32U)(INT32S)size);
    if(!plc_action[1])
        return NULL;
    MEMSET((INT08S *)plc_action[1], 0, size);
    for(i = 0; i < PLC_ITEM_NR; i++){
        retval = plc_set_action(plc_action[1]+i,
                                0, 
                                FATEK_PLC_READ_R, 
                                plc_addr[1][i], 
                               &plc_value[1][i], 
                                1, 
                                0,
                                NULL, 
                                1);
        if(!retval){
            /* actions on fail */
        }
    }

    MEMSET((INT08S *)plc_addr, 0, sizeof(plc_addr)); 

    /* 
    ** 用户界面初始化 --------------------------------------------------------------------
    */
    x = 131;
    y = 206;
    width = 736;
    height = 403;
    
    /* 主界面   */
    plc_screen = gui_create_widget(GUI_WIDGET_FORM, x, y, width, height, 0, 0, 1, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE);
    if(!plc_screen)
        goto some_error;
    gui_form_init_private(plc_screen, 128);
    gui_form_set_icon(plc_screen, &icon);
    gui_form_set_caption(plc_screen, pick_string("PLC暂存器监控", "PLC Monitor"));
    
    /* 工具条   */
    plc_status_bar = gui_create_widget(GUI_WIDGET_LABEL, 5, (height-35), (width-11), 30, 0, 0, 1, LABEL_STYLE_CLIENT_BDR);
    if(!plc_status_bar)
        goto some_error;
    gui_widget_link(plc_screen, plc_status_bar);
    gui_label_init_private(plc_status_bar, 100);
    gui_label_set_text(plc_status_bar, pick_string("监控内容可包括: R值, M单点, X输入点, Y输出点等",
                                                   "Monitor: R, M, X, Y"));


    /* 监控数据 */
    #define ___view_widget_width  683 /*(width-20)*/
    plc_view = gui_create_widget(GUI_WIDGET_VIEW, 25, 41, ___view_widget_width, (height-135), 0, 0, 1, VIEW_STYLE_FIELDS_TITLE|VIEW_STYLE_NONE_FIRST|VIEW_STYLE_NONE_SELECT);
    if(!plc_view)
        goto some_error;
    gui_widget_link(plc_screen, plc_view);
    if(!init_plc_view_private())
        goto some_error;

    /* 功能按钮 */
    x = 43; y = (height-85);
    width = 148; height = 42;
    buttons_caption = pick_string(buttons_caption_zh, buttons_caption_en);
    for(i=0; i<nr_buttons; i++){
        buttons[i] = gui_create_widget(GUI_WIDGET_BUTTON, x, y, width, height, 0, 0, 1, BUTTON_STYLE_CLIENT_BDR);
        if(!buttons[i])
            goto some_error;
        gui_widget_link(plc_screen, buttons[i]);
        gui_button_init_private(buttons[i], 32);
        gui_button_set_caption(buttons[i], buttons_caption[i]);
        x += (width+18);
    }

    return plc_screen;

some_error:
    sys_print("init_plc_monitor_screen(): failed to create widgets!\n");
    ExitApplication();
    return NULL;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      楞别深浅数据的视图对象
 * 
**---------------------------------------------------------------------------------------*/
enum order_view_fields_id {
    __id_addr1 = 0,
    __id_value1,
    __id_input1,
    __id_dummy1,
    
    __id_addr2,
    __id_value2,
    __id_input2,
    __id_dummy2,
    
    __id_addr3,
    __id_value3,
    __id_input3,
    __id_dummy3,
};

static view_fields_t plc_view_fields[] =
{
#define ____style        0
#define ____draw_style   DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER
    { "编号",       __id_addr1,  6,  8,  ____style,  ____draw_style,  "", },
    { "资料",       __id_value1, 6,  8,  ____style,  ____draw_style,  "", },
    { "输入",       __id_input1, 6,  8,  ____style,  ____draw_style,  "", },
    { "",           __id_dummy1, 1,  1,  ____style,  ____draw_style,  "", },
    { "编号",       __id_addr2,  6,  8,  ____style,  ____draw_style,  "", },
    { "资料",       __id_value2, 6,  8,  ____style,  ____draw_style,  "", },
    { "输入",       __id_input2, 6,  8,  ____style,  ____draw_style,  "", },
    { "",           __id_dummy2, 1,  1,  ____style,  ____draw_style,  "", },
    { "编号",       __id_addr3,  6,  8,  ____style,  ____draw_style,  "", },
    { "资料",       __id_value3, 6,  8,  ____style,  ____draw_style,  "", },
    { "输入",       __id_input3, 6,  8,  ____style,  ____draw_style,  "", },
    { NULL, }
#undef ____style
#undef ____draw_style
};

#define ___idx(index, field_id)  ((3 * index) + (field_id / 4))
    
static int get_max(void)
{
    return 7;
}

static BOOL is_writable(int index, int field_id, INT16U option)
{
    index  = index;
    option = option;

    switch(field_id){
        case __id_addr1:
        case __id_addr2:
        case __id_addr3:
        case __id_input1:
        case __id_input2:
        case __id_input3:
            return ok;
        case __id_value1:
        case __id_dummy1:
        case __id_value2:
        case __id_dummy2:
        case __id_value3:
        case __id_dummy3:
            return fail;
        default:
            return fail;
    }
}

static BOOL get_item(int index, int field_id, char * buf, int buf_len, INT16U option)
{
    INT16U chg_flag, idx;
    PLC_ACTION * action;
    
    FamesAssert(buf);
    if(!buf)
        return fail;

    if(!plc_action[current_plc_to_mntr]){
        STRCPY(buf, "");
        return ok;
    }

    option  = option;
    buf_len = buf_len;  /* 这里不用, 是因为这里可以保证不会超过这个长度 */

    if(index > get_max())
        return fail;

    chg_flag = CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_SIG;

    idx = ___idx(index, field_id);
    action = (plc_action[current_plc_to_mntr] + idx);

    if(action->disabled){
        STRCPY(buf, "");
        return ok;
    }

    switch(field_id & 0x3){
        case 0:
            STRCPY(buf, action->addr);
            break;
        case 1:
            INT16toSTR(buf, plc_value[current_plc_to_mntr][idx], chg_flag);
            break;
        case 2:
            #if 1
            INT16toSTR(buf, plc_input[current_plc_to_mntr][idx], chg_flag);
            break;
            #endif
        case 3:
            STRCPY(buf, "");
            break;
        default:
            break;
    }

    return  ok;
}

static BOOL set_item(int index, int field_id, char * buf, int buf_len, KEYCODE key, INT16U option)
{
    INT16U chg_flag;
    int    temp, cmd, idx, plc;
    char   ___s[8];
    PLC_ACTION * action;

    FamesAssert(buf);
    if(!buf)
        return fail;

    option  = option;
    buf_len = buf_len;  /* 这里不用, 是因为这里可以保证不会超过这个长度 */

    if(index > get_max())
        return fail;

    if(ESC == key)
        return ok;

    chg_flag = CHG_OPT_DEC;

    plc = current_plc_to_mntr;

    idx = ___idx(index, field_id);
    action = (plc_action[plc] + idx);

    if((field_id & 3) == 2){   /* 数值栏位 */
        switch(plc_addr[plc][idx][0]){
            case 'r':
            case 'R':
            case 'w':
            case 'W':
                cmd = FATEK_PLC_WRITE_R;
                break;
            case 'm':
            case 'M':
            case 'x':
            case 'X':
            case 'y':
            case 'Y':
                cmd = FATEK_PLC_WRITE_M;
                break;
            default:
                cmd = -1;
                break;
        }
        if(action->disabled || !slc_plc[plc])
            cmd = -1;
        if(cmd > 0 && key == F10){
            temp = STRtoINT16(buf, chg_flag);
            sprintf(___s, plc_addr[plc][idx]);
            gui_set_widget_style(buttons[2], BUTTON_STYLE_CLIENT_BDR|BUTTON_STYLE_PRESSED);
            plc_rw(slc_plc[plc], cmd, ___s, &temp, 1);
            gui_set_widget_style(buttons[2], BUTTON_STYLE_CLIENT_BDR);
            plc_input[plc][idx] = temp;
        }
    } else if((field_id & 3) == 0){   /* 地址栏位 */
        temp = STRtoINT16(&buf[1], chg_flag);
        switch(buf[0]){
            case 'r':
            case 'R':
                cmd = FATEK_PLC_READ_R;
                sprintf(___s, "R%05u", temp);
                break;
            case 'w':
            case 'W':
                cmd  = FATEK_PLC_READ_R;
                temp = STRtoINT16(&buf[2], chg_flag);
                temp &= (~0x7);
                switch(buf[1]){
                    case 'm':
                    case 'M':
                        sprintf(___s, "WM%04d", temp);
                        break;
                    case 'x':
                    case 'X':
                        sprintf(___s, "WX%04d", temp);
                        break;
                    case 'y':
                    case 'Y':
                        sprintf(___s, "WY%04d", temp);
                        break;
                    default:
                        cmd = -1;
                        break;
                }
                break;
            case 'm':
            case 'M':
                cmd = FATEK_PLC_READ_M;
                if(temp > 9999 || temp < 0)
                    temp = 9999;
                sprintf(___s, "M%04d", temp);
                break;
            case 'x':
            case 'X':
                cmd = FATEK_PLC_READ_M;
                if(temp > 9999 || temp < 0)
                    temp = 9999;
                sprintf(___s, "X%04d", temp);
                break;
            case 'y':
            case 'Y':
                cmd = FATEK_PLC_READ_M;
                if(temp > 9999 || temp < 0)
                    temp = 9999;
                sprintf(___s, "Y%04d", temp);
                break;
            default:
                cmd = -1;
                break;
        }
        if(cmd < 0){ /* 若输入了无效的地址, 那么就删除这一项, 参考上面的 "default:" 选项 */
            lock_kernel();
            action->disabled = 1;
            plc_value[plc][idx] = 0;
            plc_input[plc][idx] = 0;
            unlock_kernel();
        } else {
            lock_kernel();
            sprintf(plc_addr[plc][idx], ___s);
            action->cmd = cmd;
            action->disabled = 0;
            unlock_kernel();
        }
    }
    
    return  ok;
}

static BOOL init_plc_view_private(void)
{
    plc_view_fields[0].caption  = pick_string("编号", "Addr");
    plc_view_fields[1].caption  = pick_string("资料", "Value");
    plc_view_fields[2].caption  = pick_string("输入", "Input");
    plc_view_fields[4].caption  = pick_string("编号", "Addr");
    plc_view_fields[5].caption  = pick_string("资料", "Value");
    plc_view_fields[6].caption  = pick_string("输入", "Input");
    plc_view_fields[8].caption  = pick_string("编号", "Addr");
    plc_view_fields[9].caption  = pick_string("资料", "Value");
    plc_view_fields[10].caption = pick_string("输入", "Input");

    gui_view_init_private( plc_view, 
                           plc_view_fields, 
                           get_max, 
                           get_item, 
                           set_item, 
                           is_writable,
                           NULL, 
                           "", 
                           NULL,
                           NULL,
                           COLOR_WHITE, 
                           COLOR_BLACK,
                           29,
                           1
                         );
    return ok;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      楞别深浅画面下的操作
 * 
**---------------------------------------------------------------------------------------*/
void plc_monitor(void)
{
    KEYCODE key;
    int i;

    #define ____bkcolor  144

    if(!plc_view)
        return;

    if(slc_plc[0]){
        for(i = 0; i < PLC_ITEM_NR; i++){
            do_plc_action(slc_plc[0], (plc_action[0]+i), PLC_ACTION_FLAG_LINK);
        }
    }
    if(slc_plc[1]){
        for(i = 0; i < PLC_ITEM_NR; i++){
            do_plc_action(slc_plc[1], (plc_action[1]+i), PLC_ACTION_FLAG_LINK);
        }
    }

    switch(current_plc_to_mntr){
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
            FamesAssert(!"current_plc_to_mntr has a invalid value!");
            break;
    };

    for(;;){
        key = gui_view_editing(plc_view, 0);
        switch(key){
            case ESC:
                goto out;
            case F1:
                current_plc_to_mntr = 0;
                gui_set_widget_color(buttons[0], COLOR_WHITE);
                gui_set_widget_bkcolor(buttons[0], ____bkcolor);
                gui_set_widget_color(buttons[1], 0);
                gui_set_widget_bkcolor(buttons[1], 0);
                /*
                gui_refresh_widget(plc_view);
                */
                break;
            case F2:
                current_plc_to_mntr = 1;
                gui_set_widget_color(buttons[0], 0);
                gui_set_widget_bkcolor(buttons[0], 0);
                gui_set_widget_color(buttons[1], COLOR_WHITE);
                gui_set_widget_bkcolor(buttons[1], ____bkcolor);
                /*
                gui_refresh_widget(plc_view);
                */
                break;
            case UP:
                gui_view_move_up(plc_view);
                break;
            case DOWN:
                gui_view_move_down(plc_view);
                break;
            case PGUP:
                gui_view_page_up(plc_view);
                break;
            case PGDN:
                gui_view_page_down(plc_view);
                break;
            case CTRL_HOME:
                gui_view_goto_top(plc_view);
                break;
            case CTRL_END:
                gui_view_goto_bottom(plc_view);
                break;
        }
    }

out:    
    if(slc_plc[0]){
        for(i = 0; i < PLC_ITEM_NR; i++){
            do_plc_action(slc_plc[0], (plc_action[0]+i), PLC_ACTION_FLAG_UNLK);
        }
    }
    if(slc_plc[1]){
        for(i = 0; i < PLC_ITEM_NR; i++){
            do_plc_action(slc_plc[1], (plc_action[1]+i), PLC_ACTION_FLAG_UNLK);
        }
    }

    return;
}


/*------------------------------------------------------------------------------------
 *
 *    下面为I/O点监控部分
 *
**----------------------------------------------------------------------------------*/
static gui_widget * main_form = NULL;
static gui_widget * group[2];
static gui_widget * x_text[2], * x_caption[2];
static gui_widget * y_text[2], * y_caption[2];

extern BMPINFO machine_name[];

#define  bytes_per_row  48

gui_widget * init_plc_io_monitor_screen(void)
{
    int x, y, w, h, i;
    char ___s[32];

    #define ___X_1  20
    #define ___X_2  504

    #define ___BK_COLOR  236
    
    /* 主界面   */
    main_form = gui_create_widget(GUI_WIDGET_FORM, 10, 422, 998, 226, 0, ___BK_COLOR, 0, FORM_STYLE_THIN_BDR);
    if(!main_form)
        goto some_error;
    gui_form_init_private(main_form, 8);

    /*
    ** --------------------------  1 机 -------------------------------------------
    */
    i = 0;
    x = ___X_1;
    y = 10;
    w = 468;
    h = 196;
    group[i] = gui_create_widget(GUI_WIDGET_GROUPBOX, x, y, w, h, 0, ___BK_COLOR, 1, GROUPBOX_STYLE_CAPTION);
    if(!group)
        goto some_error;
    gui_widget_link(main_form, group[i]);
    gui_groupbox_init_private(group[i], 16);
    gui_groupbox_set_caption(group[i], pick_string(" 机1-PLC ", " M1-PLC "));

    x = 14;
    y = 48;
    w = 132;
    h = 36;
    x_caption[i] = gui_create_widget(GUI_WIDGET_LABEL, x, y, w, h, 0, ___BK_COLOR, 1, LABEL_STYLE_TRANSPARENT);
    if(!x_caption[i])
        goto some_error;
    gui_widget_link(group[i], x_caption[i]);
    gui_label_init_private(x_caption[i], 16);
    sprintf(___s, "X%d - X%d:", (i*32), (i*32)+31);
    gui_label_set_text(x_caption[i], ___s);
    
    x = x + 80;
    w = 342;
    x_text[i] = gui_create_widget(GUI_WIDGET_EDIT, x, y, w, h, 0, 0, 1, EDIT_STYLE_STATIC_BDR|EDIT_STYLE_NO_BORDER);
    if(!x_text[i])
        goto some_error;
    gui_widget_link(group[i], x_text[i]);
    gui_edit_init_private(x_text[i], bytes_per_row);

    x = 14;
    y = 108;
    w = 132;
    h = 36;
    y_caption[i] = gui_create_widget(GUI_WIDGET_LABEL, x, y, w, h, 0, ___BK_COLOR, 1, LABEL_STYLE_TRANSPARENT);
    if(!y_caption[i])
        goto some_error;
    gui_widget_link(group[i], y_caption[i]);
    gui_label_init_private(y_caption[i], 16);
    sprintf(___s, "Y%d - Y%d:", (i*32), (i*32)+31);
    gui_label_set_text(y_caption[i], ___s);
    
    x = x + 80;
    w = 342;
    y_text[i] = gui_create_widget(GUI_WIDGET_EDIT, x, y, w, h, 0, 0, 1, EDIT_STYLE_STATIC_BDR|EDIT_STYLE_NO_BORDER);
    if(!y_text[i])
        goto some_error;
    gui_widget_link(group[i], y_text[i]);
    gui_edit_init_private(y_text[i], bytes_per_row);

    /*
    ** --------------------------  2 机 -------------------------------------------
    */
    i = 1;
    x = ___X_2;
    y = 10;
    w = 468;
    h = 196;
    group[i] = gui_create_widget(GUI_WIDGET_GROUPBOX, x, y, w, h, 0, ___BK_COLOR, 1, GROUPBOX_STYLE_CAPTION);
    if(!group)
        goto some_error;
    gui_widget_link(main_form, group[i]);
    gui_groupbox_init_private(group[i], 16);
    gui_groupbox_set_caption(group[i], pick_string(" 机2-PLC ", " M2-PLC "));

    x = 14;
    y = 48;
    w = 132;
    h = 36;
    x_caption[i] = gui_create_widget(GUI_WIDGET_LABEL, x, y, w, h, 0, ___BK_COLOR, 1, LABEL_STYLE_TRANSPARENT);
    if(!x_caption[i])
        goto some_error;
    gui_widget_link(group[i], x_caption[i]);
    gui_label_init_private(x_caption[i], 16);
    sprintf(___s, "X%d - X%d:", 0, 31);
    gui_label_set_text(x_caption[i], ___s);
    
    x = x + 80;
    w = 342;
    x_text[i] = gui_create_widget(GUI_WIDGET_EDIT, x, y, w, h, 0, 0, 1, EDIT_STYLE_STATIC_BDR|EDIT_STYLE_NO_BORDER);
    if(!x_text[i])
        goto some_error;
    gui_widget_link(group[i], x_text[i]);
    gui_edit_init_private(x_text[i], bytes_per_row);

    x = 14;
    y = 108;
    w = 132;
    h = 36;
    y_caption[i] = gui_create_widget(GUI_WIDGET_LABEL, x, y, w, h, 0, ___BK_COLOR, 1, LABEL_STYLE_TRANSPARENT);
    if(!y_caption[i])
        goto some_error;
    gui_widget_link(group[i], y_caption[i]);
    gui_label_init_private(y_caption[i], 16);
    sprintf(___s, "Y%d - Y%d:", 0, 31);
    gui_label_set_text(y_caption[i], ___s);
    
    x = x + 80;
    w = 342;
    y_text[i] = gui_create_widget(GUI_WIDGET_EDIT, x, y, w, h, 0, 0, 1, EDIT_STYLE_STATIC_BDR|EDIT_STYLE_NO_BORDER);
    if(!y_text[i])
        goto some_error;
    gui_widget_link(group[i], y_text[i]);
    gui_edit_init_private(y_text[i], bytes_per_row);

    return main_form;

some_error:
    sys_print("init_cim_link_monitor_screen(): failed to create widgets!\n");
    ExitApplication();
    return NULL;
}

void plc_io_monitor(void)
{
    int  i, j, m;
    INT16U k;
    char ___s[64];
    INT08S buf[bytes_per_row];
    extern PLC * slc_plc[];
    extern int * plc_io_x[];
    extern int * plc_io_y[];

    FamesAssert(main_form);

    if(!main_form)
        return;

    if(slc_plc[0]){
        plc_action_enable_id(slc_plc[0], id_io_x);
        plc_action_enable_id(slc_plc[0], id_io_y);
    }
    if(slc_plc[1]){
        plc_action_enable_id(slc_plc[1], id_io_x);
        plc_action_enable_id(slc_plc[1], id_io_y);
    }

    gui_show_widget(main_form);

    for(;;){
        /*
        ** --------------------------  1 机 ---------------------------------------
        */
        m = 0;
        /* X 点部分 */
        lock_kernel();
        if(plc_io_x[m]){
            for(i = 0; i < 2; i++){ /* 生成二进制内容 */
                k = (INT16U)plc_io_x[m][i];
                for(j=0; j<16; j++){
                    ___s[i*16+j] = (k&1)?'1':'-';
                    k >>= 1;
                }
            }
        } else {
            MEMSET(___s, '0', 160);
        }
        unlock_kernel();
        for(i = 0, j = 1; i < 32; j++, i++){
            buf[j]  = ___s[i];
            if(!((i+1)%8) && i<31){
                buf[++j] = ' ';
                buf[++j] = ' ';
            }
        }
        buf[0] = ' ';
        buf[j] = 0;
        gui_edit_set_text(x_text[m], buf);
        /* Y 点部分 */
        lock_kernel();
        if(plc_io_y[m]){
            for(i = 0; i < 2; i++){ /* 生成二进制内容 */
                k = (INT16U)plc_io_y[m][i];
                for(j=0; j<16; j++){
                    ___s[i*16+j] = (k&1)?'1':'-';
                    k >>= 1;
                }
            }
        } else {
            MEMSET(___s, '0', 160);
        }
        unlock_kernel();
        for(i = 0, j = 1; i < 32; j++, i++){
            buf[j]  = ___s[i];
            if(!((i+1)%8) && i<31){
                buf[++j] = ' ';
                buf[++j] = ' ';
            }
        }
        buf[0] = ' ';
        buf[j] = 0;
        gui_edit_set_text(y_text[m], buf);

        /*
        ** --------------------------  2 机 ---------------------------------------
        */
        m = 1;
        /* X 点部分 */
        lock_kernel();
        if(plc_io_x[m]){
            for(i = 0; i < 2; i++){ /* 生成二进制内容 */
                k = (INT16U)plc_io_x[m][i];
                for(j=0; j<16; j++){
                    ___s[i*16+j] = (k&1)?'1':'-';
                    k >>= 1;
                }
            }
        } else {
            MEMSET(___s, '0', 160);
        }
        unlock_kernel();
        for(i = 0, j = 1; i < 32; j++, i++){
            buf[j]  = ___s[i];
            if(!((i+1)%8) && i<31){
                buf[++j] = ' ';
                buf[++j] = ' ';
            }
        }
        buf[0] = ' ';
        buf[j] = 0;
        gui_edit_set_text(x_text[m], buf);
        /* Y 点部分 */
        lock_kernel();
        if(plc_io_y[m]){
            for(i = 0; i < 2; i++){ /* 生成二进制内容 */
                k = (INT16U)plc_io_y[m][i];
                for(j=0; j<16; j++){
                    ___s[i*16+j] = (k&1)?'1':'-';
                    k >>= 1;
                }
            }
        } else {
            MEMSET(___s, '0', 160);
        }
        unlock_kernel();
        for(i = 0, j = 1; i < 32; j++, i++){
            buf[j]  = ___s[i];
            if(!((i+1)%8) && i<31){
                buf[++j] = ' ';
                buf[++j] = ' ';
            }
        }
        buf[0] = ' ';
        buf[j] = 0;
        gui_edit_set_text(y_text[m], buf);

        /*
        ** -------------- 等待按键 ------------------------------------------------
        */
        if(waitkey(20L) == ESC)
            break;
    }
    
    gui_hide_widget(main_form);
    
    if(slc_plc[0]){
        plc_action_disable_id(slc_plc[0], id_io_x);
        plc_action_disable_id(slc_plc[0], id_io_y);
    }
    if(slc_plc[1]){
        plc_action_disable_id(slc_plc[1], id_io_x);
        plc_action_disable_id(slc_plc[1], id_io_y);
    }
}


/*=========================================================================================
 * 
 * 本文件结束: plc_test.c
 * 
**=======================================================================================*/


