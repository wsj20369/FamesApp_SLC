/******************************************************************************************
 * 文件:    slc/slc_para.c
 *
 * 描述:    分压机参数设置
 *
 * 作者:    Jun
******************************************************************************************/
#define  SLC_PARAMETER_C
#include <FamesOS.h>
#include "common.h"


extern struct slc_config_s config;

/*-----------------------------------------------------------------------------------------
 * 
 *      正在设置参数的SLC序号
 * 
**---------------------------------------------------------------------------------------*/
static int __current_slc_to_setup = 0; /* 0/1 */


/*-----------------------------------------------------------------------------------------
 *          
 *      参数画面中的控件及其它定义
 * 
**---------------------------------------------------------------------------------------*/
gui_widget * __slc_param_screen   = NULL; /* 参数画面的主控件      */
gui_widget * slc_param_status_bar = NULL; /* 参数画面的状态条      */
gui_widget * slc_param_cpu_status = NULL; /* 参数画面下的CPU使用率 */

gui_widget * param_machine_name = NULL;   /* 参数画面下机器名      */

gui_widget * param_kl_view  = NULL;       /* 用于刀线参数的控件    */
gui_widget * param_misc_dlg = NULL;       /* 用于其它参数的对话框  */

extern BMPINFO  icon;                     /* 图标                  */

BOOL param_kl_init_view_private(void);
gui_widget * init_misc_param_dlg(void);

/*-----------------------------------------------------------------------------------------
 *          
 *      参数画面的定义(或初始化)
 * 
**---------------------------------------------------------------------------------------*/
gui_widget * init_slc_param_screen(void)
{
    /* 主界面       */
    __slc_param_screen = gui_create_widget(GUI_WIDGET_FORM, 1, 1, 1021, 765, 0, 0, font16, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE);
    if(!__slc_param_screen)
        goto some_error;
    gui_widget_link(NULL, __slc_param_screen);      /* 设置主界面背景 */
    gui_form_init_private(__slc_param_screen, 128);
    gui_form_set_icon(__slc_param_screen, &icon);
    gui_form_set_caption(__slc_param_screen, pick_string("参数设置", "Setup"));
    
    /* 工具条       */
    slc_param_status_bar = gui_create_widget(GUI_WIDGET_LABEL, 7, 725, 872, 30, 0, 0, 1, LABEL_STYLE_CLIENT_BDR);
    if(!slc_param_status_bar)
        goto some_error;
    gui_widget_link(__slc_param_screen, slc_param_status_bar);
    gui_label_init_private(slc_param_status_bar, 128);
    gui_label_set_text(slc_param_status_bar, pick_string(">>>F1 其它参数        >>>F10 确认                  >>>ESC 返回",
                                                         ">>>F1 Other Param     >>>F10 Save                  >>>ESC Return"));

    /* CPU使用率    */
    slc_param_cpu_status = gui_create_widget(GUI_WIDGET_LABEL, 880, 725, 132, 30, 0, 0, 1, LABEL_STYLE_CLIENT_BDR|LABEL_ALIGN_CENTER);
    if(!slc_param_cpu_status)
        goto some_error;
    gui_widget_link(__slc_param_screen, slc_param_cpu_status);
    gui_label_init_private(slc_param_cpu_status, 32);
    gui_label_set_text(slc_param_cpu_status, "");

    /* 机器名       */
    param_machine_name = gui_create_widget(GUI_WIDGET_LABEL, 10, 36, 998, 32, COLOR_YELLOW, 17, font24, LABEL_ALIGN_CENTER|LABEL_STYLE_SUBSIDE);
    if(!param_machine_name)
        goto some_error;
    gui_widget_link(__slc_param_screen, param_machine_name);
    gui_label_init_private(param_machine_name, 128);
    gui_label_set_text(param_machine_name, "");

    /* 刀线参数     */
    param_kl_view = gui_create_widget(GUI_WIDGET_VIEW, 10, 72, 998, 633, 0, 0, 1, 0x70|VIEW_STYLE_NONE_FIRST);
    if(!param_kl_view)
        goto some_error;
    gui_widget_link(__slc_param_screen, param_kl_view);
    if(!param_kl_init_view_private())
        goto some_error;

    /* 其它参数     */
    param_misc_dlg = init_misc_param_dlg();
    if(!param_misc_dlg)
        goto some_error;
    gui_hide_widget(param_misc_dlg);
    gui_widget_link(__slc_param_screen, param_misc_dlg);
    
    return __slc_param_screen;

some_error:
    sys_print("init_slc_param_screen(): failed to create widgets!\n");
    ExitApplication();
    return NULL;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      刀线参数区域(param_kl)的相关定义
 * 
**---------------------------------------------------------------------------------------*/
static struct slc_config_s * __kl_config = NULL;

enum param_kl_fields_id {
    __id_kl_caption,
    __id_lmt_left,
    __id_lmt_right,
    __id_distance,
    __id_disable,
    __id_standard,
    __id_regress,
    __id_kl_unit,
    __id_kl_fixset,
    __id_kl_adjust,
};

static view_fields_t param_kl_fields[] =
{
#define ____style        0
#define ____draw_style   DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER
    { "刀/线",     __id_kl_caption,  6,  8,  ____style,  ____draw_style,  "刀名/线名", },
    { "左限",      __id_lmt_left,    8,  11, ____style,  ____draw_style,  "左限, 刀线可到达的最左边位置", },
    { "右限",      __id_lmt_right,   8,  11, ____style,  ____draw_style,  "右限, 刀线可到达的最右边位置", },
    { "右间距",    __id_distance,    8,  11, ____style,  ____draw_style,  "右间距, 刀刀间距或线线间距", },
    { "不可用",    __id_disable,     1,  8,  ____style,  ____draw_style,  "不可用, 刀线的不可用状态", },
    { "标准位置",  __id_standard,    8,  11, ____style,  ____draw_style,  "标准位置, 未用刀线的默认位置", },
    { "归零位置",  __id_regress,     8,  11, ____style,  ____draw_style,  "归零位置, 用于自动校正", },
    { "单位值",    __id_kl_unit,     8,  11, ____style,  ____draw_style,  "刀线单位值", },
    { "+/-值",     __id_kl_fixset,   8,  11, ____style,  ____draw_style,  "刀线+/-值", },
    { "校正值",    __id_kl_adjust,   8,  11, ____style,  ____draw_style,  "刀线的实际位置校正", },
    { NULL, }
#undef ____style
#undef ____draw_style
};

static void ____set_comment_en(void)
{
    char * s;

    #define ____do_set(index, name_en, comment_en) \
                    do { \
                        s = pick_string(NULL, name_en); \
                        if(s) \
                            param_kl_fields[index].caption = s; \
                        s = pick_string(NULL, comment_en); \
                        if(s) \
                            param_kl_fields[index].comment = comment_en; \
                    } while(0)

    ____do_set(0, "K/L",       "Knife/Line No");
    ____do_set(1, "Leftmost",  "The Leftmost Position");
    ____do_set(2, "Rightmost", "The Rightmost Position");
    ____do_set(3, "Distance",  "The Distance Between K/K or L/L");
    ____do_set(4, "Disable",   "Indicate a K/L that can Not be Used");
    ____do_set(5, "Standard",  "Standard or Default Position for Unused K/L");
    ____do_set(6, "Regress",   "The Regress Position, Used for Auto-Adjust");
    ____do_set(7, "Unit",      "The Unit for K/L");
    ____do_set(8, "+/-",       "The +/- value for K/L");
    ____do_set(9, "Adjust",    "Adjust the Position for K/L");

    #undef ____do_set
}

int param_kl_get_max(void)
{
    slc_descriptor_t * slc;

    lock_kernel();
    slc = &(__kl_config->slc[__current_slc_to_setup]);
    unlock_kernel();
    
    return ((slc->k_number + slc->l_number) - 1);
}

static void param_kl_show_record(int index, int row, 
                               int x, int y, int width_zoom, int height, 
                               COLOR color, COLOR bkcolor, int font,
                               COLOR marker_color, COLOR marker_bkcolor,
                               void *old, int  *fields_width, INT16U option)
{
    struct ___old_buf {
        char kl_name[8];
        char lmt_left[10];
        char lmt_right[10];
        char distance[10];
        char disable[8];
        char standard[10];
        char regress[10];
        char unit[10];
        char fixset[10];
        char adjust[10];
    };
    struct ___old_buf * record_buf_old;
    int j, to_left;
    char ___s[16];
    int kl, is_k;
    INT16U chg_flag, draw_flag;
    slc_descriptor_t * slc;
    
    FamesAssert(__kl_config);
    FamesAssert(old);
    if(!__kl_config || !old)
        return;

    option  = option;

    slc = &(__kl_config->slc[__current_slc_to_setup]);
    record_buf_old = (struct ___old_buf *)old;
    row = row;
    to_left = width_zoom;

    kl = index;
    is_k = 1;
    color = 254;
    if(kl >= slc->k_number){
        kl -= slc->k_number;
        is_k = 0;
        color = 251;
    }

    chg_flag = CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_SIG|CHG_OPT_FRC|0x81;

    draw_flag = DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER;

    if(index <= param_kl_get_max()){
        /* 刀线名 */
        j = 0;
        sprintf(___s, "%s%d", (is_k?pick_string("刀", "K"):pick_string("线", "L")), (kl+1));
        draw_font_for_widget(x+3, y, (fields_width[j]-to_left)-6, height, ___s, 
                             record_buf_old->kl_name, marker_color, marker_bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 左限 */
        j++;
        INT16toSTR(___s, (is_k?slc->k_lmt_left[kl]:slc->l_lmt_left[kl]), chg_flag);
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                             record_buf_old->lmt_left, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 右限 */
        j++;
        INT16toSTR(___s, (is_k?slc->k_lmt_right[kl]:slc->l_lmt_right[kl]), chg_flag);
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                             record_buf_old->lmt_right, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 右间距 */
        j++;
        INT16toSTR(___s, (is_k?slc->k_distance[kl]:slc->l_distance[kl]), chg_flag);
        if(is_k){
            if(kl >= (slc->k_number-1)){
                STRCPY(___s, "--");
            }
        } else {
            if(kl >= (slc->l_number-1)){
                STRCPY(___s, "--");
            }
            if(slc->slc_type == SLC_TYPE_DOUBLE && kl >= (slc->l_number-2)){
                STRCPY(___s, "--");
            }
        }
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                             record_buf_old->distance, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 不可用 */
        j++;
        if(is_k?slc->k_disable[kl]:slc->l_disable[kl])
            ___s[0] = '1';
        else
            ___s[0] = '0';
        ___s[1] = 0;
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                             record_buf_old->disable, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 标准位置 */
        j++;
        INT16toSTR(___s, (is_k?slc->kl_standard.k_location[kl]:slc->kl_standard.l_location[kl]), chg_flag);
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                             record_buf_old->standard, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 归零位置 */
        j++;
        INT16toSTR(___s, (is_k?slc->kl_regress.k_location[kl]:slc->kl_regress.l_location[kl]), chg_flag);
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                             record_buf_old->regress, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 单位值 */
        j++;
        INT16toSTR(___s, (is_k?slc->unit.k_unit[kl]:slc->unit.l_unit[kl]), chg_flag);
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                             record_buf_old->unit, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* +/-值 */
        j++;
        INT16toSTR(___s, (is_k?slc->fix_set.k_fix_set[kl]:slc->fix_set.l_fix_set[kl]), chg_flag);
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                             record_buf_old->fixset, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 校正值 */
        j++;
        INT16toSTR(___s, (is_k?slc->kl_adjust.k_location[kl]:slc->kl_adjust.l_location[kl]), chg_flag);
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                             record_buf_old->adjust, color, bkcolor, font, draw_flag);
        x += fields_width[j];
    } else {
        /* 刀线名 */
        j = 0;
        draw_font_for_widget(x+3, y, (fields_width[j]-to_left)-6, height, "", 
                             record_buf_old->kl_name, marker_color, marker_bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 左限 */
        j++;
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, "", 
                             record_buf_old->lmt_left, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 右限 */
        j++;
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, "", 
                             record_buf_old->lmt_right, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 右间距 */
        j++;
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, "", 
                             record_buf_old->distance, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 不可用 */
        j++;
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, "", 
                             record_buf_old->disable, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 标准位置 */
        j++;
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, "", 
                             record_buf_old->standard, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 归零位置 */
        j++;
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, "", 
                             record_buf_old->regress, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 单位值 */
        j++;
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, "", 
                             record_buf_old->unit, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* +/-值 */
        j++;
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, "", 
                             record_buf_old->fixset, color, bkcolor, font, draw_flag);
        x += fields_width[j];
        /* 校正值 */
        j++;
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, "", 
                             record_buf_old->adjust, color, bkcolor, font, draw_flag);
        x += fields_width[j];
    }
}

void param_kl_show_statistics(int index,
                   int x, int y, int width, int height, 
                   int color, int bkcolor, int font,
                   INT08S *field_comment,
                   INT08S *old, INT16U option)
{
    char ___s[96];
    int  kl, is_k;
    slc_descriptor_t * slc;
    
    FamesAssert(__kl_config);
    FamesAssert(old);
    if(!__kl_config || !old)
        return;

    option = option;

    lock_kernel();
    slc = &(__kl_config->slc[__current_slc_to_setup]);
    unlock_kernel();
    
    kl = index;
    is_k = 1;
    if(kl >= slc->k_number){
        kl -= slc->k_number;
        is_k = 0;
    }
    sprintf(___s, " %s%d:  %s.", (is_k?pick_string("刀", "K"):pick_string("线", "L")), (kl+1), (field_comment?field_comment:""));

    draw_font_for_widget(x, y, width, height, ___s, old, color, bkcolor, font, DRAW_OPT_FIL_BG);
}

BOOL param_kl_is_writable(int index, int field_id, INT16U option)
{
    int kl, is_k;
    BOOL retval;
    slc_descriptor_t * slc;
    
    FamesAssert(__kl_config);
    if(!__kl_config)
        return fail;

    option = option;

    slc = &(__kl_config->slc[__current_slc_to_setup]);

    kl = index;
    is_k = 1;
    if(kl >= slc->k_number){
        kl -= slc->k_number;
        is_k = 0;
    }
    
    retval = ok;

    switch(field_id){
        case __id_lmt_left:   /* 左限       */
        case __id_lmt_right:  /* 右限       */
        case __id_disable:    /* 不可用     */
        case __id_standard:   /* 标准位置   */
        case __id_regress:    /* 归零位置   */
        case __id_kl_unit:    /* 单位值     */
        case __id_kl_fixset:  /* +/-值      */
        case __id_kl_adjust:  /* 校正值     */
            break;
        case __id_distance:   /* 右间距     */
            if(is_k){
                if(kl >= (slc->k_number-1)){
                    retval = fail;
                }
            } else {
                if(kl >= (slc->l_number-1)){
                    retval = fail;
                }
                if(slc->slc_type == SLC_TYPE_DOUBLE && kl >= (slc->l_number-2)){
                    retval = fail;
                }
            }
            retval = ok; 
            break;
        case __id_kl_caption: /* 刀线名     */
            retval = fail;
            break;
        default:
            retval = fail;
            break;
    }
    
    return  retval;
}

BOOL param_kl_get_item(int index, int field_id, char * buf, int buf_len, INT16U option)
{
    int kl, is_k;
    INT16U chg_flag;
    BOOL retval;
    slc_descriptor_t * slc;
    
    FamesAssert(__kl_config);
    FamesAssert(buf);
    if(!__kl_config || !buf)
        return fail;

    option  = option;
    buf_len = buf_len;  /* 这里不用, 是因为这里可以保证不会超过这个长度 */

    if(index > param_kl_get_max())
        return fail;

    slc = &(__kl_config->slc[__current_slc_to_setup]);

    kl = index;
    is_k = 1;
    if(kl >= slc->k_number){
        kl -= slc->k_number;
        is_k = 0;
    }
    
    chg_flag = CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_SIG|CHG_OPT_FRC|0x81;

    retval = ok;

    switch(field_id){
        case __id_kl_caption: /* 刀线名 */
            sprintf(buf, "%s%d", (is_k?pick_string("刀", "K"):pick_string("线", "L")), (kl+1));
            break;
        case __id_lmt_left: /* 左限 */
            INT16toSTR(buf, (is_k?slc->k_lmt_left[kl]:slc->l_lmt_left[kl]), chg_flag);
            break;
        case __id_lmt_right: /* 右限 */
            INT16toSTR(buf, (is_k?slc->k_lmt_right[kl]:slc->l_lmt_right[kl]), chg_flag);
            break;
        case __id_distance: /* 右间距 */
            INT16toSTR(buf, (is_k?slc->k_distance[kl]:slc->l_distance[kl]), chg_flag);
            if(is_k){
                if(kl >= (slc->k_number-1)){
                    STRCPY(buf, "--");
                }
            } else {
                if(kl >= (slc->l_number-1)){
                    STRCPY(buf, "--");
                }
                if(slc->slc_type == SLC_TYPE_DOUBLE && kl >= (slc->l_number-2)){
                    STRCPY(buf, "--");
                }
            }
            break;
        case __id_disable: /* 不可用 */
            if(is_k?slc->k_disable[kl]:slc->l_disable[kl])
                buf[0] = '1';
            else
                buf[0] = '0';
            buf[1] = 0;
            break;
        case __id_standard: /* 标准位置 */
            INT16toSTR(buf, (is_k?slc->kl_standard.k_location[kl]:slc->kl_standard.l_location[kl]), chg_flag);
            break;
        case __id_regress: /* 归零位置 */
            INT16toSTR(buf, (is_k?slc->kl_regress.k_location[kl]:slc->kl_regress.l_location[kl]), chg_flag);
            break;
        case __id_kl_unit: /* 单位值 */
            INT16toSTR(buf, (is_k?slc->unit.k_unit[kl]:slc->unit.l_unit[kl]), chg_flag);
            break;
        case __id_kl_fixset: /* +/-值 */
            INT16toSTR(buf, (is_k?slc->fix_set.k_fix_set[kl]:slc->fix_set.l_fix_set[kl]), chg_flag);
            break;
        case __id_kl_adjust: /* 校正值 */
            INT16toSTR(buf, (is_k?slc->kl_adjust.k_location[kl]:slc->kl_adjust.l_location[kl]), chg_flag);
            break;
        default:
            retval = fail;
            break;
    }
    
    return  retval;
}

BOOL param_kl_set_item(int index, int field_id, char * buf, int buf_len, KEYCODE key, INT16U option)
{
    int kl, is_k;
    INT16U chg_flag;
    int  temp;
    slc_descriptor_t * slc;
    
    FamesAssert(__kl_config);
    FamesAssert(buf);
    if(!__kl_config || !buf)
        return fail;

    option  = option;
    buf_len = buf_len;  /* 这里不用, 是因为这里可以保证不会超过这个长度 */

    if(index > param_kl_get_max())
        return fail;

    slc = &(__kl_config->slc[__current_slc_to_setup]);

    kl = index;
    is_k = 1;
    if(kl >= slc->k_number){
        kl -= slc->k_number;
        is_k = 0;
    }
    
    chg_flag = CHG_OPT_DEC|CHG_OPT_FRC|0x81;
    temp = STRtoINT16(buf, chg_flag);

    switch(field_id){
        case __id_lmt_left: /* 左限 */
            if(is_k)
                slc->k_lmt_left[kl] = temp;
            else 
                slc->l_lmt_left[kl] = temp;
            break;
        case __id_lmt_right: /* 右限 */
            if(is_k)
                slc->k_lmt_right[kl] = temp;
            else 
                slc->l_lmt_right[kl] = temp;
            break;
        case __id_distance: /* 右间距 */
            if(is_k)
                slc->k_distance[kl] = temp;
            else 
                slc->l_distance[kl] = temp;
            break;
        case __id_disable: /* 不可用 */
            temp = STRtoINT16(buf, CHG_OPT_DEC);
            if(is_k)
                slc->k_disable[kl] = temp;
            else 
                slc->l_disable[kl] = temp;
            break;
        case __id_standard: /* 标准位置 */
            if(is_k)
                slc->kl_standard.k_location[kl] = temp;
            else 
                slc->kl_standard.l_location[kl] = temp;
            break;
        case __id_regress: /* 归零位置 */
            if(is_k)
                slc->kl_regress.k_location[kl] = temp;
            else 
                slc->kl_regress.l_location[kl] = temp;
            if(key == F10){
                copy_to_config(__kl_config);
                slc_send_regress_value(__current_slc_to_setup+1);
            }
            break;
        case __id_kl_unit:  /* 单位值 */
            if(is_k)
                slc->unit.k_unit[kl] = temp;
            else 
                slc->unit.l_unit[kl] = temp;
            if(key == F10){
                copy_to_config(__kl_config);
                slc_send_kl_unit_value(__current_slc_to_setup+1);
            }
            break;
        case __id_kl_fixset: /* +/-值 */
            if(is_k)
                slc->fix_set.k_fix_set[kl] = temp;
            else 
                slc->fix_set.l_fix_set[kl] = temp;
            if(key == F10){
                copy_to_config(__kl_config);
                slc_send_kl_fix_value(__current_slc_to_setup+1);
            }
            break;
        case __id_kl_adjust:  /* 校正值 */
            if(is_k)
                slc->kl_adjust.k_location[kl] = temp;
            else 
                slc->kl_adjust.l_location[kl] = temp;
            if(key == F10){
                copy_to_config(__kl_config);
                slc_send_kl_act_value(__current_slc_to_setup+1);
            }
            break;
        default:
            return fail;
    }

    return  ok;
}

KEYCODE setup_kl_param(void)
{
    KEYCODE key;

    FamesAssert(__kl_config);
    if(!__kl_config)
        return NONEKEY;

    key = gui_view_editing(param_kl_view, 0);

    if(key == NONEKEY)
        key = waitkey(0L);

    return key;
}

BOOL param_kl_init_view_private(void)
{
    BOOL retval;

    __kl_config = (struct slc_config_s *)mem_alloc(sizeof(*__kl_config));
    if(!__kl_config)
        return fail;
    *__kl_config = config;

    ____set_comment_en();
    
    retval = gui_view_init_private( param_kl_view, 
                                    param_kl_fields, 
                                    param_kl_get_max,
                                    param_kl_get_item,
                                    param_kl_set_item, 
                                    param_kl_is_writable,
                                    param_kl_show_record,
                                    pick_string("=说=明=", "Comment"),
                                    param_kl_show_statistics, 
                                    NULL,
                                    COLOR_YELLOW, /* 123, */
                                    17, /* WIDGET_BKCOLOR+65, */
                                    30, /*height_per_row*/
                                    1
                                  );

    return retval;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      其它参数区域(param_other)的相关定义
 * 
**---------------------------------------------------------------------------------------*/
enum misc_param_id {
    misc_p_width,
    misc_p_trim,
    misc_p_prep,
    misc_p_a_std,
    misc_p_lmax,
    misc_p_rmax,
    misc_p_offset,

    misc_p_u_fan,
    misc_p_u_p1,
    misc_p_u_p2,
    misc_p_spg,
    misc_p_ad_f,
    misc_p_ad_p1,
    misc_p_ad_p2,
    misc_p_ad_big,

    misc_p_big_k,
    misc_p_sml_k,
    misc_p_big_l,
    misc_p_sml_l,
    misc_p_big_f,
    misc_p_sml_f,
    misc_p_big_p,
    misc_p_sml_p,

    misc_p_dn_k,
    misc_p_up_k,
    misc_p_dn_l,
    misc_p_up_l,
    misc_p_dn_l2,
    misc_p_up_l2,
    misc_k_dn_dly,
    misc_k_up_dly,
    misc_l_dn_dly,
    misc_l_up_dly,
    misc_l2_dn_dly,
    misc_l2_up_dly,

    misc_p_adj_p1,
    misc_p_adj_p2,
    misc_p_adj_fan,

    misc_p_yx_big,
    misc_p_yx_small,

    misc_p_spdscale,

    misc_p_group,
};

#define ___STYLE  0
#define ___FONT   1  /* font16 */
#define ___BYTES  8
#define ___X  22
#define ___Y  42
DECLARE_INPUT_DIALOG_START(misc_param_dialog)
/*                 参数名称          参数ID          字节数          X         Y       W   H     FONT     STYLE      COMMENT     */
INPUT_DIALOG_GROUP("", misc_p_group, ___X, ___Y, 422, 162, 0, 0)
INPUT_DIALOG_ITEM("机械幅宽:",       misc_p_width,    ___BYTES,  ___X+10,  ___Y+10,   164, 28, ___FONT, ___STYLE, "1200mm ~ 2800mm")
INPUT_DIALOG_ITEM("是否修边:",       misc_p_trim,     1,         ___X+10,  ___Y+48,   164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("是否预压:",       misc_p_prep,     1,         ___X+10,  ___Y+86,   164, 28, ___FONT, ___STYLE, "自动预压时, 后排线跟随前排线而动, 仅用于零压型")
INPUT_DIALOG_ITEM("自动标准:",       misc_p_a_std,    1,         ___X+10,  ___Y+124,  164, 28, ___FONT, ___STYLE, "是否使用自动生成的标准位置")
INPUT_DIALOG_ITEM("向左最大偏移度:", misc_p_lmax,     ___BYTES,  ___X+194, ___Y+10,   212, 28, ___FONT, ___STYLE, "排单时的最大中心偏移(向左时)")
INPUT_DIALOG_ITEM("向右最大偏移度:", misc_p_rmax,     ___BYTES,  ___X+194, ___Y+67,   212, 28, ___FONT, ___STYLE, "排单时的最大中心偏移(向右时)")
INPUT_DIALOG_ITEM("整  体  偏  移:", misc_p_offset,   ___BYTES,  ___X+194, ___Y+124,  212, 28, ___FONT, ___STYLE, "送到PLC时的整体偏移(正数=右偏移, 负数=左偏移)")
#undef  ___X
#undef  ___Y
#define ___X  22
#define ___Y  212
INPUT_DIALOG_GROUP("", misc_p_group, ___X, ___Y, 422, 162, 0, 0)
INPUT_DIALOG_ITEM("吸风单位:",       misc_p_u_fan,    ___BYTES,  ___X+10,  ___Y+10,  164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("压1 单位:",       misc_p_u_p1,     ___BYTES,  ___X+10,  ___Y+48,  164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("压2 单位:",       misc_p_u_p2,     ___BYTES,  ___X+10,  ___Y+86,  164, 28, ___FONT, ___STYLE, "后压深单位, 仅用于零压型")
INPUT_DIALOG_ITEM("车速 SPG:",       misc_p_spg,      ___BYTES,  ___X+10,  ___Y+124, 164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("吸风  +/-值:   ", misc_p_ad_f,     ___BYTES,  ___X+194, ___Y+10,  212, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("压1   +/-值:   ", misc_p_ad_p1,    ___BYTES,  ___X+194, ___Y+48,  212, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("压2   +/-值:   ", misc_p_ad_p2,    ___BYTES,  ___X+194, ___Y+86,  212, 28, ___FONT, ___STYLE, "后压深+/-值, 仅用于零压型")
INPUT_DIALOG_ITEM("粗启动+/-值:   ", misc_p_ad_big,   ___BYTES,  ___X+194, ___Y+124, 212, 28, ___FONT, ___STYLE, "")
#undef  ___X
#undef  ___Y
#define ___X  22
#define ___Y  382
INPUT_DIALOG_GROUP("", misc_p_group, ___X, ___Y, 422, 124, 0, 0)
INPUT_DIALOG_ITEM("压1 校正:",       misc_p_adj_p1,   ___BYTES,  ___X+10,  ___Y+10,  164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("压2 校正:",       misc_p_adj_p2,   ___BYTES,  ___X+10,  ___Y+48,  164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("吸风校正:",       misc_p_adj_fan,  ___BYTES,  ___X+10,  ___Y+86,  164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("车速比率:      ", misc_p_spdscale, ___BYTES,  ___X+194, ___Y+10,  212, 28, ___FONT, ___STYLE, "")
#undef  ___X
#undef  ___Y
#define ___X  454
#define ___Y  42
INPUT_DIALOG_GROUP("", misc_p_group, ___X, ___Y, 390, 162, 0, 0)
INPUT_DIALOG_ITEM("刀粗定位:",       misc_p_big_k,    ___BYTES,  ___X+10,  ___Y+10,  164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("刀细定位:",       misc_p_sml_k,    ___BYTES,  ___X+10,  ___Y+48,  164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("线粗定位:",       misc_p_big_l,    ___BYTES,  ___X+10,  ___Y+86,  164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("线细定位:",       misc_p_sml_l,    ___BYTES,  ___X+10,  ___Y+124, 164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("吸风粗定位:",     misc_p_big_f,    ___BYTES,  ___X+194, ___Y+10,  180, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("吸风细定位:",     misc_p_sml_f,    ___BYTES,  ___X+194, ___Y+48,  180, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("压深粗定位:",     misc_p_big_p,    ___BYTES,  ___X+194, ___Y+86,  180, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("压深细定位:",     misc_p_sml_p,    ___BYTES,  ___X+194, ___Y+124, 180, 28, ___FONT, ___STYLE, "")
#undef  ___X
#undef  ___Y
#define ___X  454
#define ___Y  212
INPUT_DIALOG_GROUP("", misc_p_group, ___X, ___Y, 390, 238, 0, 0)
INPUT_DIALOG_ITEM("刀下米数:",       misc_p_dn_k,     ___BYTES,  ___X+10,  ___Y+10,  164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("刀上米数:",       misc_p_up_k,     ___BYTES,  ___X+10,  ___Y+48,  164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("线下米数:",       misc_p_dn_l,     ___BYTES,  ___X+10,  ___Y+86,  164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("线上米数:",       misc_p_up_l,     ___BYTES,  ___X+10,  ___Y+124, 164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("线2 下米:",       misc_p_dn_l2,    ___BYTES,  ___X+10,  ___Y+162, 164, 28, ___FONT, ___STYLE, "后排线下线米数, 仅用于零压型")
INPUT_DIALOG_ITEM("线2 上米:",       misc_p_up_l2,    ___BYTES,  ___X+10,  ___Y+200, 164, 28, ___FONT, ___STYLE, "后排线上线米数, 仅用于零压型")
INPUT_DIALOG_ITEM("刀下延时:  ",     misc_k_dn_dly,   ___BYTES,  ___X+194, ___Y+10,  180, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("刀上延时:  ",     misc_k_up_dly,   ___BYTES,  ___X+194, ___Y+48,  180, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("线下延时:  ",     misc_l_dn_dly,   ___BYTES,  ___X+194, ___Y+86,  180, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("线上延时:  ",     misc_l_up_dly,   ___BYTES,  ___X+194, ___Y+124, 180, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("线2下延时: ",     misc_l2_dn_dly,  ___BYTES,  ___X+194, ___Y+162, 180, 28, ___FONT, ___STYLE, "后排线下线延时, 仅用于零压型")
INPUT_DIALOG_ITEM("线2上延时: ",     misc_l2_up_dly,  ___BYTES,  ___X+194, ___Y+200, 180, 28, ___FONT, ___STYLE, "后排线上线延时, 仅用于零压型")
#undef  ___X
#undef  ___Y
#define ___X  454
#define ___Y  458
INPUT_DIALOG_GROUP("", misc_p_group, ___X, ___Y, 390, 48, 0, 0)
INPUT_DIALOG_ITEM("YX-BIG  :",       misc_p_yx_big,   ___BYTES,  ___X+10,  ___Y+10,  164, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_ITEM("YX-SMALL : ",     misc_p_yx_small, ___BYTES,  ___X+194, ___Y+10,  180, 28, ___FONT, ___STYLE, "")
INPUT_DIALOG_SET(misc_param_dialog, "其它参数", &icon, 75, 91, 867, 602, 0, 236, 1, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE)
DECLARE_INPUT_DIALOG_ENDED(misc_param_dialog);

gui_widget * init_misc_param_dlg(void)
{
    gui_widget * widget;
    char * s;
    extern char ___bkcolor_other_dialog[];

    misc_param_dialog.bkcolor = atoi(___bkcolor_other_dialog);

    input_dialog_preset_title(&misc_param_dialog, pick_string(NULL, "Misc Parameters"));

    #define ____do_set(id, name_en, comment_en) \
                  do { \
                    s = pick_string(NULL, name_en); \
                    if(s) \
                        input_dialog_preset_item_name(&misc_param_dialog, id, s); \
                    s = pick_string(NULL, comment_en); \
                    if(s) \
                        input_dialog_preset_item_comment(&misc_param_dialog, id, s); \
                  } while(0)

    ____do_set(misc_p_width,    "MaxWidth:",      NULL);
    ____do_set(misc_p_trim,     "Trim:    ",      "");
    ____do_set(misc_p_prep,     "PrePress:",      "");
    ____do_set(misc_p_a_std,    "AutoStrd:",      "Auto Generate Standard Position");
    ____do_set(misc_p_lmax,     "MaxLeftOffs :",  "Max Left Offset");
    ____do_set(misc_p_rmax,     "MaxRightOffs:",  "Max Right Offset");
    ____do_set(misc_p_offset,   "Whole-Offset:",  "");

    ____do_set(misc_p_u_fan,    "Fan Unit:",      "");
    ____do_set(misc_p_u_p1,     "P1  Unit:",      "");
    ____do_set(misc_p_u_p2,     "P2  Unit:",      "Used in Dual only");
    ____do_set(misc_p_spg,      "SPG:     ",      "");
    ____do_set(misc_p_ad_f,     "Fan +/- Val: ",  "");
    ____do_set(misc_p_ad_p1,    "P1  +/- Val: ",  "");
    ____do_set(misc_p_ad_p2,    "P2  +/- Val: ",  "Used in Dual only");
    ____do_set(misc_p_ad_big,   "Big +/- Val: ",  "");

    ____do_set(misc_p_big_k,    "BigFix K:",      "");
    ____do_set(misc_p_sml_k,    "SmlFix K:",      "");
    ____do_set(misc_p_big_l,    "BigFix L:",      "");
    ____do_set(misc_p_sml_l,    "SmlFix L:",      "");
    ____do_set(misc_p_big_f,    "BigFix Fan:",    "");
    ____do_set(misc_p_sml_f,    "SmlFix Fan:",    "");
    ____do_set(misc_p_big_p,    "BigFix Prs:",    "");
    ____do_set(misc_p_sml_p,    "SmlFix Prs:",    "");

    ____do_set(misc_p_dn_k,     "K Dn  in:",      "");
    ____do_set(misc_p_up_k,     "K Up  in:",      "");
    ____do_set(misc_p_dn_l,     "L Dn  in:",      "");
    ____do_set(misc_p_up_l,     "L Up  in:",      "");
    ____do_set(misc_p_dn_l2,    "L2 Dn in:",      "");
    ____do_set(misc_p_up_l2,    "L2 Up in:",      "");
    ____do_set(misc_k_dn_dly,   "K  Dn Dlay:",    "");
    ____do_set(misc_k_up_dly,   "K  Up Dlay:",    "");
    ____do_set(misc_l_dn_dly,   "L  Dn Dlay:",    "");
    ____do_set(misc_l_up_dly,   "L  Up Dlay:",    "");
    ____do_set(misc_l2_dn_dly,  "L2 Dn Dlay:",    "");
    ____do_set(misc_l2_up_dly,  "L2 Up Dlay:",    "");

    ____do_set(misc_p_adj_p1,   "P1 Adjst:",      "");
    ____do_set(misc_p_adj_p2,   "P2 Adjst:",      "");
    ____do_set(misc_p_adj_fan,  "F  Adjst:",      "");

    ____do_set(misc_p_spdscale, "SpdScale:    ",  "");

    #undef ____do_set

    widget = input_dialog_initialize(&misc_param_dialog);
    input_dialog_set_buttons_caption(&misc_param_dialog, 
                                     pick_string("确认[F10]", "OK.[F10]"), 
                                     pick_string("取消[ESC]", "No.[ESC]"));
    input_dialog_set_ok_comment(&misc_param_dialog, 
                                     pick_string("确认, 请按F10或回车键", "Press F10/Enter to Confirm"));
    input_dialog_set_cancel_comment(&misc_param_dialog, 
                                     pick_string("取消, 请按ESC或回车键", "Press ESC/Enter to Cancel"));
    return widget;
}

static void ____alert(char * s)
{
    input_dialog_alert(&misc_param_dialog, s, COLOR_WARNING_236);
}

void misc_param_prepare(int id, char *buf, void * data, INT16U opt)
{
    slc_descriptor_t * slc;
    INT16U  chg_flag;
    struct slc_config_s * __config;

    FamesAssert(buf);
    FamesAssert(data);
    if(!buf || !data)
        return;

    opt = opt;

    __config = (struct slc_config_s *)data;

    lock_kernel();
    slc = &(__config->slc[__current_slc_to_setup]);
    unlock_kernel();

    chg_flag = CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_SIG|CHG_OPT_FRC|0x81;
    
    switch(id){
        case misc_p_width:
            INT16toSTR(buf, slc->hw_width/10, CHG_OPT_END|CHG_OPT_DEC);
            break;
        case misc_p_trim:
            if(slc->slc_flag & SLC_FLAG_TRIM)
                buf[0] = '1';
            else
                buf[0] = '0';
            buf[1] = 0;
            break;
        case misc_p_prep:
            if(slc->slc_flag & SLC_FLAG_PREP)
                buf[0] = '1';
            else
                buf[0] = '0';
            buf[1] = 0;
            break;
        case misc_p_a_std:
            if(slc->slc_flag & SLC_FLAG_ASTD)
                buf[0] = '1';
            else
                buf[0] = '0';
            buf[1] = 0;
            break;
        case misc_p_lmax:
            INT16toSTR(buf, slc->max_left_offs, chg_flag);
            break;
        case misc_p_rmax:
            INT16toSTR(buf, slc->max_right_offs, chg_flag);
            break;
        case misc_p_offset:
            INT16toSTR(buf, slc->hw_offset, chg_flag);
            break;
        case misc_p_u_fan:
            INT16toSTR(buf, slc->unit.fan_unit, chg_flag);
            break;
        case misc_p_u_p1:
            INT16toSTR(buf, slc->unit.press_1_unit, chg_flag);
            break;
        case misc_p_u_p2:
            INT16toSTR(buf, slc->unit.press_2_unit, chg_flag);
            break;
        case misc_p_spg:
            INT16toSTR(buf, slc->slc_spg, CHG_OPT_END|CHG_OPT_DEC);
            break;
        case misc_p_ad_f:
            INT16toSTR(buf, slc->fix_set.fan_fix_set, chg_flag);
            break;
        case misc_p_ad_p1:
            INT16toSTR(buf, slc->fix_set.press_1_fix_set, chg_flag);
            break;
        case misc_p_ad_p2:
            INT16toSTR(buf, slc->fix_set.press_2_fix_set, chg_flag);
            break;
        case misc_p_ad_big:
            INT16toSTR(buf, slc->fix_set.big_start_set, chg_flag);
            break;
        case misc_p_big_k:
            INT16toSTR(buf, slc->fix_set.k_big_fix, chg_flag);
            break;
        case misc_p_sml_k:
            INT16toSTR(buf, slc->fix_set.k_small_fix, chg_flag);
            break;
        case misc_p_big_l:
            INT16toSTR(buf, slc->fix_set.l_big_fix, chg_flag);
            break;
        case misc_p_sml_l:
            INT16toSTR(buf, slc->fix_set.l_small_fix, chg_flag);
            break;
        case misc_p_big_f:
            INT16toSTR(buf, slc->fix_set.fan_big_fix, chg_flag);
            break;
        case misc_p_sml_f:
            INT16toSTR(buf, slc->fix_set.fan_small_fix, chg_flag);
            break;
        case misc_p_big_p:
            INT16toSTR(buf, slc->fix_set.press_big_fix, chg_flag);
            break;
        case misc_p_sml_p:
            INT16toSTR(buf, slc->fix_set.press_small_fix, chg_flag);
            break;
        case misc_p_dn_k:
            INT16toSTR(buf, slc->kl_ctrl.k_down_m, chg_flag);
            break;
        case misc_p_up_k:
            INT16toSTR(buf, slc->kl_ctrl.k_up_m, chg_flag);
            break;
        case misc_p_dn_l:
            INT16toSTR(buf, slc->kl_ctrl.l_down_m, chg_flag);
            break;
        case misc_p_up_l:
            INT16toSTR(buf, slc->kl_ctrl.l_up_m, chg_flag);
            break;
        case misc_p_dn_l2:
            INT16toSTR(buf, slc->kl_ctrl.l2_down_m, chg_flag);
            break;
        case misc_p_up_l2:
            INT16toSTR(buf, slc->kl_ctrl.l2_up_m, chg_flag);
            break;
        case misc_k_dn_dly:
            INT16toSTR(buf, slc->kl_ctrl.k_down_delay, chg_flag);
            break;
        case misc_k_up_dly:
            INT16toSTR(buf, slc->kl_ctrl.k_up_delay, chg_flag);
            break;
        case misc_l_dn_dly:
            INT16toSTR(buf, slc->kl_ctrl.l_down_delay, chg_flag);
            break;
        case misc_l_up_dly:
            INT16toSTR(buf, slc->kl_ctrl.l_up_delay, chg_flag);
            break;
        case misc_l2_dn_dly:
            INT16toSTR(buf, slc->kl_ctrl.l2_down_delay, chg_flag);
            break;
        case misc_l2_up_dly:
            INT16toSTR(buf, slc->kl_ctrl.l2_up_delay, chg_flag);
            break;
        case misc_p_adj_p1:
            INT16toSTR(buf, slc->kl_adjust.press_1_location, chg_flag);
            break;
        case misc_p_adj_p2:
            INT16toSTR(buf, slc->kl_adjust.press_2_location, chg_flag);
            break;
        case misc_p_adj_fan:
            INT16toSTR(buf, slc->kl_adjust.fan_location, chg_flag);
            break;
        case misc_p_yx_big:
            INT16toSTR(buf, slc->misc.yx_big, CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_SIG);
            break;
        case misc_p_yx_small:
            INT16toSTR(buf, slc->misc.yx_small, CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_SIG);
            break;
        case misc_p_spdscale:
            INT16toSTR(buf, slc->speed_scale, CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_SIG);
            break;            
        default:
            sprintf(buf, "--/--");
            break;
    } /* switch(id) */

    return;
}


/***************************************************************************************
** 返回值: 0=内容无效, 1=内容有效, -1=所有内容须刷新
*/
int misc_param_finish(int id, char *buf, void * data, KEYCODE key)
{
    slc_descriptor_t * slc;
    INT16U  chg_flag;
    struct slc_config_s * __config;
    int  temp;

    FamesAssert(buf);
    FamesAssert(data);
    if(!buf || !data)
        return 1; /* 如果缓冲无效, 那么就认为其内容总是有效的 */

    __config = (struct slc_config_s *)data;

    lock_kernel();
    slc = &(__config->slc[__current_slc_to_setup]);
    unlock_kernel();

    chg_flag = CHG_OPT_DEC|CHG_OPT_FRC|0x81;

    temp = STRtoINT16(buf, chg_flag);
    
    #define ____write(addr, value) \
                do { \
                    INT32S tmp32; \
                    if(key != F10) \
                        break; \
                    tmp32 = (INT32S)(value); \
                    ___slc_plc_rw((__current_slc_to_setup+1), FATEK_PLC_WRITE_DR, addr, &tmp32, 1); \
                } while(0)

    switch(id){
        case misc_p_width:
            temp = STRtoINT16(buf, CHG_OPT_DEC);
            if(temp < 1200 || temp > 2800){
                ____alert("机械幅宽的范围是: [1200, 2800]");
                return 0;
            } else {
                slc->hw_width = temp*10;
            }
            break;
        case misc_p_trim:
            if(atoi(buf)){
                slc->slc_flag |= SLC_FLAG_TRIM;
            } else {
                slc->slc_flag &= ~SLC_FLAG_TRIM;
            }
            break;
        case misc_p_prep:
            if(atoi(buf)){
                slc->slc_flag |= SLC_FLAG_PREP;
            } else {
                slc->slc_flag &= ~SLC_FLAG_PREP;
            }
            break;
        case misc_p_a_std:
            if(atoi(buf)){
                slc->slc_flag |= SLC_FLAG_ASTD;
            } else {
                slc->slc_flag &= ~SLC_FLAG_ASTD;
            }
            break;
        case misc_p_lmax:
            slc->max_left_offs = temp;
            break;
        case misc_p_rmax:
            slc->max_right_offs = temp;
            break;
        case misc_p_offset:
            slc->hw_offset = temp;
            break;
        case misc_p_u_fan:
            slc->unit.fan_unit = temp;
            ____write(PLC_ADDR_UNIT_FAN, temp);
            break;
        case misc_p_u_p1:
            slc->unit.press_1_unit = temp;
            ____write(PLC_ADDR_UNIT_P1, temp);
            break;
        case misc_p_u_p2:
            slc->unit.press_2_unit = temp;
            ____write(PLC_ADDR_UNIT_P2, temp);
            break;
        case misc_p_spg:
            temp = STRtoINT16(buf, CHG_OPT_DEC);
            slc->slc_spg = temp;
            ____write(PLC_ADDR_SPG, temp);
            break;
        case misc_p_ad_f:
            slc->fix_set.fan_fix_set = temp;
            ____write(PLC_ADDR_FIX_FAN, temp);
            break;
        case misc_p_ad_p1:
            slc->fix_set.press_1_fix_set = temp;
            ____write(PLC_ADDR_FIX_P1, temp);
            break;
        case misc_p_ad_p2:
            slc->fix_set.press_2_fix_set = temp;
            ____write(PLC_ADDR_FIX_P2, temp);
            break;
        case misc_p_ad_big:
            slc->fix_set.big_start_set = temp;
            ____write(PLC_ADDR_FIX_BIG, temp);
            break;
        case misc_p_big_k:
            slc->fix_set.k_big_fix = temp;
            ____write(PLC_ADDR_K_BIG_FIX, temp);
            break;
        case misc_p_sml_k:
            slc->fix_set.k_small_fix = temp;
            ____write(PLC_ADDR_K_SML_FIX, temp);
            break;
        case misc_p_big_l:
            slc->fix_set.l_big_fix = temp;
            ____write(PLC_ADDR_L_BIG_FIX, temp);
            break;
        case misc_p_sml_l:
            slc->fix_set.l_small_fix = temp;
            ____write(PLC_ADDR_L_SML_FIX, temp);
            break;
        case misc_p_big_f:
            slc->fix_set.fan_big_fix = temp;
            ____write(PLC_ADDR_F_BIG_FIX, temp);
            break;
        case misc_p_sml_f:
            slc->fix_set.fan_small_fix = temp;
            ____write(PLC_ADDR_F_SML_FIX, temp);
            break;
        case misc_p_big_p:
            slc->fix_set.press_big_fix = temp;
            ____write(PLC_ADDR_P_BIG_FIX, temp);
            break;
        case misc_p_sml_p:
            slc->fix_set.press_small_fix = temp;
            ____write(PLC_ADDR_P_SML_FIX, temp);
            break;
        case misc_p_dn_k:
            slc->kl_ctrl.k_down_m = temp;
            ____write(PLC_ADDR_CTRL_K_DN, temp);
            break;
        case misc_p_up_k:
            slc->kl_ctrl.k_up_m = temp;
            ____write(PLC_ADDR_CTRL_K_UP, temp);
            break;
        case misc_p_dn_l:
            slc->kl_ctrl.l_down_m = temp;
            ____write(PLC_ADDR_CTRL_L_DN, temp);
            break;
        case misc_p_up_l:
            slc->kl_ctrl.l_up_m = temp;
            ____write(PLC_ADDR_CTRL_L_UP, temp);
            break;
        case misc_p_dn_l2:
            slc->kl_ctrl.l2_down_m = temp;
            ____write(PLC_ADDR_CTRL_L2_DN, temp);
            break;
        case misc_p_up_l2:
            slc->kl_ctrl.l2_up_m = temp;
            ____write(PLC_ADDR_CTRL_L2_UP, temp);
            break;
        case misc_k_dn_dly:
            slc->kl_ctrl.k_down_delay = temp;
            ____write(PLC_ADDR_CTRL_K_DN_DLY, temp);
            break;
        case misc_k_up_dly:
            slc->kl_ctrl.k_up_delay = temp;
            ____write(PLC_ADDR_CTRL_K_UP_DLY, temp);
            break;
        case misc_l_dn_dly:
            slc->kl_ctrl.l_down_delay = temp;
            ____write(PLC_ADDR_CTRL_L_DN_DLY, temp);
            break;
        case misc_l_up_dly:
            slc->kl_ctrl.l_up_delay = temp;
            ____write(PLC_ADDR_CTRL_L_UP_DLY, temp);
            break;
        case misc_l2_dn_dly:
            slc->kl_ctrl.l2_down_delay = temp;
            ____write(PLC_ADDR_CTRL_L2_DN_DLY, temp);
            break;
        case misc_l2_up_dly:
            slc->kl_ctrl.l2_up_delay = temp;
            ____write(PLC_ADDR_CTRL_L2_UP_DLY, temp);
            break;
        case misc_p_adj_p1:
            slc->kl_adjust.press_1_location = temp;
            ____write(PLC_ADDR_ADJ_PRS1, temp);
            if(key == 10){
                int tmp = 1;
                ___slc_plc_rw((__current_slc_to_setup+1), FATEK_PLC_WRITE_M, PLC_ADDR_ADJ_P1_CFM, &tmp, 1); \
            }
            break;
        case misc_p_adj_p2:
            slc->kl_adjust.press_2_location = temp;
            ____write(PLC_ADDR_ADJ_PRS2, temp);
            if(key == 10){
                int tmp = 1;
                ___slc_plc_rw((__current_slc_to_setup+1), FATEK_PLC_WRITE_M, PLC_ADDR_ADJ_P2_CFM, &tmp, 1); \
            }
            break;
        case misc_p_adj_fan:
            slc->kl_adjust.fan_location = temp;
            ____write(PLC_ADDR_ADJ_FAN, temp);
            if(key == 10){
                int tmp = 1;
                ___slc_plc_rw((__current_slc_to_setup+1), FATEK_PLC_WRITE_M, PLC_ADDR_ADJ_FAN_CFM, &tmp, 1); \
            }
            break;
        case misc_p_yx_big:
            slc->misc.yx_big = STRtoINT16(buf, CHG_OPT_DEC);
            ____write(PLC_ADDR_YXBIG, temp);
            break;
        case misc_p_yx_small:
            slc->misc.yx_small = STRtoINT16(buf, CHG_OPT_DEC);
            ____write(PLC_ADDR_YXSMALL, temp);
            break;
        case misc_p_spdscale:
            slc->speed_scale= STRtoINT16(buf, CHG_OPT_DEC);
            break;
        default:
            break;
    } /* switch(id) */
    
    return 1;
}

BOOL setup_misc_param(struct slc_config_s * __config)
{
    BOOL retval;
    struct slc_config_s t;
    slc_descriptor_t * slc;
     
    FamesAssert(param_misc_dlg);
    FamesAssert(__config);
    if(!param_misc_dlg || !__config)
        return fail;

    #define ____read(addr, value) \
                do { \
                    INT32S tmp32; \
                    tmp32 = (INT32S)(value); \
                    ___slc_plc_rw((__current_slc_to_setup+1), FATEK_PLC_READ_DR, addr, &tmp32, 1); \
                    value = (int)tmp32; \
                } while(0)

    t = *__config;
    slc = &(t.slc[__current_slc_to_setup]);

    ____read(PLC_ADDR_UNIT_FAN,         slc->unit.fan_unit);
    ____read(PLC_ADDR_UNIT_P1,          slc->unit.press_1_unit);
    ____read(PLC_ADDR_UNIT_P2,          slc->unit.press_2_unit);
    ____read(PLC_ADDR_SPG,              slc->slc_spg);
    ____read(PLC_ADDR_FIX_FAN,          slc->fix_set.fan_fix_set);
    ____read(PLC_ADDR_FIX_P1,           slc->fix_set.press_1_fix_set);
    ____read(PLC_ADDR_FIX_P2,           slc->fix_set.press_2_fix_set);
    ____read(PLC_ADDR_FIX_BIG,          slc->fix_set.big_start_set);
    ____read(PLC_ADDR_K_BIG_FIX,        slc->fix_set.k_big_fix);
    ____read(PLC_ADDR_K_SML_FIX,        slc->fix_set.k_small_fix);
    ____read(PLC_ADDR_L_BIG_FIX,        slc->fix_set.l_big_fix);
    ____read(PLC_ADDR_L_SML_FIX,        slc->fix_set.l_small_fix);
    ____read(PLC_ADDR_F_BIG_FIX,        slc->fix_set.fan_big_fix);
    ____read(PLC_ADDR_F_SML_FIX,        slc->fix_set.fan_small_fix);
    ____read(PLC_ADDR_P_BIG_FIX,        slc->fix_set.press_big_fix);
    ____read(PLC_ADDR_P_SML_FIX,        slc->fix_set.press_small_fix);
    ____read(PLC_ADDR_CTRL_K_DN,        slc->kl_ctrl.k_down_m);
    ____read(PLC_ADDR_CTRL_K_UP,        slc->kl_ctrl.k_up_m);
    ____read(PLC_ADDR_CTRL_L_DN,        slc->kl_ctrl.l_down_m);
    ____read(PLC_ADDR_CTRL_L_UP,        slc->kl_ctrl.l_up_m);
    ____read(PLC_ADDR_CTRL_L2_DN,       slc->kl_ctrl.l2_down_m);
    ____read(PLC_ADDR_CTRL_L2_UP,       slc->kl_ctrl.l2_up_m);
    ____read(PLC_ADDR_CTRL_K_DN_DLY,    slc->kl_ctrl.k_down_delay);
    ____read(PLC_ADDR_CTRL_K_UP_DLY,    slc->kl_ctrl.k_up_delay);
    ____read(PLC_ADDR_CTRL_L_DN_DLY,    slc->kl_ctrl.l_down_delay);
    ____read(PLC_ADDR_CTRL_L_UP_DLY,    slc->kl_ctrl.l_up_delay);
    ____read(PLC_ADDR_CTRL_L2_DN_DLY,   slc->kl_ctrl.l2_down_delay);
    ____read(PLC_ADDR_CTRL_L2_UP_DLY,   slc->kl_ctrl.l2_up_delay);
    ____read(PLC_ADDR_YXBIG,            slc->misc.yx_big);
    ____read(PLC_ADDR_YXSMALL,          slc->misc.yx_small);

    gui_show_widget(param_misc_dlg);
    
    for(;;){
        retval = input_dialog_method(&misc_param_dialog, misc_param_prepare, misc_param_finish, (void *)&t, 1);
        if(retval){
            *__config = t;
            lock_kernel();
            copy_to_config(&t);
            save_config();
            unlock_kernel();
            input_dialog_alert(&misc_param_dialog, pick_string("保存成功!", "Saved successfully"), 222);
        } else {
            break;
        }
    }
    gui_hide_widget(param_misc_dlg);
    
    return retval;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      参数画面下的操作
 * 
**---------------------------------------------------------------------------------------*/
void enter_slc_param_setup(int machine)
{
    KEYCODE key;
    slc_descriptor_t * slc;
    char ___s[64];

    FamesAssert(__kl_config);
    FamesAssert(machine == 1 || machine == 2);

    machine--;

    if(!(machine == 0 || machine == 1) || !__kl_config)
        return;
    if(!__slc_param_screen)
        return;

    lock_kernel();
    __current_slc_to_setup = machine;
    *__kl_config = config;
    unlock_kernel();

    slc = &(__kl_config->slc[machine]);
    
    sprintf(___s, pick_string("机%d, %d刀 %d线, %s  ", "SLC-%d, %d/%d, %s  "), 
                (machine+1), slc->k_number, slc->l_number, 
                ((slc->slc_type == SLC_TYPE_DOUBLE)
                    ?pick_string("双排型(零压型)", "Single")
                    :pick_string("单排型(普通型)", "Dual")));
    gui_label_set_text(param_machine_name, ___s);
    sprintf(___s, pick_string("参数设置 - 机%d","Setup - SLC-%d"), (machine+1));
    gui_form_set_caption(__slc_param_screen, ___s);
    gui_set_root_widget(__slc_param_screen);

    slc_read_regress_value(machine+1);
    slc_read_kl_unit_value(machine+1);
    slc_read_kl_fix_value(machine+1);
    slc_read_act_value(machine+1);
    
    slc->kl_adjust = slc->kl_act; /* 拷贝到校正值 */

    for(;;){
        key = setup_kl_param();
        switch(key){
            case ESC:
                return;
            case F1:
                gui_hide_widget(param_kl_view);
                setup_misc_param(__kl_config);
                gui_show_widget(param_kl_view);
                break;
            case UP:
                gui_view_move_up(param_kl_view);
                break;
            case DOWN:
                gui_view_move_down(param_kl_view);
                break;
            case PGUP:
                gui_view_page_up(param_kl_view);
                break;
            case PGDN:
                gui_view_page_down(param_kl_view);
                break;
            case CTRL_HOME:
                gui_view_goto_top(param_kl_view);
                break;
            case CTRL_END:
                gui_view_goto_bottom(param_kl_view);
                break;
            case F10:
                lock_kernel();
                copy_to_config(__kl_config);
                save_config();
                unlock_kernel();
                break;
            default:
                break;
        }
    }
}


/*=========================================================================================
 * 
 * 本文件结束: slc/slc_para.c
 * 
**=======================================================================================*/



