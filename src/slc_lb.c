/******************************************************************************************
 * 文件:    slc/slc_lb.c
 *
 * 描述:    楞别深浅设置
 *
 * 作者:    Jun
******************************************************************************************/
#define  SLC_LB_C
#include <FamesOS.h>
#include "common.h"

/*------------------------------------------------------------------------------------
 * 
 *          文件名
 * 
**----------------------------------------------------------------------------------*/
static char * lb_dat_fname = "lb.dat";    /* 楞别配置文件 */

static struct slc_lb_file * lb_image = NULL;

void read_lb_dat(void)
{
    int fd = -1;

    if(!lb_image){
        lb_image = mem_alloc((INT32U)sizeof(struct slc_lb_file));
    }
    if(!lb_image){
        sys_print("Out of memory in read_lb_dat()!\n");
        ExitApplication();
    }    
    
    lock_kernel();
    fd=open(lb_dat_fname, O_RDONLY|O_BINARY);
    if(fd<0){                                 /* 如果打开失败,则要尝试创建文件     */
        fd=open(lb_dat_fname, O_WRONLY|O_BINARY|O_CREAT, S_IREAD|S_IWRITE);
        if(fd>=0){
            MEMSET((INT08S *)lb_image, 0, sizeof(struct slc_lb_file));
            write(fd, (void *)lb_image, sizeof(struct slc_lb_file));
            close(fd);
            fd=open(lb_dat_fname, O_RDONLY|O_BINARY);
        } else {                     /* 创建失败,可能是磁盘空间不足,退出! */
            goto out;
        }        
    }
    if(fd>=0){
        read(fd, (void *)lb_image, sizeof(struct slc_lb_file));
        close(fd);
    }

out:
    unlock_kernel();
}

void save_lb_dat(void)
{
    int fd = -1;
    
    lock_kernel();
    fd=open(lb_dat_fname, O_WRONLY|O_BINARY|O_CREAT, S_IREAD|S_IWRITE);
    if(fd>=0){
        write(fd, (void *)lb_image, sizeof(struct slc_lb_file));
        close(fd);
    }        
    unlock_kernel();
}

/*-----------------------------------------------------------------------------------------
 *          
 *      返回楞别深浅设定值
 * 
**---------------------------------------------------------------------------------------*/
int get_deep_value(char * flute, int yx)
{
    int i;

    FamesAssert(flute);
    FamesAssert(yx <= YX_MAX_NR);

    if(!flute)
        return -1;
    if(yx > YX_MAX_NR)
        return -1;

    if(yx < 1)
        yx = 1;

    for(i=0; i<FLUTE_MAX_NR; i++){
        lb_image->items[i].flute[3] = 0;
        if(!STRCMP(flute, lb_image->items[i].flute))
            return lb_image->items[i].value[yx-1];
    }
    return lb_image->default_item.value[yx-1];
}

/*-----------------------------------------------------------------------------------------
 *          
 *      控件及其它定义
 * 
**---------------------------------------------------------------------------------------*/
static gui_widget * lb_screen   = NULL;             /* 深浅画面的主控件      */
static gui_widget * status_bar  = NULL;             /* 深浅画面的状态条      */
static gui_widget * lb_view     = NULL;             /* 楞别深浅的显示        */

#define nr_buttons 2                                /* 按钮个数              */
static gui_widget * buttons[nr_buttons];            /* 按钮                  */
static char * buttons_caption_zh[nr_buttons] = {    /* 按钮标题              */
    "F10 保存", "ESC 返回"
};
static char * buttons_caption_en[nr_buttons] = {    /* 按钮标题              */
    "F10 Save", "ESC Return"
};
static char ** buttons_caption = NULL;
 
extern BMPINFO  icon;                               /* 图标                  */

BOOL init_lb_view_private(void);
/*-----------------------------------------------------------------------------------------
 *          
 *      画面的定义(或初始化)
 * 
**---------------------------------------------------------------------------------------*/
gui_widget * init_lb_screen(void)
{
    int i, x, y, width, height;

    x = 110;
    y = 180;
    width = 736;
    height = 457;
    
    /* 主界面   */
    lb_screen = gui_create_widget(GUI_WIDGET_FORM, x, y, width, height, 0, 0, 1, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE);
    if(!lb_screen)
        goto some_error;
    gui_form_init_private(lb_screen, 128);
    gui_form_set_icon(lb_screen, &icon);
    gui_form_set_caption(lb_screen, pick_string("楞别深浅设置", "Flute Depth Setup"));
    
    /* 工具条   */
    status_bar = gui_create_widget(GUI_WIDGET_LABEL, 5, (height-35), (width-11), 30, 0, 0, 1, LABEL_STYLE_CLIENT_BDR);
    if(!status_bar)
        goto some_error;
    gui_widget_link(lb_screen, status_bar);
    gui_label_init_private(status_bar, 100);
    gui_label_set_text(status_bar, pick_string("最底行的深浅值是默认值(楞别为: \"***\")", "Bottom is the Default Value(\"***\")"));

    /* 深浅数据 */
    #define __lb_view_widget_width  672 /*(width-20)*/
    lb_view = gui_create_widget(GUI_WIDGET_VIEW, 30, 42, __lb_view_widget_width, 323, 0, 0, 1, VIEW_STYLE_FIELDS_TITLE|VIEW_STYLE_STATISTIC_BAR|VIEW_STYLE_NONE_FIRST);
    if(!lb_view)
        goto some_error;
    gui_widget_link(lb_screen, lb_view);
    if(!init_lb_view_private())
        goto some_error;

    /* 功能按钮 */
    x = 124; y = (height-84);
    width = 180; height = 42;
    buttons_caption = pick_string(buttons_caption_zh, buttons_caption_en);
    for(i=0; i<nr_buttons; i++){
        buttons[i] = gui_create_widget(GUI_WIDGET_BUTTON, x, y, width, height, 0, 0, 1, BUTTON_STYLE_CLIENT_BDR);
        if(!buttons[i])
            goto some_error;
        gui_widget_link(lb_screen, buttons[i]);
        gui_button_init_private(buttons[i], 32);
        gui_button_set_caption(buttons[i], buttons_caption[i]);
        x += (width+120);
    }

    return lb_screen;

some_error:
    sys_print("init_lb_screen(): failed to create widgets!\n");
    ExitApplication();
    return NULL;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      楞别深浅数据的视图对象
 * 
**---------------------------------------------------------------------------------------*/
static int get_max(void)
{
    return FLUTE_MAX_NR;
}

static BOOL is_writable(int index, int field_id, INT16U option)
{
    index  = index;
    option = option;
    field_id = field_id;

    return  ok;
}

static BOOL get_item(int index, int field_id, char * buf, int buf_len, INT16U option)
{
    INT16U chg_flag;
    BOOL retval;
    
    FamesAssert(buf);
    if(!buf)
        return fail;

    option  = option;
    buf_len = buf_len;  /* 这里不用, 是因为这里可以保证不会超过这个长度 */

    if(index > get_max())
        return fail;


    chg_flag = CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_FRC|0x51;

    retval = ok;

    if(index == FLUTE_MAX_NR){
        if(field_id < 0){
            STRCPY(buf, "***");
        } else {
            INT16toSTR(buf, lb_image->default_item.value[field_id], chg_flag);
        }
    } else {
        if(field_id < 0){
            STRCPY(buf, lb_image->items[index].flute);
        } else {
            INT16toSTR(buf, lb_image->items[index].value[field_id], chg_flag);
        }
    }
    
    return  retval;
}

static BOOL set_item(int index, int field_id, char * buf, int buf_len, KEYCODE key, INT16U option)
{
    INT16U chg_flag;
    int  temp;
    
    FamesAssert(buf);
    if(!buf)
        return fail;

    key     = key;
    option  = option;
    buf_len = buf_len;  /* 这里不用, 是因为这里可以保证不会超过这个长度 */

    if(index > get_max())
        return fail;

    chg_flag = CHG_OPT_DEC|CHG_OPT_FRC|0x51;
    temp = STRtoINT16(buf, chg_flag);

    if(index == FLUTE_MAX_NR){
        if(field_id < 0){
            ; /* 默认值的楞别总是: *** */
        } else {
            lb_image->default_item.value[field_id] = temp;
        }
    } else {
        if(field_id < 0){
            buf[3] = 0;
            STRCPY(lb_image->items[index].flute, buf);
        } else {
            lb_image->items[index].value[field_id]= temp;
        }
    }

    return  ok;
}

static void show_statistics(int index, int x, int y, int width, int height, 
                                       int color, int bkcolor, int font,
                                       INT08S *comment,
                                       INT08S *old, INT16U option)
{
    char ___s[96];

    comment = comment;
    option  = option;
    
    if(index == FLUTE_MAX_NR)
        sprintf(___s, pick_string(" === 默认值 ===", " === Default ==="));
    else
        sprintf(___s, "  === %d ===", index+1);

    draw_font_for_widget(x, y, width-16, height, ___s, old, color, bkcolor, font, DRAW_OPT_FIL_BG);
}


BOOL init_lb_view_private(void)
{
    static view_fields_t * lb_fields = NULL;
    static char (* captions)[12] = NULL;
    int  i, fields, width;

    if(!lb_fields){
        lb_fields = mem_alloc(sizeof(view_fields_t) * (YX_MAX_NR+2));
    }
    if(!captions){
        captions = mem_alloc(12 * YX_MAX_NR);
    }
    if(!lb_fields || !captions)
        return fail;

    fields = (get_yx_types());

    if(fields > 0)
        width = (__lb_view_widget_width-80)/fields;
    else
        width = (__lb_view_widget_width-80);
    width -= 16;
    width /= 8;
      
    lb_fields[0].caption = pick_string("楞别", "Flute");
    lb_fields[0].id = -1;
    lb_fields[0].bytes = 3;
    lb_fields[0].bytes_for_width = 9;
    lb_fields[0].style = 0;
    lb_fields[0].draw_style = DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER;
    lb_fields[0].comment = "";
    for(i=1; i<=fields; i++){
        sprintf(captions[i-1], "%s", get_yx_string(i));
        lb_fields[i].caption = captions[i-1];
        lb_fields[i].id = i-1;
        lb_fields[i].bytes = 5;
        lb_fields[i].bytes_for_width = width;
        lb_fields[i].style = 0;
        lb_fields[i].draw_style = DRAW_OPT_FIL_BG|DRAW_OPT_ALIGN_CENTER;
        lb_fields[i].comment = "";
    }
    lb_fields[i].caption = NULL;
    
    gui_view_init_private( lb_view, 
                           lb_fields, 
                           get_max, 
                           get_item, 
                           set_item, 
                           is_writable,
                           NULL, 
                           "", 
                           show_statistics,
                           NULL,
                           COLOR_WHITE, 
                           17, 
                           32,
                           1
                         );
    return ok;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      楞别深浅画面下的操作
 * 
**---------------------------------------------------------------------------------------*/
void lb_setup(void)
{
    KEYCODE key;

    if(!lb_screen)
        return;

    for(;;){
        key = gui_view_editing(lb_view, 0);
        switch(key){
            case ESC:
                read_lb_dat();
                return;
            case UP:
                gui_view_move_up(lb_view);
                break;
            case DOWN:
                gui_view_move_down(lb_view);
                break;
            case PGUP:
                gui_view_page_up(lb_view);
                break;
            case PGDN:
                gui_view_page_down(lb_view);
                break;
            case CTRL_HOME:
                gui_view_goto_top(lb_view);
                break;
            case CTRL_END:
                gui_view_goto_bottom(lb_view);
                break;
            case F10:
                gui_set_widget_style(buttons[0], BUTTON_STYLE_CLIENT_BDR|BUTTON_STYLE_PRESSED);
                save_lb_dat();
                TaskDelay(280);
                gui_set_widget_style(buttons[0], BUTTON_STYLE_CLIENT_BDR);
                break;
            case F5:
                gui_refresh_widget(lb_screen);
                break;
        }
    }
}

/*=========================================================================================
 * 
 * 本文件结束: slc/slc_lb.c
 * 
**=======================================================================================*/


