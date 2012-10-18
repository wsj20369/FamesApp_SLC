/******************************************************************************************
 * 文件:    slc/slc_mntr.c
 *
 * 描述:    刀线位置监控
 *
 * 作者:    Jun
******************************************************************************************/
#define  SLC_MONITOR_C
#include <FamesOS.h>
#include "common.h"


extern struct slc_config_s config;

#define  STATE_COLOR    COLOR_YELLOW
#define  STATE_BKCOLOR  2

/*-----------------------------------------------------------------------------------------
 * 
 *      正在监控的SLC序号
 * 
**---------------------------------------------------------------------------------------*/
static int __current_slc_to_monitor = 0; /* 0/1 */

/*-----------------------------------------------------------------------------------------
 *          
 *      监控画面中的控件及其它定义
 * 
**---------------------------------------------------------------------------------------*/
gui_widget * slc_mntr_screen  = NULL;    /* 监控画面的主控件      */
gui_widget * slc_mntr_status_bar = NULL; /* 监控画面的状态条      */
gui_widget * slc_mntr_cpu_status = NULL; /* 监控画面下的CPU使用率 */
gui_widget * kl_animation     = NULL;    /* 刀线移动的监控        */
gui_widget * wheel_position   = NULL;    /* 压线位置的显示        */
gui_widget * knife_position   = NULL;    /* 裁刀位置的显示        */

gui_widget * mntr_speed_label = NULL;    /* 车速显示              */

gui_widget * knife_state_up   = NULL;    /* 刀上状态              */
gui_widget * knife_state_dn   = NULL;    /* 刀下状态              */
gui_widget * wheel_state_up   = NULL;    /* 线上状态              */
gui_widget * wheel_state_dn   = NULL;    /* 线下状态              */

gui_widget * slc_mntr_start   = NULL;    /* 启动                  */
gui_widget * slc_mntr_fixed   = NULL;    /* 定位                  */
gui_widget * slc_mntr_mannl   = NULL;    /* 手调                  */
gui_widget * slc_mntr_o_chg   = NULL;    /* 换单                  */
gui_widget * slc_mntr_stopd   = NULL;    /* 停止                  */
gui_widget * __states_group   = NULL;    /* 上面各状态的组控件    */

gui_widget * fine_tune_form   = NULL;    /* 微调画面的窗体FORM    */
gui_widget * fine_tune_text   = NULL;    /* 微调提示文本          */
gui_widget * fine_tune_value  = NULL;    /* 微调数值文件框        */

extern BMPINFO  icon;                          /* 图标                  */

BOOL init_knife_position_private(int knives, int first_call);
BOOL init_wheel_position_private(int wheels, int slc_type, int first_call);
int  kl_animation_initialize(void);
BOOL kl_animation_init_private(gui_widget * animation);
BOOL refresh_state_for_kl_monitor_initialize(void);


static COLOR  ANIMATION_COLOR;    /* 刀线监控前景色 */
static COLOR  ANIMATION_BKCOLOR;  /* 刀线监控背景色 */
static COLOR  KL_BORDER_COLOR;    /* 刀线图形边框色 */


/*-----------------------------------------------------------------------------------------
 *          
 *      监控画面的定义(或初始化)
 * 
**---------------------------------------------------------------------------------------*/
gui_widget * init_slc_monitor_screen(void)
{
    int x, y, width, height;
    int __id;
    char ___s[32];
    COLOR color, bkcolor;

    sprintf(___s, "71");
    load_string(___s, sizeof(___s), "ANIMATION_COLOR");  /* 加载颜色 */
    ANIMATION_COLOR = (COLOR)atoi(___s);
    sprintf(___s, "70");
    load_string(___s, sizeof(___s), "ANIMATION_BKCOLOR");
    ANIMATION_BKCOLOR = (COLOR)atoi(___s);
    sprintf(___s, "71");
    load_string(___s, sizeof(___s), "KL_BORDER_COLOR");
    KL_BORDER_COLOR =  (COLOR)atoi(___s);
    
    /* 主界面       */
    slc_mntr_screen = gui_create_widget(GUI_WIDGET_FORM, 1, 1, 1021, 765, 0, 0, font16, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE);
    if(!slc_mntr_screen)
        goto some_error;
    gui_widget_link(NULL, slc_mntr_screen);      /* 设置主界面背景 */
    gui_form_init_private(slc_mntr_screen, 128);
    gui_form_set_icon(slc_mntr_screen, &icon);
    
    /* 工具条       */
    slc_mntr_status_bar = gui_create_widget(GUI_WIDGET_LABEL, 7, 725, 872, 30, 0, 0, 1, LABEL_STYLE_CLIENT_BDR);
    if(!slc_mntr_status_bar)
        goto some_error;
    gui_widget_link(slc_mntr_screen, slc_mntr_status_bar);
    gui_label_init_private(slc_mntr_status_bar, 100);
    gui_label_set_text(slc_mntr_status_bar, pick_string(">>>F1 微调     >>>ESC 返回", ">>>F1 Fine-Tune     >>>ESC Return"));

    /* CPU使用率    */
    slc_mntr_cpu_status = gui_create_widget(GUI_WIDGET_LABEL, 880, 725, 132, 30, 0, 0, 1, LABEL_STYLE_CLIENT_BDR|LABEL_ALIGN_CENTER);
    if(!slc_mntr_cpu_status)
        goto some_error;
    gui_widget_link(slc_mntr_screen, slc_mntr_cpu_status);
    gui_label_init_private(slc_mntr_cpu_status, 32);
    gui_label_set_text(slc_mntr_cpu_status, "");

    /* 刀线图标显示 */
    x = 9;
    y = 36;
    width = 1000;
    height = 678;
    __id = kl_animation_initialize();
    if(!__id){
        sys_print("kl_animation_initialize() failed\n");
        getch();
        quit();
    }
    kl_animation = gui_create_widget(__id, x, y, width, height, ANIMATION_COLOR, ANIMATION_BKCOLOR, 1, PREVIEW_STYLE_CLIENT_BDR|PREVIEW_STYLE_MODAL_FRAME);
    if(!kl_animation)
        goto some_error;
    gui_widget_link(slc_mntr_screen, kl_animation);
    kl_animation_init_private(kl_animation);

    /* 线位置的显示 */
    #define __wheel_position_widget_width  988
    wheel_position = gui_create_widget(GUI_WIDGET_VIEW, 6, 7, __wheel_position_widget_width, 139, COLOR_YELLOW, 37, 1, 0xB0);
    if(!wheel_position)
        goto some_error;
    gui_widget_link(kl_animation, wheel_position);
    if(!init_wheel_position_private(SLC_L_MAX_NR, SLC_TYPE_DOUBLE, 1))
        goto some_error;

    /* 刀位置的显示 */
    #define __knife_position_widget_width  720
    knife_position = gui_create_widget(GUI_WIDGET_VIEW, 6, 531, __knife_position_widget_width, 139, COLOR_YELLOW, 37, 1, 0xB0);
    if(!knife_position)
        goto some_error;
    gui_widget_link(kl_animation, knife_position);
    if(!init_knife_position_private(SLC_K_MAX_NR, 1))
        goto some_error;

    /* 刀线上下状态 */
    x = 934;
    width = 56;
    knife_state_up = gui_create_widget(GUI_WIDGET_BUTTON, x, 397, width, 36, 0, 0, font24, 0);
    knife_state_dn = gui_create_widget(GUI_WIDGET_BUTTON, x, 437, width, 36, 0, 0, font24, 0);
    wheel_state_up = gui_create_widget(GUI_WIDGET_BUTTON, x, 196, width, 36, 0, 0, font24, 0);
    wheel_state_dn = gui_create_widget(GUI_WIDGET_BUTTON, x, 236, width, 36, 0, 0, font24, 0);
    if(!knife_state_up || !knife_state_dn ||
       !wheel_state_up || !wheel_state_dn)
       goto some_error;
    gui_widget_link(kl_animation, knife_state_up);
    gui_widget_link(kl_animation, knife_state_dn);
    gui_widget_link(kl_animation, wheel_state_up);
    gui_widget_link(kl_animation, wheel_state_dn);
    gui_button_init_private(knife_state_up, 32);
    gui_button_init_private(knife_state_dn, 32);
    gui_button_init_private(wheel_state_up, 32);
    gui_button_init_private(wheel_state_dn, 32);
    gui_button_set_caption(knife_state_up, pick_string("刀上", "K UP"));
    gui_button_set_caption(knife_state_dn, pick_string("刀下", "K DN"));
    gui_button_set_caption(wheel_state_up, pick_string("线上", "L UP"));
    gui_button_set_caption(wheel_state_dn, pick_string("线下", "L DN"));

    /* 其它状态 */
    x = __knife_position_widget_width+9;
    y = 531;
    width = 266;
    height = 139;
    __states_group = gui_create_widget(GUI_WIDGET_LABEL, x, y, width, height, 0, 0, 0, LABEL_STYLE_CLIENT_BDR);
    if(!__states_group)
        goto some_error;
    gui_widget_link(kl_animation, __states_group);
    gui_label_init_private(__states_group, 8);
    x = 5;
    y = 5;
    width  -= 7;
    width  /= 2;
    height -= 15;
    height /= 3;
    slc_mntr_start = gui_create_widget(GUI_WIDGET_BUTTON, x,       y,  width-2,    height, STATE_COLOR, STATE_BKCOLOR, font24, 0);
    slc_mntr_fixed = gui_create_widget(GUI_WIDGET_BUTTON, x+width, y,  width-2,    height, STATE_COLOR, STATE_BKCOLOR, font24, 0);
    y += (height + 2);
    slc_mntr_o_chg = gui_create_widget(GUI_WIDGET_BUTTON, x,       y,  width-2,    height, STATE_COLOR, STATE_BKCOLOR, font24, 0);
    slc_mntr_mannl = gui_create_widget(GUI_WIDGET_BUTTON, x+width, y,  width-2,    height, STATE_COLOR, STATE_BKCOLOR, font24, 0);
    y += (height + 2);
    slc_mntr_stopd = gui_create_widget(GUI_WIDGET_BUTTON, x,       y, (width*2-2), height, STATE_COLOR, STATE_BKCOLOR, font24, 0);
    if(!slc_mntr_start || !slc_mntr_fixed ||
       !slc_mntr_o_chg || !slc_mntr_mannl ||
       !slc_mntr_stopd)
       goto some_error;
    gui_widget_link(__states_group, slc_mntr_start);
    gui_widget_link(__states_group, slc_mntr_fixed);
    gui_widget_link(__states_group, slc_mntr_o_chg);
    gui_widget_link(__states_group, slc_mntr_mannl);
    gui_widget_link(__states_group, slc_mntr_stopd);
    gui_button_init_private(slc_mntr_start, 32);
    gui_button_init_private(slc_mntr_fixed, 32);
    gui_button_init_private(slc_mntr_mannl, 32);
    gui_button_init_private(slc_mntr_o_chg, 32);
    gui_button_init_private(slc_mntr_stopd, 32);
    gui_button_set_caption(slc_mntr_start, pick_string("启 动", "Start"));
    gui_button_set_caption(slc_mntr_fixed, pick_string("定 位", "Fixed"));
    gui_button_set_caption(slc_mntr_o_chg, pick_string("换 单", "OrdChg"));
    gui_button_set_caption(slc_mntr_mannl, pick_string("手 调", "Regulate"));
    gui_button_set_caption(slc_mntr_stopd, pick_string("停   止", "== Major Stop =="));

    /* 状态刷新 */
    if(!refresh_state_for_kl_monitor_initialize())
        goto some_error;

    /* 车速显示 */
    mntr_speed_label = gui_create_widget(GUI_WIDGET_LABEL, 144, 160, 132, 30, 0, 0, 1, LABEL_STYLE_CLIENT_BDR);
    if(!mntr_speed_label)
        goto some_error;
    gui_widget_link(kl_animation, mntr_speed_label);
    gui_label_init_private(mntr_speed_label, 32);
    gui_label_set_text(mntr_speed_label, "");

    /* 微调画面 */
    x = 230;
    y = 315;
    width = 512;
    height = 80;
    color = 0;
    bkcolor = 0;
    /* 窗体 */
    fine_tune_form = gui_create_widget(GUI_WIDGET_FORM, x, y, width, height, 0, bkcolor, 0, FORM_STYLE_CLIENT_BDR);
    if(!fine_tune_form)
        goto some_error;
    gui_widget_link(slc_mntr_screen, fine_tune_form);
    gui_hide_widget(fine_tune_form);
    gui_form_init_private(fine_tune_form, 32);
    /* 文本 */
    y = ((height-38)/2);
    fine_tune_text = gui_create_widget(GUI_WIDGET_LABEL, 32, y-1, 256, 36, color, bkcolor, font24, LABEL_STYLE_TRANSPARENT|LABEL_ALIGN_CENTER);
    if(!fine_tune_text)
        goto some_error;
    gui_widget_link(fine_tune_form, fine_tune_text);
    gui_label_init_private(fine_tune_text, 32);
    /* 输入 */
    fine_tune_value = gui_create_widget(GUI_WIDGET_EDIT, 300, y, 128, 36, 0, 0, font24, 0);
    if(!fine_tune_value)
        goto some_error;
    gui_widget_link(fine_tune_form, fine_tune_value);
    gui_edit_init_private(fine_tune_value, 8);


    return slc_mntr_screen;

some_error:
    sys_print("init_slc_monitor_screen(): failed to create widgets for slc monitor screen!\n");
    quit();
    return NULL;
}

/*-----------------------------------------------------------------------------------------
 * 
 *      刀线位置监控控件
 * 
**---------------------------------------------------------------------------------------*/
static int __widget_id_kl_animation = -1; /* 控件ID */

struct __kl_draw_old_state_s {
      int     old_x;
      INT16U  old_state;
};

struct gui_kl_animation_private_s {
    RECT  order_info_area;
    int   order_info_font;
    COLOR order_info_color;
    int   owner_slc;
    int   a_x, a_y, a_x1, a_y1, a_ruler_y, a_K_y, a_L1_y, a_L2_y;    
    int   pixels_per_100mm;
    int   base_for_real_x;
    struct __kl_draw_old_state_s k_old[SLC_K_MAX_NR];
    struct __kl_draw_old_state_s l_old[SLC_L_MAX_NR];
    int    selected_knife, selected_line;
    char   order_info_old[256];
};

typedef struct gui_kl_animation_private_s kl_animation_private;

#define COLOR_UNUSED   COLOR_BLUE
#define COLOR_KL_UP    COLOR_GREEN
#define COLOR_KL_DOWN  COLOR_RED

/*-----------------------------------------------------------------------------------------
 * 函数:    kl_animation_init_private()
 *
 * 描述:    实例初始化, 主要是给私有结构申请内存
**---------------------------------------------------------------------------------------*/
BOOL kl_animation_init_private(gui_widget * animation)
{
    INT08S * buf;
    kl_animation_private * t, * t2;
    int  bytes;
    
    FamesAssert(animation);

    if(!animation)
        return fail;

    FamesAssert(animation->type == __widget_id_kl_animation);

    if(animation->type != __widget_id_kl_animation)
        return fail;

    bytes = (int)sizeof(kl_animation_private);

    buf = (INT08S *)mem_alloc((INT32U)(INT32S)bytes);

    if(buf){
        MEMSET(buf, 0, bytes);
        t = (kl_animation_private *)buf;/*lint !e826*/
        t->order_info_font = animation->font;
        t->order_info_color = animation->color;
        MEMSET((INT08S *)(&t->k_old), 0xFF, sizeof(t->k_old));
        MEMSET((INT08S *)(&t->l_old), 0xFF, sizeof(t->l_old));
        t->selected_knife = -1;
        t->selected_line  = -1;
        if(animation->private_data){
            lock_kernel();
            t2 = animation->private_data;
            animation->private_data = NULL;
            unlock_kernel();
            mem_free(t2);
        }
        lock_kernel();
        animation->private_data = (void *)t;
        unlock_kernel();
        gui_refresh_widget(animation);
        return ok;
    } else {
        return fail;
    }
}

/*-----------------------------------------------------------------------------------------
 * 函数:    kl_animation_set_slc()
 *
 * 描述:    设置分压机句柄
**---------------------------------------------------------------------------------------*/
BOOL kl_animation_set_slc(gui_widget * animation, int slc)
{
    kl_animation_private * t;
    BOOL retval;

    FamesAssert(animation);

    if(!animation)
        return fail;

    retval = fail;
    
    lock_kernel();
    t = (kl_animation_private *)animation->private_data;
    if(t){
        if(slc == 0 || slc == 1){
            t->owner_slc = slc;
            retval = ok;
            gui_refresh_widget(animation);
        }
    }
    unlock_kernel();
    
    return retval;
}

/*-----------------------------------------------------------------------------------------
 * 函数:    ___draw_knife()
 *
 * 描述:    画一把刀的图形
 *
 * 说明:    只用于函数__widget_kl_animation_draw()
**---------------------------------------------------------------------------------------*/
void ___draw_knife(int x, int y, int index, INT16U state, COLOR bkcolor)
{
    COLOR outline_color = KL_BORDER_COLOR;

    static INT08U knife_outline[] = {
        0x00, 0x00, 0x01,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x05,
        0x00, 0x00, 0x05,
        0x00, 0x00, 0x09,
        0x00, 0x00, 0x09,
        0x00, 0x00, 0x11,
        0x00, 0x00, 0x11,
        0x00, 0x00, 0x21,
        0x00, 0x00, 0x21,
        0x00, 0x00, 0x41,
        0x00, 0x00, 0x41,
        0x00, 0x00, 0x81,
        0x00, 0x00, 0x81,
        0xFF, 0xFF, 0x81,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x07,

        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x07,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x07,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        0x80, 0x00, 0x04,
        
        0x80, 0x00, 0x07,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0xFF, 0xFF, 0x81,
        0x00, 0x00, 0x81,
        0x00, 0x00, 0x81,
        0x00, 0x00, 0x41,
        0x00, 0x00, 0x41,
        0x00, 0x00, 0x21,
        0x00, 0x00, 0x21,
        0x00, 0x00, 0x11,
        0x00, 0x00, 0x11,
        0x00, 0x00, 0x09,
        0x00, 0x00, 0x09,
        0x00, 0x00, 0x05,
        0x00, 0x00, 0x05,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x01,
    };
    static INT08U knife_inside[] = {
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x02,
        0x00, 0x00, 0x02,
        0x00, 0x00, 0x06,
        0x00, 0x00, 0x06,
        0x00, 0x00, 0x0E,
        0x00, 0x00, 0x0E,
        0x00, 0x00, 0x1E,
        0x00, 0x00, 0x1E,
        0x00, 0x00, 0x3E,
        0x00, 0x00, 0x3E,
        0x00, 0x00, 0x7E,
        0x00, 0x00, 0x7E,
        0x00, 0x00, 0x7E,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xF8,

        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xF8,
        
        0x7F, 0xFF, 0xF8,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x00, 0x00, 0x7E,
        0x00, 0x00, 0x7E,
        0x00, 0x00, 0x7E,
        0x00, 0x00, 0x3E,
        0x00, 0x00, 0x3E,
        0x00, 0x00, 0x1E,
        0x00, 0x00, 0x1E,
        0x00, 0x00, 0x0E,
        0x00, 0x00, 0x0E,
        0x00, 0x00, 0x06,
        0x00, 0x00, 0x06,
        0x00, 0x00, 0x02,
        0x00, 0x00, 0x02,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
    };
    static INT08U knife_bkground[] = {
        0xFF, 0xFF, 0xFE,
        0xFF, 0xFF, 0xFC,
        0xFF, 0xFF, 0xFC,
        0xFF, 0xFF, 0xF8,
        0xFF, 0xFF, 0xF8,
        0xFF, 0xFF, 0xF0,
        0xFF, 0xFF, 0xF0,
        0xFF, 0xFF, 0xE0,
        0xFF, 0xFF, 0xE0,
        0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0x80,
        0xFF, 0xFF, 0x80,
        0xFF, 0xFF, 0x00,
        0xFF, 0xFF, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,

        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        0x00, 0x00, 0x03,
        
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0xFF, 0xFF, 0x00,
        0xFF, 0xFF, 0x00,
        0xFF, 0xFF, 0x80,
        0xFF, 0xFF, 0x80,
        0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xE0,
        0xFF, 0xFF, 0xE0,
        0xFF, 0xFF, 0xF0,
        0xFF, 0xFF, 0xF0,
        0xFF, 0xFF, 0xF8,
        0xFF, 0xFF, 0xF8,
        0xFF, 0xFF, 0xFC,
        0xFF, 0xFF, 0xFC,
        0xFF, 0xFF, 0xFE,
    };
    COLOR color;
    int i, j, __x, __y;
    char __s[16];

    FamesAssert(index >=0 && index < 32);
    if(index < 0 || index >= 32)
        return;

    if(state & KL_STATE_BLINK){
        color = COLOR_BLINK;
    } else if(state & KL_STATE_SLCT){
        if(state & KL_STATE_DOWN)
            color = COLOR_KL_DOWN;
        else
            color = COLOR_KL_UP;
    } else {
        color = COLOR_UNUSED; /* bkcolor */
    }

    x -= 24;
    y -= 31;
    __x = x;
    __y = y;

    for(i=0,j=0; i<64; i++){
        gdi_draw_h_bitmap(x, y, 8, knife_bkground[j], bkcolor);
        gdi_draw_h_bitmap(x, y, 8, knife_outline[j],  outline_color);
        gdi_draw_h_bitmap(x, y, 8, knife_inside[j++], color);
        x += 8;
        gdi_draw_h_bitmap(x, y, 8, knife_bkground[j], bkcolor);
        gdi_draw_h_bitmap(x, y, 8, knife_outline[j],  outline_color);
        gdi_draw_h_bitmap(x, y, 8, knife_inside[j++], color);
        x += 8;
        gdi_draw_h_bitmap(x, y, 8, knife_bkground[j], bkcolor);
        gdi_draw_h_bitmap(x, y, 8, knife_outline[j],  outline_color);
        gdi_draw_h_bitmap(x, y, 8, knife_inside[j++], color);
        x -= 16;
        y ++;
    }
    
    x = __x;
    y = __y;
    INT16toSTR(__s, index+1, CHG_OPT_DEC|CHG_OPT_END);
    draw_font_ex(x+2, y, 20, 64, __s, outline_color, bkcolor, 1, DRAW_OPT_ALIGN_CENTER|DRAW_OPT_FIL_BG);
}

/*-----------------------------------------------------------------------------------------
 * 函数:    ___draw_wheel()
 *
 * 描述:    画一个压线轮的图形
 *
 * 说明:    只用于函数__widget_kl_animation_draw()
**---------------------------------------------------------------------------------------*/
void ___draw_wheel(int x, int y, int index, INT16U state, COLOR bkcolor)
{
    COLOR outline_color = KL_BORDER_COLOR;

    static INT08U wheel_outline[] = {
        0x00, 0x3C, 0x00,
        0x00, 0x42, 0x00,
        0x00, 0x81, 0x00,
        0x01, 0x00, 0x80,
        0x01, 0x00, 0x80,
        0x01, 0x00, 0x80,
        0x01, 0x00, 0x80,
        0x01, 0x00, 0x80,
        0x02, 0x00, 0x40,
        0x02, 0x00, 0x40,
        0x02, 0x00, 0x40,
        0x02, 0x00, 0x40,
        0x02, 0x00, 0x40,
        0x02, 0x00, 0x40,
        0x02, 0x00, 0x40,
        0xFE, 0x00, 0x7F,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,

        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0xFE, 0x00, 0x7F,
        0x02, 0x00, 0x40,
        0x02, 0x00, 0x40,
        0x02, 0x00, 0x40,
        0x02, 0x00, 0x40,
        0x02, 0x00, 0x40,
        0x02, 0x00, 0x40,
        0x02, 0x00, 0x40,
        0x01, 0x00, 0x80,
        0x01, 0x00, 0x80,
        0x01, 0x00, 0x80,
        0x01, 0x00, 0x80,
        0x01, 0x00, 0x80,
        0x00, 0x81, 0x00,
        0x00, 0x42, 0x00,
        0x00, 0x3C, 0x00,
    };
    static INT08U wheel_inside[] = {
        0x00, 0x00, 0x00,
        0x00, 0x3C, 0x00,
        0x00, 0x7E, 0x00,
        0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,

        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x7F, 0xFF, 0xFE,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x01, 0xFF, 0x80,
        0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00,
        0x00, 0x7E, 0x00,
        0x00, 0x3C, 0x00,
        0x00, 0x00, 0x00,
    };
    static INT08U wheel_bkground[] = {
        0xFF, 0xC3, 0xFF,
        0xFF, 0x81, 0xFF,
        0xFF, 0x00, 0xFF,
        0xFE, 0x00, 0x7F,
        0xFE, 0x00, 0x7F,
        0xFE, 0x00, 0x7F,
        0xFE, 0x00, 0x7F,
        0xFE, 0x00, 0x7F,
        0xFC, 0x00, 0x3F,
        0xFC, 0x00, 0x3F,
        0xFC, 0x00, 0x3F,
        0xFC, 0x00, 0x3F,
        0xFC, 0x00, 0x3F,
        0xFC, 0x00, 0x3F,
        0xFC, 0x00, 0x3F,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,

        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0xFC, 0x00, 0x3F,
        0xFC, 0x00, 0x3F,
        0xFC, 0x00, 0x3F,
        0xFC, 0x00, 0x3F,
        0xFC, 0x00, 0x3F,
        0xFC, 0x00, 0x3F,
        0xFC, 0x00, 0x3F,
        0xFE, 0x00, 0x7F,
        0xFE, 0x00, 0x7F,
        0xFE, 0x00, 0x7F,
        0xFE, 0x00, 0x7F,
        0xFE, 0x00, 0x7F,
        0xFF, 0x00, 0xFF,
        0xFF, 0x81, 0xFF,
        0xFF, 0xC3, 0xFF,
    };
    COLOR color;
    int i, j, __x, __y;
    char __s[16];

    FamesAssert(index >=0 && index < 32);
    if(index < 0 || index >= 32)
        return;

    if(state & KL_STATE_BLINK){
        color = COLOR_BLINK;
    } else if(state & KL_STATE_SLCT){
        if(state & KL_STATE_DOWN)
            color = COLOR_KL_DOWN;
        else
            color = COLOR_KL_UP;
    } else {
        color = COLOR_UNUSED; /* bkcolor */
    }

    x -= 12;
    y -= 31;
    __x = x;
    __y = y;

    for(i=0,j=0; i<64; i++){
        gdi_draw_h_bitmap(x, y, 8, wheel_bkground[j], bkcolor);
        gdi_draw_h_bitmap(x, y, 8, wheel_outline[j],  outline_color);
        gdi_draw_h_bitmap(x, y, 8, wheel_inside[j++], color);
        x += 8;
        gdi_draw_h_bitmap(x, y, 8, wheel_bkground[j], bkcolor);
        gdi_draw_h_bitmap(x, y, 8, wheel_outline[j],  outline_color);
        gdi_draw_h_bitmap(x, y, 8, wheel_inside[j++], color);
        x += 8;
        gdi_draw_h_bitmap(x, y, 8, wheel_bkground[j], bkcolor);
        gdi_draw_h_bitmap(x, y, 8, wheel_outline[j],  outline_color);
        gdi_draw_h_bitmap(x, y, 8, wheel_inside[j++], color);
        x -= 16;
        y ++;
    }
    
    x = __x;
    y = __y;
    INT16toSTR(__s, index+1, CHG_OPT_DEC|CHG_OPT_END);
    draw_font_ex(x+2, y, 20, 64, __s, outline_color, bkcolor, 1, DRAW_OPT_ALIGN_CENTER|DRAW_OPT_FIL_BG);
}

/*-----------------------------------------------------------------------------------------
 * 函数:    ___draw_blank()
 *
 * 描述:    画一个空线的图形
 *
 * 说明:    只用于函数__widget_kl_animation_draw()
**---------------------------------------------------------------------------------------*/
void ___draw_blank(int x, int y, int x1, COLOR bkcolor)
{
    if(x <= x1){
        gdi_draw_box(x, y-31, x1, y-6,  bkcolor);
        gdi_draw_h_line(x, y-5, x1, KL_BORDER_COLOR);
        gdi_draw_box(x, y-4,  x1, y+5,  bkcolor);
        gdi_draw_h_line(x, y+6, x1, KL_BORDER_COLOR);
        gdi_draw_box(x, y+7,  x1, y+32, bkcolor);
    }
}


/*-----------------------------------------------------------------------------------------
 * 函数:    __widget_kl_animation_draw()
 *
 * 描述:    绘图
**---------------------------------------------------------------------------------------*/
void __widget_kl_animation_draw(gui_widget * animation)
{
    int x, y, x1, y1, w, move;
    COLOR bkcolor, color;
    kl_animation_private * t;
    order_struct * order;
    int i, __v;
    int __x, __y, __x1, __y1;
    int a_x, a_y, a_x1, a_y1, a_ruler_y, a_K_y, a_L1_y, a_L2_y;
    RECT * inner_rect, * oi_area;
    char order_info_buf[256];
    slc_descriptor_t * slc;
    extern BMPINFO machine_name[];

    #define PAPER_MAX_WIDTH 3000            /* 监控支持的最大纸宽是3米 */
    #define M_MARGIN_WIDTH  64              /* 机台边缘宽度            */
    #define M_MARGIN_COLOR  17              /* 机台边缘颜色            */
    #define M_TOP_MARGIN    143             /* 机台上边缘高度          */
    #define M_BTM_MARGIN    113             /* 机台下边缘高度          */

    FamesAssert(animation);

    if(!animation)
        return;

    t = (kl_animation_private *)animation->private_data;
    if(!t)
        return;

    FamesAssert(t->owner_slc ==0 || t->owner_slc == 1);
    if(!(t->owner_slc ==0 || t->owner_slc == 1))
        return;

    slc = &(config.slc[t->owner_slc]);

    inner_rect = &animation->inner_rect;
    oi_area    = &t->order_info_area;
    color = animation->color;
    bkcolor = animation->bkcolor;
    if(bkcolor == 0)
        bkcolor = WIDGET_BKCOLOR;
    
    if(animation->flag & GUI_WIDGET_FLAG_REFRESH){
        x  = animation->real_rect.x;
        y  = animation->real_rect.y;
        x1 = animation->real_rect.width + x;
        y1 = animation->real_rect.height + y;
        if(animation->style & SLC_MNTR_STYLE_MODAL_FRAME){
            move = gui_widget_draw_modal_frame(x, y, x1, y1);
            ___gui_widget_xy_move(move);
        } 
        if(animation->style & SLC_MNTR_STYLE_CLIENT_BDR){
            move = gui_widget_draw_client_bdr(x, y, x1, y1);
            ___gui_widget_xy_move(move);
        } 
        if(animation->style & SLC_MNTR_STYLE_BORDER){
            move = gui_widget_draw_static_bdr(x, y, x1, y1);
            ___gui_widget_xy_move(move);
        } 
        if(animation->style & SLC_MNTR_STYLE_SUBSIDE){
            move = gui_widget_draw_subside_bdr(x, y, x1, y1);
            ___gui_widget_xy_move(move);
        } 
        if(!gdi_draw_box(x, y, x1, y1, bkcolor)){
            ;
        }
        __x  = x;
        __y  = y;
        __x1 = x1;
        __y1 = y1;
        if(1){ /* 显示机台边缘 */
            y += M_TOP_MARGIN;
            y1-= M_BTM_MARGIN;
            for(i=M_MARGIN_WIDTH/2; i<=M_MARGIN_WIDTH; i++){
                gdi_draw_h_line((x),    y, (x+i), M_MARGIN_COLOR);
                gdi_draw_h_line((x1-i), y, (x1),  M_MARGIN_COLOR);
                y++;
            }
            i--;
            gdi_draw_box((x),    y, (x+i), (y1-M_MARGIN_WIDTH), M_MARGIN_COLOR);
            gdi_draw_box((x1-i), y, (x1),  (y1-M_MARGIN_WIDTH), M_MARGIN_COLOR);
            __v = (y1-M_MARGIN_WIDTH);
            __v -= y;
            __v -= (machine_name[t->owner_slc].height);
            __v /= 2;
            ShowBmp((x+3), (y+__v)-16, &machine_name[t->owner_slc]);
            y = (y1-M_MARGIN_WIDTH)+1;
            for(i=M_MARGIN_WIDTH; i>=M_MARGIN_WIDTH/2; i--){
                gdi_draw_h_line((x),    y, (x+i), M_MARGIN_COLOR);
                gdi_draw_h_line((x1-i), y, (x1),  M_MARGIN_COLOR);
                y++;
            }
        }
        if(1){ /* 计算位置     */
            a_x  = __x  +  M_MARGIN_WIDTH;
            a_x1 = __x1 -  M_MARGIN_WIDTH;
            a_y  = __y  + (M_MARGIN_WIDTH/2) + M_TOP_MARGIN + 52;
            a_y1 = __y1 - (M_MARGIN_WIDTH  ) - M_BTM_MARGIN - 60;
            if(slc->slc_type == SLC_TYPE_DOUBLE){
                __v = (a_y1-a_y)/3;
                a_L1_y = a_y;
                a_L2_y = a_L1_y + __v;
                a_K_y  = a_y1;
                a_ruler_y = a_K_y - __v;
            } else {
                __v = (a_y1-a_y)/2;
                a_L1_y = a_y;
                a_L2_y = a_L1_y ;
                a_K_y  = a_y1;
                a_ruler_y = a_K_y - __v;
            }
            gui_init_rect(oi_area, __x + (M_MARGIN_WIDTH/2), __y1 - M_MARGIN_WIDTH - M_BTM_MARGIN, 900, 32);
        }
        if(1){ /* 显示机台标尺 */
            char *__s[] = { "-1500", "-1000", "-500", "0", "500", "1000", "1500" };
            int   _y, _y1;
            w  = (a_x1-a_x)-64;
            x  = a_x + 32;
            x += (w %30 )/2;
            __v = (w / 30);
            __x = x;
            x -= 22;
            for(i=0; i<7; i++){
                draw_font_ex(x, (a_ruler_y-25), 50, 8, __s[i], color, bkcolor, animation->font, DRAW_OPT_ALIGN_CENTER);
                x += (__v * 5);
            }
            gdi_draw_h_line(a_x, a_ruler_y  , a_x1, color);
            x  = __x;
            for(i=0; i<=30; i++){ /* 画标尺的小竖线 */  
                if(i%5){
                    _y  = (a_ruler_y-4);
                    _y1 = (a_ruler_y+4);
                } else {
                    _y  = (a_ruler_y-6);
                    _y1 = (a_ruler_y+6);
                }
                gdi_draw_v_line(x, _y, _y1, color);
                x += __v;
            }
            for(; x <= a_x1;){ /* 右边的小竖线 */  
                _y  = (a_ruler_y-4);
                _y1 = (a_ruler_y+4);
                gdi_draw_v_line(x, _y, _y1, color);
                x += __v;
            }
            x  = __x;
            for(; x >= a_x;){  /* 左边的小竖线 */  
                _y  = (a_ruler_y-4);
                _y1 = (a_ruler_y+4);
                gdi_draw_v_line(x, _y, _y1, color);
                x -= __v;
            }
        }
        t->pixels_per_100mm = __v;
        t->base_for_real_x  = __x;
        t->a_x       = a_x;
        t->a_y       = a_y;
        t->a_x1      = a_x1;
        t->a_y1      = a_y1;
        t->a_K_y     = a_K_y;
        t->a_L1_y    = a_L1_y;
        t->a_L2_y    = a_L2_y;
        t->a_ruler_y = a_ruler_y;
        gui_init_rect(inner_rect, __x, __y, (__x1-__x)+1, (__y1-__y)+1);
        MEMSET((INT08S *)(&t->k_old), 0xFF, sizeof(t->k_old));
        MEMSET((INT08S *)(&t->l_old), 0xFF, sizeof(t->l_old));
        t->order_info_old[0] = 0;
    } else { /* if(animation->flag & GUI_WIDGET_FLAG_REFRESH) */
        int blank_flag, blank_x1, blank_x2, real_x;
        int act_value;
        int pixels_per_100mm, base_for_real_x;
        INT16U state;
        INT32S __lvalue;
        x  = inner_rect->x;
        y  = inner_rect->y;
        x1 = inner_rect->width + x;
        y1 = inner_rect->height + y;
        a_x       = t->a_x;
        a_y       = t->a_y;
        a_x1      = t->a_x1;
        a_y1      = t->a_y1;
        a_K_y     = t->a_K_y;
        a_L1_y    = t->a_L1_y;
        a_L2_y    = t->a_L2_y;
        a_ruler_y = t->a_ruler_y;
        pixels_per_100mm = t->pixels_per_100mm;
        base_for_real_x = t->base_for_real_x;
        /* 画压线轮 */
        if(slc->slc_type == SLC_TYPE_DOUBLE){ /* 双排线 */
            /* 前排线 */
            blank_x1 = a_x+1;
            blank_flag = 0;
            for(i=0; i<slc->l_number; i+=2){
                state = KL_STATE_NONE;
                if(slc->kl_set.l_selected[i])
                    state |= KL_STATE_SLCT;
                if(slc->state.l_down)
                    state |= KL_STATE_DOWN;
                if(t->selected_line == i)
                    state |= KL_STATE_BLINK;
                act_value = slc->kl_act.l_location[i];
                __lvalue = 15000L + (INT32S)act_value;
                if(__lvalue < 0L)
                    __lvalue = 0L;
                if(__lvalue > 30000L)
                    __lvalue = 30000L;
                __lvalue *= (INT32S)pixels_per_100mm;
                real_x    =  (int)(__lvalue / 1000L) + base_for_real_x + 1;
                if(real_x != t->l_old[i].old_x ||
                    state  != t->l_old[i].old_state){
                    blank_x2 = real_x - 13;
                    ___draw_blank(blank_x1, a_L1_y, blank_x2, bkcolor);
                    blank_x1 = real_x + 12;
                    blank_flag = 1;
                    ___draw_wheel(real_x, a_L1_y, i, state, bkcolor);
                    t->l_old[i].old_x = real_x;
                    t->l_old[i].old_state = state;
                    #if 1 /* 相邻距离太近所导致的重绘 */
                    if(i > 0){
                        if((real_x-24) < t->l_old[i-2].old_x)
                             t->l_old[i-2].old_x = 0xFFFF;
                    }
                    if(i < (slc->l_number-2)){
                        if((real_x+24) > t->l_old[i+2].old_x)
                             t->l_old[i+2].old_x = 0xFFFF;
                    }
                    #endif
                } else {
                    if(blank_flag){
                        blank_x2 = real_x - 13;
                        ___draw_blank(blank_x1, a_L1_y, blank_x2, bkcolor);
                    }
                    blank_x1 = real_x + 12;
                    blank_flag = 0;
                }
            }
            if(blank_flag){
                ___draw_blank(blank_x1, a_L1_y, (a_x1-1), bkcolor);
            }
            /* 后排线 */
            blank_x1 = a_x+1;
            blank_flag = 0;
            for(i=1; i<slc->l_number; i+=2){
                state = KL_STATE_NONE;
                if(slc->kl_set.l_selected[i])
                    state |= KL_STATE_SLCT;
                if(slc->state.l_down)
                    state |= KL_STATE_DOWN;
                if(t->selected_line == i)
                    state |= KL_STATE_BLINK;
                act_value = slc->kl_act.l_location[i];
                __lvalue = 15000L + (INT32S)act_value;
                if(__lvalue < 0L)
                    __lvalue = 0L;
                if(__lvalue > 30000L)
                    __lvalue = 30000L;
                __lvalue *= (INT32S)pixels_per_100mm;
                real_x    =  (int)(__lvalue / 1000L) + base_for_real_x + 1;
                if(real_x != t->l_old[i].old_x ||
                    state  != t->l_old[i].old_state){
                    blank_x2 = real_x - 13;
                    ___draw_blank(blank_x1, a_L2_y, blank_x2, bkcolor);
                    blank_x1 = real_x + 12;
                    blank_flag = 1;
                    ___draw_wheel(real_x, a_L2_y, i, state, bkcolor);
                    t->l_old[i].old_x = real_x;
                    t->l_old[i].old_state = state;
                    #if 1 /* 相邻距离太近所导致的重绘 */
                    if(i > 0){
                        if((real_x-24) < t->l_old[i-2].old_x)
                             t->l_old[i-2].old_x = 0xFFFF;
                    }
                    if(i < (slc->l_number-1)){
                        if((real_x+24) > t->l_old[i+2].old_x)
                             t->l_old[i+2].old_x = 0xFFFF;
                    }
                    #endif
                } else {
                    if(blank_flag){
                        blank_x2 = real_x - 13;
                        ___draw_blank(blank_x1, a_L2_y, blank_x2, bkcolor);
                    }
                    blank_x1 = real_x + 12;
                    blank_flag = 0;
                }
            }
            if(blank_flag){
                ___draw_blank(blank_x1, a_L2_y, (a_x1-1), bkcolor);
            }
        } else { /* 单排线 */
            blank_x1 = a_x+1;
            blank_flag = 0;
            for(i=0; i<slc->l_number; i++){
                state = KL_STATE_NONE;
                if(slc->kl_set.l_selected[i])
                    state |= KL_STATE_SLCT;
                if(slc->state.l_down)
                    state |= KL_STATE_DOWN;
                if(t->selected_line == i)
                    state |= KL_STATE_BLINK;
                act_value = slc->kl_act.l_location[i];
                __lvalue = 15000L + (INT32S)act_value;
                if(__lvalue < 0L)
                    __lvalue = 0L;
                if(__lvalue > 30000L)
                    __lvalue = 30000L;
                __lvalue *= (INT32S)pixels_per_100mm;
                real_x    =  (int)(__lvalue / 1000L) + base_for_real_x + 1;
                if(real_x != t->l_old[i].old_x ||
                    state  != t->l_old[i].old_state){
                    blank_x2 = real_x - 13;
                    ___draw_blank(blank_x1, a_L1_y, blank_x2, bkcolor);
                    blank_x1 = real_x + 12;
                    blank_flag = 1;
                    ___draw_wheel(real_x, a_L1_y, i, state, bkcolor);
                    t->l_old[i].old_x = real_x;
                    t->l_old[i].old_state = state;
                    #if 1 /* 相邻距离太近所导致的重绘 */
                    if(i > 0){
                        if((real_x-24) < t->l_old[i-1].old_x)
                             t->l_old[i-1].old_x = 0xFFFF;
                    }
                    if(i < (slc->l_number-1)){
                        if((real_x+24) > t->l_old[i+1].old_x)
                             t->l_old[i+1].old_x = 0xFFFF;
                    }
                    #endif
                } else {
                    if(blank_flag){
                        blank_x2 = real_x - 13;
                        ___draw_blank(blank_x1, a_L1_y, blank_x2, bkcolor);
                    }
                    blank_x1 = real_x + 12;
                    blank_flag = 0;
                }
            }
            if(blank_flag){
                ___draw_blank(blank_x1, a_L1_y, (a_x1-1), bkcolor);
            }
        }
        /* 画纵切刀 */
        blank_x1 = a_x+1;
        blank_flag = 0;
        for(i=0; i<slc->k_number; i++){
            state = KL_STATE_NONE;
            if(slc->kl_set.k_selected[i])
                state |= KL_STATE_SLCT;
            if(slc->state.k_down)
                state |= KL_STATE_DOWN;
            if(t->selected_knife == i)
                state |= KL_STATE_BLINK;
            act_value = slc->kl_act.k_location[i];
            __lvalue = 15000L + (INT32S)act_value;
            if(__lvalue < 0L)
                __lvalue = 0L;
            if(__lvalue > 30000L)
                __lvalue = 30000L;
            __lvalue *= (INT32S)pixels_per_100mm;
            real_x    =  (int)(__lvalue / 1000L) + base_for_real_x + 1;
            if(real_x != t->k_old[i].old_x ||
                state  != t->k_old[i].old_state){
                blank_x2 = real_x - 25;
                ___draw_blank(blank_x1, a_K_y, blank_x2, bkcolor);
                blank_x1 = real_x;
                blank_flag = 1;
                ___draw_knife(real_x, a_K_y, i, state, bkcolor);
                t->k_old[i].old_x = real_x;
                t->k_old[i].old_state = state;
                #if 1 /* 相邻距离太近所导致的重绘 */
                if(i > 0){
                    if((real_x-24) < t->k_old[i-1].old_x)
                         t->k_old[i-1].old_x = 0xFFFF;
                }
                if(i < (slc->k_number-1)){
                    if((real_x+24) > t->k_old[i+1].old_x)
                         t->k_old[i+1].old_x = 0xFFFF;
                }
                #endif
            } else {
                if(blank_flag){
                    blank_x2 = real_x - 25;
                    ___draw_blank(blank_x1, a_K_y, blank_x2, bkcolor);
                }
                blank_x1 = real_x;
                blank_flag = 0;
            }
        }
        if(blank_flag){
            ___draw_blank(blank_x1, a_K_y, (a_x1-1), bkcolor);
        }
        /* 显示当前订单 */
        order = &slc->working;
        make_order_info(order_info_buf, order);
        draw_font_for_widget(oi_area->x, oi_area->y, oi_area->width, oi_area->height, 
                              order_info_buf, t->order_info_old, t->order_info_color, bkcolor, 
                              t->order_info_font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
        
    } /* if(animation->flag & GUI_WIDGET_FLAG_REFRESH) else {...} */

    return;
}

/*-----------------------------------------------------------------------------------------
 * 函数:    kl_animation_initialize()
 *
 * 描述:    刀线位置监控控件的注册
**---------------------------------------------------------------------------------------*/
int kl_animation_initialize(void)
{
    int id;
    gui_widget_type __widget_type_kl_animation;
    
    __widget_type_kl_animation.draw = __widget_kl_animation_draw;

    id = gui_register_usr_widget(&__widget_type_kl_animation);

    lock_kernel();
    __widget_id_kl_animation = id;
    unlock_kernel();

    return id;
}

/*-----------------------------------------------------------------------------------------
 * 
 *      监控画面下的状态控件刷新(假控件)
 * 
**---------------------------------------------------------------------------------------*/
void __refresh_state_for_kl_monitor(gui_widget * c)
{
    static int k_up, k_dn, w_up, w_dn;
    static int start_up, start_dn;
    static int fixed_up, fixed_dn;
    static int regul_up, regul_dn;
    static int o_chg_up, o_chg_dn;
    static int stopd_up, stopd_dn;
    int k_state, w_state;
    int start_state, fixed_state;
    int regul_state, o_chg_state;
    int stopd_state;
    char ___s[16];
    slc_descriptor_t * slc;

    slc = &(config.slc[__current_slc_to_monitor]);
    
    if(c->flag & GUI_WIDGET_FLAG_REFRESH){
        k_up = k_dn = 0;
        w_up = w_dn = 0;
        start_up = start_dn = 0;
        fixed_up = fixed_dn = 0;
        regul_up = regul_dn = 0;
        o_chg_up = o_chg_dn = 0;
        stopd_up = stopd_dn = 0;
    } 
    if(1){
        /* 刷新车速 */
        if (slc->speed_scale < 1)
            slc->speed_scale = 1;
        sprintf(___s, pick_string("车速:    %d", "Speed:  %d"), (slc->slc_speed / slc->speed_scale));
        gui_label_set_text(mntr_speed_label, ___s);
        /* 刀上下 */
        k_state = slc->state.k_down;
        if(edge_dn(k_state, k_dn)){ /* 下降沿: 1 => 0 刀是否刚上去 */
            gui_set_widget_color(knife_state_up, COLOR_WHITE);
            gui_set_widget_bkcolor(knife_state_up, COLOR_BLUE);
            gui_set_widget_color(knife_state_dn, 0);
            gui_set_widget_bkcolor(knife_state_dn, 0);
        }
        if(edge_up(k_state, k_up)){ /* 上升沿: 0 => 1 刀是否刚下去 */
            gui_set_widget_color(knife_state_up, 0);
            gui_set_widget_bkcolor(knife_state_up, 0);
            gui_set_widget_color(knife_state_dn, 254);
            gui_set_widget_bkcolor(knife_state_dn, COLOR_KL_DOWN);
        }
        /* 线上下 */
        w_state = slc->state.l_down;
        if(edge_dn(w_state, w_dn)){ /* 下降沿: 1 => 0 线是否刚上去 */
            gui_set_widget_color(wheel_state_up, COLOR_WHITE);
            gui_set_widget_bkcolor(wheel_state_up, COLOR_BLUE);
            gui_set_widget_color(wheel_state_dn, 0);
            gui_set_widget_bkcolor(wheel_state_dn, 0);
        }
        if(edge_up(w_state, w_up)){ /* 上升沿: 0 => 1 线是否刚下去 */
            gui_set_widget_color(wheel_state_up, 0);
            gui_set_widget_bkcolor(wheel_state_up, 0);
            gui_set_widget_color(wheel_state_dn, 254);
            gui_set_widget_bkcolor(wheel_state_dn, COLOR_KL_DOWN);
        }
        /* 启动 */
        start_state = slc->state.start;
        if(edge_dn(start_state, start_dn)){ /* 下降沿: 1 => 0 */
            gui_set_widget_color(slc_mntr_start, STATE_COLOR);
            gui_set_widget_bkcolor(slc_mntr_start, STATE_BKCOLOR);
        }
        if(edge_up(start_state, start_up)){ /* 上升沿: 0 => 1 */
            gui_set_widget_color(slc_mntr_start, COLOR_WHITE);
            gui_set_widget_bkcolor(slc_mntr_start, COLOR_RED);
        }
        /* 定位 */
        fixed_state = slc->state.fixed;
        if(edge_dn(fixed_state, fixed_dn)){ /* 下降沿: 1 => 0 */
            gui_set_widget_color(slc_mntr_fixed, STATE_COLOR);
            gui_set_widget_bkcolor(slc_mntr_fixed, STATE_BKCOLOR);
        }
        if(edge_up(fixed_state, fixed_up)){ /* 上升沿: 0 => 1 */
            gui_set_widget_color(slc_mntr_fixed, COLOR_WHITE);
            gui_set_widget_bkcolor(slc_mntr_fixed, COLOR_RED);
        }
        /* 手调 */
        regul_state = slc->state.regulate;
        if(edge_dn(regul_state, regul_dn)){ /* 下降沿: 1 => 0 */
            gui_set_widget_color(slc_mntr_mannl, STATE_COLOR);
            gui_set_widget_bkcolor(slc_mntr_mannl, STATE_BKCOLOR);
        }
        if(edge_up(regul_state, regul_up)){ /* 上升沿: 0 => 1 */
            gui_set_widget_color(slc_mntr_mannl, COLOR_WHITE);
            gui_set_widget_bkcolor(slc_mntr_mannl, COLOR_RED);
        }
        /* 换单 */
        o_chg_state = slc->state.order_chg;
        if(edge_dn(o_chg_state, o_chg_dn)){ /* 下降沿: 1 => 0 */
            gui_set_widget_color(slc_mntr_o_chg, STATE_COLOR);
            gui_set_widget_bkcolor(slc_mntr_o_chg, STATE_BKCOLOR);
        }
        if(edge_up(o_chg_state, o_chg_up)){ /* 上升沿: 0 => 1 */
            gui_set_widget_color(slc_mntr_o_chg, COLOR_WHITE);
            gui_set_widget_bkcolor(slc_mntr_o_chg, COLOR_RED);
        }
        /* 停止 */
        stopd_state = slc->state.stop;
        if(edge_dn(stopd_state, stopd_dn)){ /* 下降沿: 1 => 0 */
            gui_set_widget_color(slc_mntr_stopd, STATE_COLOR);
            gui_set_widget_bkcolor(slc_mntr_stopd, STATE_BKCOLOR);
        }
        if(edge_up(stopd_state, stopd_up)){ /* 上升沿: 0 => 1 */
            gui_set_widget_color(slc_mntr_stopd, COLOR_WHITE);
            gui_set_widget_bkcolor(slc_mntr_stopd, COLOR_RED);
        }
    }
}

BOOL refresh_state_for_kl_monitor_initialize(void)
{
    int id;
    gui_widget_type __widget_type;
    gui_widget * refresh_state;  
    
    __widget_type.draw = __refresh_state_for_kl_monitor;

    id = gui_register_usr_widget(&__widget_type);
    if(id == 0)
        return fail;

    refresh_state = gui_create_widget(id, 0, 0, 2, 2, 0, 0, 0, 0);
    if(!refresh_state)
        return fail;
    gui_widget_link(slc_mntr_screen, refresh_state);

    return ok;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      显示压线位置的视图对象
 * 
**---------------------------------------------------------------------------------------*/
struct __wheel_position_old_buf {
    char caption[8];
    char press_1[8];
    char press_2[8];
    char wheels[SLC_L_MAX_NR][8];
};

void show_wheel_position(int index, int row, 
                         int x, int y, int width_zoom, int height, 
                         COLOR color, COLOR bkcolor, int font,
                         COLOR marker_color, COLOR marker_bkcolor,
                         void  *old, int *fields_width, INT16U option)
{
    struct __wheel_position_old_buf * buf_old;
    int i, j, to_left, __font;
    COLOR __color;
    char ___s[12];
    int  * value, wheels;
    slc_descriptor_t * slc;
        
    FamesAssert(old);
    FamesAssert(fields_width);

    if(!old || !fields_width)
        return;

    index  = index;
    color  = color;
    option = option;

    buf_old = (struct __wheel_position_old_buf *)old;
    i = row;
    to_left = width_zoom;

    slc = &config.slc[__current_slc_to_monitor];
    wheels = slc->l_number;

    /* 估计最适合的字体 */
    __font = font;
    if(fields_width[1] >= 86){
        font = font24;
    } else {
        font = font_mntr;
    }

    if(i == 0 || i == 1){ /* 设定值与实际值 */
        if(i == 0){
            value = &(slc->kl_set.l_location[0]);
            __color = COLOR_WHITE;
        } else {
            value = &(slc->kl_act.l_location[0]);
            __color = COLOR_YELLOW;
        }
        j = 0;
        draw_font_for_widget(x+3, y, (fields_width[j]-to_left)-6, height, 
                             ((i==0)?pick_string("设定值", "Set"):pick_string("实际值", "Act")), 
                              buf_old->caption, marker_color, marker_bkcolor, __font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
        x += fields_width[j];
        for(j = 1; j<= wheels; j++){
            INT16toSTR(___s, value[j-1], CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|CHG_OPT_SIG|0x71);
            draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                                  buf_old->wheels[j-1], __color, bkcolor, font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
            x += fields_width[j];
        }
        if(i == 0){
            INT16toSTR(___s, slc->kl_set.press_1_location, CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|CHG_OPT_SIG|0x71);
            draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                                  buf_old->press_1, __color, bkcolor, font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
            x += fields_width[j];
            if(slc->slc_type == SLC_TYPE_DOUBLE){
                INT16toSTR(___s, slc->kl_set.press_2_location, CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|CHG_OPT_SIG|0x71);
                draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                                      buf_old->press_2, __color, bkcolor, font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
                x += fields_width[j];
            }
        }
        if(i == 1){
            INT16toSTR(___s, slc->kl_act.press_1_location, CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|CHG_OPT_SIG|0x71);
            draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                                  buf_old->press_1, __color, bkcolor, font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
            x += fields_width[j];
            if(slc->slc_type == SLC_TYPE_DOUBLE){
                INT16toSTR(___s, slc->kl_act.press_2_location, CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|CHG_OPT_SIG|0x71);
                draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                                      buf_old->press_2, __color, bkcolor, font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
                x += fields_width[j];
            }
        }
    } else if(i == 2){ /* 实际值相对设定值的偏差 */
        int * act, * set;

        __color = COLOR_RED;
        
        set = &(slc->kl_set.l_location[0]);
        act = &(slc->kl_act.l_location[0]);
        j = 0;
        draw_font_for_widget(x+3, y, (fields_width[j]-to_left)-6, height, pick_string("偏  差", "Diff"), 
                              buf_old->caption, marker_color, marker_bkcolor, __font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
        x += fields_width[j];
        for(j = 1; j<= wheels; j++){
            INT16toSTR(___s, (act[j-1]-set[j-1]), CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|CHG_OPT_SIG|0x71);
            draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                                  buf_old->wheels[j-1], __color, bkcolor, font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
            x += fields_width[j];
        }
        INT16toSTR(___s, (slc->kl_act.press_1_location-slc->kl_set.press_1_location),
                                CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|CHG_OPT_SIG|0x71);
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                              buf_old->press_1, __color, bkcolor, font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
        x += fields_width[j];
        if(slc->slc_type == SLC_TYPE_DOUBLE){
            INT16toSTR(___s, (slc->kl_act.press_2_location-slc->kl_set.press_2_location),
                                CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|CHG_OPT_SIG|0x71);
            draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                                  buf_old->press_2, __color, bkcolor, font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
            x += fields_width[j];
        }
    }
}

int get_max_wheel_position(void)
{
    return 2;
}

BOOL init_wheel_position_private(int wheels, int slc_type, int first_call)
{
    static view_fields_t * wheel_position_fields = NULL;
    static char (* wheel_position_caption)[8] = NULL;
    int  i, fields, width;


    FamesAssert(wheels > 3);
    if(wheels <= 3)
        return fail;

    if(!wheel_position_fields){
        wheel_position_fields = mem_alloc(sizeof(view_fields_t) * (SLC_L_MAX_NR+4));
    }
    if(!wheel_position_caption){
        wheel_position_caption = mem_alloc(8 * SLC_L_MAX_NR);
    }
    if(!wheel_position_fields || !wheel_position_caption)
        return fail;

    fields = wheels;
    if(slc_type == SLC_TYPE_DOUBLE)
        fields += 2;
    else
        fields += 1;

    width = (__wheel_position_widget_width-16-56)/fields;
    width -= 8;
    width /= 8;
      
    wheel_position_fields[0].caption = pick_string("==线==", "= L =");
    wheel_position_fields[0].id = 0;
    wheel_position_fields[0].bytes = 7;
    wheel_position_fields[0].bytes_for_width = 7;
    wheel_position_fields[0].style = 0;
    wheel_position_fields[0].draw_style = DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER;
    wheel_position_fields[0].comment = "";
    for(i=1; i<=wheels; i++){
        sprintf(wheel_position_caption[i-1], pick_string("线%d", "L%d"), i);
        wheel_position_fields[i].caption = wheel_position_caption[i-1];
        wheel_position_fields[i].id = i;
        wheel_position_fields[i].bytes = 7;
        wheel_position_fields[i].bytes_for_width = width;
        wheel_position_fields[i].style = 0;
        wheel_position_fields[i].draw_style = DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER;
        wheel_position_fields[i].comment = "";
    }
    if(slc_type == SLC_TYPE_DOUBLE){
        wheel_position_fields[i].caption = pick_string("前压深", "Deep1");
        wheel_position_fields[i].id = i;
        wheel_position_fields[i].bytes = 7;
        wheel_position_fields[i].bytes_for_width = width;
        wheel_position_fields[i].style = 0;
        wheel_position_fields[i].draw_style = DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER;
        wheel_position_fields[i].comment = "";
        i++;
        wheel_position_fields[i].caption = pick_string("后压深", "Deep2");
        wheel_position_fields[i].id = i;
        wheel_position_fields[i].bytes = 7;
        wheel_position_fields[i].bytes_for_width = width;
        wheel_position_fields[i].style = 0;
        wheel_position_fields[i].draw_style = DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER;
        wheel_position_fields[i].comment = "";
        i++;
    } else {
        wheel_position_fields[i].caption = pick_string("压深", "Deep");
        wheel_position_fields[i].id = i;
        wheel_position_fields[i].bytes = 7;
        wheel_position_fields[i].bytes_for_width = width;
        wheel_position_fields[i].style = 0;
        wheel_position_fields[i].draw_style = DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER;
        wheel_position_fields[i].comment = "";
        i++;
    }
    wheel_position_fields[i].caption = NULL;
    
    gui_view_init_private( wheel_position, 
                           wheel_position_fields, 
                           get_max_wheel_position,
                           NULL, 
                           NULL, 
                           NULL,
                           show_wheel_position,
                           NULL, 
                           NULL, 
                           NULL,
                           COLOR_WHITE, 
                           17, 
                           34,
                           first_call
                         );
    return ok;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      显示刀位置的视图对象
 * 
**---------------------------------------------------------------------------------------*/
struct __knife_position_old_buf {
    char caption[8];
    char fan_position[8];
    char knives[SLC_K_MAX_NR][8];
};

void show_knife_position(int index, int row, 
                         int x, int y, int width_zoom, int height, 
                         COLOR color, COLOR bkcolor, int font,
                         COLOR marker_color, COLOR marker_bkcolor,
                         void *old, int *fields_width, INT16U option)
{
    struct __knife_position_old_buf * buf_old;
    int i, j, to_left, __font;
    COLOR __color;
    char ___s[12];
    int  * value, knives;
    slc_descriptor_t * slc;
        
    FamesAssert(old);
    FamesAssert(fields_width);

    if(!old || !fields_width)
        return;

    index  = index;
    color  = color;
    option = option;

    buf_old = (struct __knife_position_old_buf *)old;
    i = row;
    to_left = width_zoom;

    slc = &config.slc[__current_slc_to_monitor];
    knives = slc->k_number;

    /* 估计最适合的字体 */
    __font = font;
    if(fields_width[1] >= 86){
        font = font24;
    } else {
        font = font_mntr;
    }

    if(i == 0 || i == 1){ /* 设定值与实际值 */
        if(i == 0){
            value = &(slc->kl_set.k_location[0]);
            __color = COLOR_WHITE;
        } else {
            value = &(slc->kl_act.k_location[0]);
            __color = COLOR_YELLOW;
        }
        j = 0;
        draw_font_for_widget(x+3, y, (fields_width[j]-to_left)-6, height, 
                             ((i==0)?pick_string("设定值", "Set"):pick_string("实际值", "Act")),
                              buf_old->caption, marker_color, marker_bkcolor, __font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
        x += fields_width[j];
        for(j = 1; j<= knives; j++){
            INT16toSTR(___s, value[j-1], CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|CHG_OPT_SIG|0x71);
            draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                                  buf_old->knives[j-1], __color, bkcolor, font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
            x += fields_width[j];
        }
        if(i == 0){
            INT16toSTR(___s, slc->kl_set.fan_location, CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|CHG_OPT_SIG|0x71);
            draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                                  buf_old->fan_position, __color, bkcolor, font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
            x += fields_width[j];
        }
        if(i == 1){
            INT16toSTR(___s, slc->kl_act.fan_location, CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|CHG_OPT_SIG|0x71);
            draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                                  buf_old->fan_position, __color, bkcolor, font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
            x += fields_width[j];
        }
    } else if(i == 2){ /* 实际值相对设定值的偏差 */
        int * act, * set;

        __color = COLOR_RED;
        
        set = &(slc->kl_set.k_location[0]);
        act = &(slc->kl_act.k_location[0]);
        j = 0;
        draw_font_for_widget(x+3, y, (fields_width[j]-to_left)-6, height, pick_string("偏  差", "Diff"), 
                              buf_old->caption, marker_color, marker_bkcolor, __font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
        x += fields_width[j];
        for(j = 1; j<= knives; j++){
            INT16toSTR(___s, (act[j-1]-set[j-1]), CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|CHG_OPT_SIG|0x71);
            draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                                  buf_old->knives[j-1], __color, bkcolor, font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
            x += fields_width[j];
        }
        INT16toSTR(___s, (slc->kl_act.fan_location-slc->kl_set.fan_location),
                                CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|CHG_OPT_SIG|0x71);
        draw_font_for_widget(x, y, fields_width[j]-to_left, height, ___s, 
                              buf_old->fan_position, __color, bkcolor, font, DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER);
        x += fields_width[j];
    }
}

int get_max_knife_position(void)
{
    return 2;
}

BOOL init_knife_position_private(int knives, int first_call)
{
    static view_fields_t * knife_position_fields = NULL;
    static char (* knife_position_caption)[8] = NULL;
    int  i, fields, width;


    FamesAssert(knives > 2);
    if(knives <= 2)
        return fail;

    if(!knife_position_fields){
        knife_position_fields = mem_alloc(sizeof(view_fields_t) * (SLC_K_MAX_NR+3));
    }
    if(!knife_position_caption){
        knife_position_caption = mem_alloc(8 * SLC_K_MAX_NR);
    }
    if(!knife_position_fields || !knife_position_caption)
        return fail;

    fields = knives+1;

    width = (__knife_position_widget_width-16-56)/fields;
    width -= 8;
    width /= 8;
      
    knife_position_fields[0].caption = pick_string("==刀==", "= K =");
    knife_position_fields[0].id = 0;
    knife_position_fields[0].bytes = 7;
    knife_position_fields[0].bytes_for_width = 7;
    knife_position_fields[0].style = 0;
    knife_position_fields[0].draw_style = DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER;
    knife_position_fields[0].comment = "";
    for(i=1; i<=knives; i++){
        sprintf(knife_position_caption[i-1], pick_string("刀%d", "L%d"), i);
        knife_position_fields[i].caption = knife_position_caption[i-1];
        knife_position_fields[i].id = i;
        knife_position_fields[i].bytes = 7;
        knife_position_fields[i].bytes_for_width = width;
        knife_position_fields[i].style = 0;
        knife_position_fields[i].draw_style = DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER;
        knife_position_fields[i].comment = "";
    }
    knife_position_fields[i].caption = pick_string("吸风", "Fan");
    knife_position_fields[i].id = i;
    knife_position_fields[i].bytes = 7;
    knife_position_fields[i].bytes_for_width = width;
    knife_position_fields[i].style = 0;
    knife_position_fields[i].draw_style = DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER;
    knife_position_fields[i].comment = "";
    i++;
    knife_position_fields[i].caption = NULL;
    
    gui_view_init_private( knife_position, 
                           knife_position_fields, 
                           get_max_knife_position, 
                           NULL, 
                           NULL, 
                           NULL,
                           show_knife_position, 
                           NULL, 
                           NULL,
                           NULL,
                           COLOR_WHITE, 
                           17, 
                           34,
                           first_call
                         );
    return ok;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      监控画面下的操作
 * 
**---------------------------------------------------------------------------------------*/
void enter_slc_monitor(int machine)
{
    KEYCODE key;
    slc_descriptor_t * slc;
    char ___s[64];
    extern PLC * slc_plc[];
    void slc_fine_tune(kl_animation_private * data);

    machine--;

    if(!(machine == 0 || machine == 1))
        return;

    lock_kernel();
    __current_slc_to_monitor = machine;
    unlock_kernel();

    slc = &(config.slc[machine]);

    if(!slc_mntr_screen)
        return;
    sprintf(___s, pick_string("刀线位置监控 - 机%d", "SLC MONITOR - [%d]"), (machine+1));
    gui_form_set_caption(slc_mntr_screen, ___s);
    kl_animation_set_slc(kl_animation, machine);
    init_knife_position_private(slc->k_number, 0);
    init_wheel_position_private(slc->l_number, slc->slc_type, 0);
    gui_set_root_widget(slc_mntr_screen);

    if(slc_plc[machine]){
        plc_action_id_set_number(slc_plc[machine], id_read_act_k, slc->k_number);
        plc_action_id_set_number(slc_plc[machine], id_read_act_l, slc->l_number);
        plc_action_enable_id(slc_plc[machine], id_read_act_k);
        plc_action_enable_id(slc_plc[machine], id_read_act_l);
        plc_action_enable_id(slc_plc[machine], id_read_act_p1);
        plc_action_enable_id(slc_plc[machine], id_read_act_p2);
        plc_action_enable_id(slc_plc[machine], id_read_act_fan);
        plc_action_enable_id(slc_plc[machine], id_read_speed);
    }

    for(;;){
        key = waitkey(0L);
        switch(key){
            case ESC:
                if(slc_plc[machine]){
                    plc_action_disable_id(slc_plc[machine], id_read_act_k);
                    plc_action_disable_id(slc_plc[machine], id_read_act_l);
                    plc_action_disable_id(slc_plc[machine], id_read_act_p1);
                    plc_action_disable_id(slc_plc[machine], id_read_act_p2);
                    plc_action_disable_id(slc_plc[machine], id_read_act_fan);
                    plc_action_disable_id(slc_plc[machine], id_read_speed);
                }
                return;
            case F1:
                slc_fine_tune((kl_animation_private *)(kl_animation->private_data));
                break;
            case F5:
                gui_refresh_widget(slc_mntr_screen);
                break;
        }
    }
}

/*-----------------------------------------------------------------------------------------
 *          
 *      监控画面下的微调操作(fine-tune)
 * 
**---------------------------------------------------------------------------------------*/
void slc_fine_tune(kl_animation_private * data)
{
    int i, is_k, slc_index;
    KEYCODE key;
    slc_descriptor_t * slc;
    int  value, is_sub;
    char  ___s[32];

    FamesAssert(data);
    if(!data)
        return;
     
    slc = &(config.slc[__current_slc_to_monitor]);

    slc_index = __current_slc_to_monitor+1; /* 1 or 2 */

    lock_kernel();
    data->selected_line = 0;
    unlock_kernel();

    gui_label_set_text(slc_mntr_status_bar, 
                          pick_string(" PAGE-DOWN 手动下    PAGE-UP 手动上    减号: 刀线左移    加号: 刀线右移    >>>ESC 返回",
                                      " PAGE-DOWN Select    PAGE-UP Deselect    '-': To-Left    '+': To-Right    >>>ESC Return")
                      );

    for(i = 0, is_k = 0;;){ /* (is_k=0)意思是先选择线 */
        key = waitkey(0L);
        switch(key){
            case ESC:
                goto out;
            case ENTER:
            case TAB:
            case DOWN:
            case RIGHT:
                i++;
                if(!is_k && i >= slc->l_number){
                    is_k = 1;
                    i = 0;
                    lock_kernel();
                    data->selected_line = -1;
                    unlock_kernel();
                }
                if(is_k && i >= slc->k_number){
                    is_k = 0;
                    i = 0;
                    lock_kernel();
                    data->selected_knife = -1;
                    unlock_kernel();
                }
                if(is_k){
                    lock_kernel();
                    data->selected_knife = i;
                    unlock_kernel();
                } else {
                    lock_kernel();
                    data->selected_line = i;
                    unlock_kernel();
                }
                break;
            case SHIFT_TAB:
            case UP:
            case LEFT:
                i--;
                if(!is_k && i < 0){
                    is_k = 1;
                    i = slc->k_number-1;
                    lock_kernel();
                    data->selected_line = -1;
                    unlock_kernel();
                }
                if(is_k && i < 0){
                    is_k = 0;
                    i = slc->l_number-1;;
                    lock_kernel();
                    data->selected_knife = -1;
                    unlock_kernel();
                }
                if(is_k){
                    lock_kernel();
                    data->selected_knife = i;
                    unlock_kernel();
                } else {
                    lock_kernel();
                    data->selected_line = i;
                    unlock_kernel();
                }
                break;
            case '-':  /* 微调 */
            case '+':
                if(key == '-')
                    is_sub = 1;
                else
                    is_sub = 0;
                sprintf(___s, 
                        pick_string("%s %d 向 %s [%s]:", "%s[%d] To %s [%s]:"), 
                        (is_k?pick_string("刀", "K"):pick_string("线", "L")), 
                        (i+1), 
                        (is_sub?pick_string("左", "Left"):pick_string("右", "Right")), 
                        (is_sub?"<=":"=>"));
                gui_label_set_text(fine_tune_text, ___s);
                gui_hide_widget(kl_animation);
                gui_show_widget(fine_tune_form);
                ___s[0] = 0;
                do {
                    key = gui_edit_input(fine_tune_value, ___s, 5, 0);
                    switch(key){
                        case ENTER:
                        case F10:
                            value = STRtoINT16(___s, CHG_OPT_DEC);
                            if(value < 0 || value > 30){ /* 0 ~ 30 */
                                break; /* 超出范围 */
                            } else {
                                INT32S temp32;
                                INT16S temp16;
                                if(is_sub)
                                    value = -value;
                                if(is_k){
                                    value += slc->kl_set.k_location[i];
                                    slc->kl_set.k_location[i] = value;
                                    temp32 = (INT32S)value;
                                    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_SET_VALUE_K[i], &temp32, 1);
                                    temp16 = 1;
                                    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M,  PLC_ADDR_FINE_TUNE_K[i], &temp16, 1);
                                    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M,  PLC_ADDR_FINE_TUNE,      &temp16, 1);
                                } else {
                                    value += slc->kl_set.l_location[i];
                                    slc->kl_set.l_location[i] = value;
                                    temp32 = (INT32S)value;
                                    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_SET_VALUE_L[i], &temp32, 1);
                                    temp16 = 1;
                                    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M,  PLC_ADDR_FINE_TUNE_L[i], &temp16, 1);
                                    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M,  PLC_ADDR_FINE_TUNE,      &temp16, 1);
                                }
                                key = DUMMY_KEY;
                            }
                            break;
                        case ESC:
                            key = DUMMY_KEY;
                            break;
                        default:
                            break;
                    } /* switch(key) */
                } while(key != DUMMY_KEY);
                gui_hide_widget(fine_tune_form);
                gui_show_widget(kl_animation);
                break;                
            case PGUP: /* 手动起刀 */
                if(is_k){
                    lock_kernel();
                    data->selected_knife = -1;
                    slc->kl_set.k_selected[i] = 0;
                    unlock_kernel();
                    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M,  PLC_ADDR_SELECTED_K[i],  &(slc->kl_set.k_selected[i]), 1);
                } else {
                    lock_kernel();
                    data->selected_line = -1;
                    slc->kl_set.l_selected[i] = 0;
                    unlock_kernel();
                    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M,  PLC_ADDR_SELECTED_L[i],  &(slc->kl_set.l_selected[i]), 1);
                }
                break;
            case PGDN: /* 手动下刀 */
                if(is_k){
                    lock_kernel();
                    data->selected_knife = -1;
                    slc->kl_set.k_selected[i] = 1;
                    unlock_kernel();
                    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M,  PLC_ADDR_SELECTED_K[i],  &(slc->kl_set.k_selected[i]), 1);
                } else {
                    lock_kernel();
                    data->selected_line = -1;
                    slc->kl_set.l_selected[i] = 1;
                    unlock_kernel();
                    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M,  PLC_ADDR_SELECTED_L[i],  &(slc->kl_set.l_selected[i]), 1);
                }
                break;
            default:
                break;
        }
    }

out:
    lock_kernel();
    data->selected_line  = -1;
    data->selected_knife = -1;
    unlock_kernel();
    gui_label_set_text(slc_mntr_status_bar, pick_string(">>>F1 微调     >>>ESC 返回", ">>>F1 Fine-Tune     >>>ESC Return"));
    return;    
}

/*=========================================================================================
 * 
 * 本文件结束: slc/slc_mntr.c
 * 
**=======================================================================================*/


