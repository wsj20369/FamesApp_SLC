/******************************************************************************************
 * 文件:    slc/ord_edit.c
 *
 * 描述:    订单编辑(输单, 调单等)
 *
 * 作者:    Jun
******************************************************************************************/
#define  SLC_INPUT_ORDER_C
#include <FamesOS.h>
#include "common.h"

/*-----------------------------------------------------------------------------------------
 *          
 *      输单画面中的控件及其它定义
 * 
**---------------------------------------------------------------------------------------*/
static gui_widget * input_screen = NULL;    /* 输单界面的主控件  */

extern BMPINFO icon;                        /* 图标              */

extern char  ___bkcolor_other_dialog[];

enum {
    ORD_EDIT_WKNO,
    ORD_EDIT_ORDNO,
    ORD_EDIT_CUTS,
    ORD_EDIT_DATA,
    ORD_EDIT_YX,
    ORD_EDIT_FLUTE,
    ORD_EDIT_TIRM,
};

/* 订单编辑对话框的定义 */
#if CONFIG_INPUT_ORDER_FONT24 == 1

DECLARE_INPUT_DIALOG_START(edit_order_dialog)
INPUT_DIALOG_ITEM("工号:", ORD_EDIT_WKNO,  4,  40,  42,  206, 36, CONFIG_FONT_24, 0, "")
INPUT_DIALOG_ITEM("单号:", ORD_EDIT_ORDNO, 8,  276, 42,  206, 36, CONFIG_FONT_24, 0, "")
INPUT_DIALOG_ITEM("剖数:", ORD_EDIT_CUTS,  2,  40,  92,  206, 36, CONFIG_FONT_24, 0, "")
INPUT_DIALOG_ITEM("压线:", ORD_EDIT_DATA,  48, 40,  142, 678, 36, CONFIG_FONT_24, 0, "")
INPUT_DIALOG_ITEM("压型:", ORD_EDIT_YX,    1,  276, 92,  206, 36, CONFIG_FONT_24, 0, "")
INPUT_DIALOG_ITEM("楞别:", ORD_EDIT_FLUTE, 3,  512, 42,  206, 36, CONFIG_FONT_24, 0, "")
INPUT_DIALOG_ITEM("修边:", ORD_EDIT_TIRM,  1,  512, 92,  206, 36, CONFIG_FONT_24, 0, "")
INPUT_DIALOG_SET(edit_order_dialog, "订单编辑", &icon, 110, 348, 808, 268, 0, 236, CONFIG_FONT_16, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE)
DECLARE_INPUT_DIALOG_ENDED(edit_order_dialog);

#else /* 默认用的是16号字体 */

#if 1       /* 压线在最下面一行 */
DECLARE_INPUT_DIALOG_START(edit_order_dialog)
INPUT_DIALOG_ITEM("工号:", ORD_EDIT_WKNO,  4,  40,  38,  196, 28, CONFIG_FONT_16, 0, "")
INPUT_DIALOG_ITEM("单号:", ORD_EDIT_ORDNO, 8,  256, 38,  196, 28, CONFIG_FONT_16, 0, "")
INPUT_DIALOG_ITEM("剖数:", ORD_EDIT_CUTS,  2,  40,  72,  196, 28, CONFIG_FONT_16, 0, "")
INPUT_DIALOG_ITEM("压线:", ORD_EDIT_DATA,  64, 40,  106, 628, 28, CONFIG_FONT_16, 0, "")
INPUT_DIALOG_ITEM("压型:", ORD_EDIT_YX,    1,  256, 72,  196, 28, CONFIG_FONT_16, 0, "")
INPUT_DIALOG_ITEM("楞别:", ORD_EDIT_FLUTE, 3,  472, 38,  196, 28, CONFIG_FONT_16, 0, "")
INPUT_DIALOG_ITEM("修边:", ORD_EDIT_TIRM,  1,  472, 72,  196, 28, CONFIG_FONT_16, 0, "")
INPUT_DIALOG_SET(edit_order_dialog, "订单编辑", &icon, 140, 388, 738, 221, 0, 236, CONFIG_FONT_16, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE)
DECLARE_INPUT_DIALOG_ENDED(edit_order_dialog);
#else       /* 压线在中间一行 */
DECLARE_INPUT_DIALOG_START(edit_order_dialog)
INPUT_DIALOG_ITEM("工号:", ORD_EDIT_WKNO,  4,  40,  38,  196, 28, 1, 0, "工号决定了生产的顺序, 0~9999")
INPUT_DIALOG_ITEM("单号:", ORD_EDIT_ORDNO, 8,  256, 38,  196, 28, 1, 0, "订单号")
INPUT_DIALOG_ITEM("剖数:", ORD_EDIT_CUTS,  2,  472, 38,  196, 28, 1, 0, "剖数, 1~9, S剖输1")
INPUT_DIALOG_ITEM("压线:", ORD_EDIT_DATA,  64, 40,  72,  628, 28, 1, 0, "压线资料, '*'代表刀, '+'代表线")
INPUT_DIALOG_ITEM("压型:", ORD_EDIT_YX,    1,  40,  106, 196, 28, 1, 0, "压型, 1~9")
INPUT_DIALOG_ITEM("楞别:", ORD_EDIT_FLUTE, 3,  256, 106, 196, 28, 1, 0, "楞别")
INPUT_DIALOG_ITEM("修边:", ORD_EDIT_TIRM,  1,  472, 106, 196, 28, 1, 0, "1=修边, 0=不修")
INPUT_DIALOG_SET(edit_order_dialog, "输单", &icon, 140, 388, 738, 221, 0, 236, 1, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE)
DECLARE_INPUT_DIALOG_ENDED(edit_order_dialog);
#endif

#endif /* CONFIG_INPUT_ORDER_FONT24 */

/*-----------------------------------------------------------------------------------------
 *          
 *      订单编辑对话框的初始化
 * 
**---------------------------------------------------------------------------------------*/
gui_widget * init_input_order_screen(void)
{
    edit_order_dialog.bkcolor = atoi(___bkcolor_other_dialog);
    input_dialog_preset_title(&edit_order_dialog, pick_string("订单编辑", "Order Input/Edit"));
    input_dialog_preset_item_name(&edit_order_dialog, ORD_EDIT_WKNO,   pick_string("工号:", "S/N :"));
    input_dialog_preset_item_name(&edit_order_dialog, ORD_EDIT_ORDNO,  pick_string("单号:", "OrdNo:"));
    input_dialog_preset_item_name(&edit_order_dialog, ORD_EDIT_CUTS,   pick_string("剖数:", "Cuts:"));
    input_dialog_preset_item_name(&edit_order_dialog, ORD_EDIT_DATA,   pick_string("压线:", "Data:"));
    input_dialog_preset_item_name(&edit_order_dialog, ORD_EDIT_YX,     pick_string("压型:", "Press:"));
    input_dialog_preset_item_name(&edit_order_dialog, ORD_EDIT_FLUTE,  pick_string("楞别:", "Flut:"));
    input_dialog_preset_item_name(&edit_order_dialog, ORD_EDIT_TIRM,   pick_string("修边:", "Trim:"));
    input_dialog_preset_item_comment(&edit_order_dialog, ORD_EDIT_WKNO, 
                                     pick_string("工号决定了生产的顺序, 0~9999", "Order S/N, 0~9999"));
    input_dialog_preset_item_comment(&edit_order_dialog, ORD_EDIT_ORDNO, 
                                     pick_string("订单号", "Order No."));
    input_dialog_preset_item_comment(&edit_order_dialog, ORD_EDIT_CUTS, 
                                     pick_string("剖数, 1~9", "Cuts, 1~9"));
    input_dialog_preset_item_comment(&edit_order_dialog, ORD_EDIT_DATA, 
                                     pick_string("压线资料, '*'代表刀, '+'代表线", "Data, '*'=K, '+'=L"));
    input_dialog_preset_item_comment(&edit_order_dialog, ORD_EDIT_YX, 
                                     pick_string("压型, 1~5", "Press Type, 1~5"));
    input_dialog_preset_item_comment(&edit_order_dialog, ORD_EDIT_FLUTE, 
                                     pick_string("楞别", "Flute"));
    input_dialog_preset_item_comment(&edit_order_dialog, ORD_EDIT_TIRM, 
                                     pick_string("1=修边, 0=不修", "1=YES, 0=NO"));
    input_screen = input_dialog_initialize(&edit_order_dialog);
    input_dialog_set_buttons_caption(&edit_order_dialog, 
                                     pick_string("确认[F10]", "OK.[F10]"), 
                                     pick_string("取消[ESC]", "No.[ESC]"));
    input_dialog_set_ok_comment(&edit_order_dialog, 
                                     pick_string("确认, 请按F10或回车键", "Press F10/Enter to Confirm"));
    input_dialog_set_cancel_comment(&edit_order_dialog, 
                                     pick_string("取消, 请按ESC或回车键", "Press ESC/Enter to Cancel"));
    gui_hide_widget(input_screen);
    return input_screen;
}

struct order_edit_private {
    order_struct order;
};

void prepare_edit(int id, char * buf, void * data, INT16U opt)
{
    order_struct * order;
    int trim_flag;
    struct order_edit_private * temp;

    FamesAssert(buf);
    FamesAssert(data);
    if(!buf || !data)
        return;

    /* 计算修边标记 */
    trim_flag = slc_is_trim_forced();

    temp = (struct order_edit_private *)data;
    order = &temp->order;
    
    switch(id){
        case ORD_EDIT_WKNO:
            sprintf(buf, "%d", order->WORKNO);
            break;
        case ORD_EDIT_ORDNO:
            sprintf(buf, "%s", order->ORDERNO);
            break;
        case ORD_EDIT_CUTS:
            if(order->CUTS <= 0)
                order->CUTS = 1;
            sprintf(buf, "%d", order->CUTS);
            break;
        case ORD_EDIT_DATA:
            sprintf(buf, "%s", order->SPECCUT);
            break;
        case ORD_EDIT_YX:
            if(opt & INPUT_DIALOG_PREPARE_OPT_EDITING)
                sprintf(buf, "%d", order->YX);
            else
                STRCPY(buf, get_yx_string(order->YX));
            break;
        case ORD_EDIT_FLUTE:
            sprintf(buf, "%s", order->FLUTE);
            break;
        case ORD_EDIT_TIRM:
            if (trim_flag) /* 有强制修边的标记 */
                order->TRIM = 1;
            if(opt & INPUT_DIALOG_PREPARE_OPT_EDITING){
                if(order->TRIM)
                    sprintf(buf, "1");
                else 
                    sprintf(buf, "0");
            } else {
                if(order->TRIM)
                    sprintf(buf, pick_string("修", "YES"));
                else 
                    sprintf(buf, pick_string("不修", "NO"));
            }
            break;
        default:
            break;
    }

    return;
}

static void ____alert(char * zh_str, char * en_str)
{
    char * s;

    s = pick_string(zh_str, en_str);
    input_dialog_alert(&edit_order_dialog, s, COLOR_WARNING_236);
}

/***************************************************************************************
** 返回值: 0=内容无效, 1=内容有效, -1=所有内容须刷新
*/
int finish_edit(int id, char * buf, void * data, KEYCODE key)
{
    order_struct * order;
    struct order_edit_private * temp;
    int    i;
    int    trim_flag;

    FamesAssert(buf);
    FamesAssert(data);
    if(!buf || !data)
        return 1; /* 如果缓冲无效, 那么就认为其内容总是有效的 */

    if(key == ESC)
        return 1;

    /* 计算修边标记 */
    trim_flag = slc_is_trim_forced();

    temp = (struct order_edit_private *)data;
    order = &temp->order;
    
    switch(id){
        case ORD_EDIT_WKNO:
            order->WORKNO = atoi(buf);
            i = FindOrder(order->WORKNO);
            if(i >= 0){
                LoadOrder(order, i, 1);
                return -1;
            }
            break;
        case ORD_EDIT_ORDNO:
            sprintf(order->ORDERNO, "%s", buf);
            break;
        case ORD_EDIT_CUTS:
            order->CUTS = atoi(buf);
            break;
        case ORD_EDIT_DATA:
            sprintf(order->SPECCUT, "%s", buf);
            break;
        case ORD_EDIT_YX:
            i = atoi(buf);
            if(i == 0)
                i = 1;
            if(i < 1 || i > get_yx_types())
                return 0;
            order->YX = i;
            break;
        case ORD_EDIT_FLUTE:
            sprintf(order->FLUTE, "%s", buf);
            break;
        case ORD_EDIT_TIRM:
            if(atoi(buf))
                order->TRIM = 1;
            else 
                order->TRIM = 0;
            if (trim_flag) /* 有强制修边标记 */
                order->TRIM = 1;
            break;
        default:
            break;
    }

    return 1;
}

/*-----------------------------------------------------------------------------------------
 * 函数:    order_edit()
 *
 * 描述:    订单编辑
**---------------------------------------------------------------------------------------*/
static struct order_edit_private order_for_edit = { INIT_order_struct };

BOOL order_edit(void)
{
    char x_buf[2048], ___s[64], ___s2[64];
    int  i, cuts;
    INT32U error;
    BOOL retval;
    int  loop;
    INT16U workno;
    slc_descriptor_t ___slc;
    extern struct slc_config_s config;

    order_for_edit.order.WORKNO = 10 + get_last_workno();

    gui_show_widget(input_screen);

    for(loop=1; loop;){
        input_again:
        retval = input_dialog_method(&edit_order_dialog, prepare_edit, finish_edit, &order_for_edit, 0);
        if(retval){
            /* 检查压线资料是否正确 */
            cuts = order_for_edit.order.CUTS;
            if(cuts<0)
                cuts = 1;
            MEMSET(x_buf, 0, sizeof(x_buf));
            STRCPY(x_buf, order_for_edit.order.SPECCUT);
            for(i=1; i<cuts; i++){
                x_buf[strlen(x_buf)]=SLC_K_TOKEN;
                strcat(x_buf, order_for_edit.order.SPECCUT);
                if(strlen(x_buf)+strlen(order_for_edit.order.SPECCUT) >= 500){
                    ____alert("压线资料太长!", "Data is too long!");
                    goto input_again;
                }
            }
            ___slc = config.slc[0];
            error = slc_locate(&___slc, x_buf, (order_for_edit.order.TRIM)?SLC_FLAG_TRIM:0);
            if(SLC_ERR_NONE != error){ /* 机1不能排, 那就试一下机2吧 */
                ___slc = config.slc[1];
                error = slc_locate(&___slc, x_buf, (order_for_edit.order.TRIM)?SLC_FLAG_TRIM:0);
            }
            if(SLC_ERR_NONE != error){
                sprintf(___s,  "压线有错误, 错误码: %08lX, %s", error, slc_error_message(error));
                sprintf(___s2, "Some Error, Code: %08lX, %s", error, slc_error_message(error));
                ____alert(___s, ___s2);
                continue;
            }
            /* 检查工号是否有效 */
            workno = order_for_edit.order.WORKNO;
            if(workno < get_first_workno()){
                ____alert("工号太小(应该大于第一笔单的工号)", "S/N is too small");
            } else {
                lock_kernel();
                retval = InsertOrder(&order_for_edit.order);
                unlock_kernel();
                if(!retval){
                    ____alert("插单失败!", "Failed to Insert!");
                    continue;
                } else {
                    loop = 0;
                    retval = ok;
                }
            }
        } else {
            loop = 0;
        }
    }

    gui_hide_widget(input_screen);

    return retval;
}

/*-----------------------------------------------------------------------------------------
 *          
 *      调单画面中的控件及其它定义
 * 
**---------------------------------------------------------------------------------------*/
static gui_widget * move_order_dialog = NULL;    /* 输单界面的主控件  */

enum {
    __old_workno,
    __new_workno,
};

/* 调单对话框的定义 */
DECLARE_INPUT_DIALOG_START(__move_order_dialog)
INPUT_DIALOG_ITEM("当前工号: ", __old_workno, 4,  64,  42,  256, 28, 1, 0, "")
INPUT_DIALOG_ITEM("新的工号: ", __new_workno, 4,  64,  80,  256, 28, 1, 0, "")
INPUT_DIALOG_SET(__move_order_dialog, "调单", &icon, 170, 418, 560, 200, 0, 236, 1, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE)
DECLARE_INPUT_DIALOG_ENDED(__move_order_dialog);

/*-----------------------------------------------------------------------------------------
 *          
 *      调单对话框的初始化
 * 
**---------------------------------------------------------------------------------------*/
gui_widget * init_move_order_dialog(void)
{
    __move_order_dialog.bkcolor = atoi(___bkcolor_other_dialog);
    input_dialog_preset_title(&__move_order_dialog, pick_string("调单", "Order Move"));
    input_dialog_preset_item_name(&__move_order_dialog, __old_workno, pick_string("当前工号: ", "Cur S/N: "));
    input_dialog_preset_item_name(&__move_order_dialog, __new_workno, pick_string("新的工号: ", "New S/N: "));
    input_dialog_preset_item_comment(&__move_order_dialog, __old_workno, 
                                     pick_string("\"当前工号\"可以选择将要移动的订单", "Current S/N indicates the Order to Move."));
    input_dialog_preset_item_comment(&__move_order_dialog, __new_workno, 
                                     pick_string("\"新的工号\"是订单将要移动到的位置", "New S/N indicates the Target Position to Move to."));
    move_order_dialog = input_dialog_initialize(&__move_order_dialog);
    input_dialog_set_buttons_caption(&__move_order_dialog, 
                                     pick_string("确认[F10]", "OK.[F10]"), 
                                     pick_string("取消[ESC]", "No.[ESC]"));
    gui_hide_widget(move_order_dialog);
    input_dialog_set_ok_comment(&__move_order_dialog, 
                                     pick_string("确认, 请按F10或回车键", "Press F10/Enter to Confirm"));
    input_dialog_set_cancel_comment(&__move_order_dialog, 
                                     pick_string("取消, 请按ESC或回车键", "Press ESC/Enter to Cancel"));
    return move_order_dialog;
}

struct ___struct {
    INT16U old_wkno;
    INT16U new_wkno;
};

static void ___prepare_move(int id, char * buf, void * data, INT16U opt)
{
    struct ___struct * t;
    
    FamesAssert(buf);
    FamesAssert(data);
    if(!buf || !data)
        return;

    opt = opt;

    t = (struct ___struct *)data;
    
    switch(id){
        case __old_workno:
            sprintf(buf, "%d", t->old_wkno);
            break;
        case __new_workno:
            sprintf(buf, "%d", t->new_wkno);
            break;
        default:
            break;
    }

    return;
}

static int ___finish_move(int id, char * buf, void * data, KEYCODE key)
{
    struct ___struct * t;
    
    FamesAssert(buf);
    FamesAssert(data);
    if(!buf || !data)
        return 1;

    if(key == ESC)
        return 1;

    t = (struct ___struct *)data;
    
    switch(id){
        case __old_workno:
            t->old_wkno = (INT16U)STRtoINT16(buf, CHG_OPT_DEC);
            break;
        case __new_workno:
            t->new_wkno = (INT16U)STRtoINT16(buf, CHG_OPT_DEC);
            if(key == ENTER){
                putkey(F10);
            }
            break;
        default:
            break;
    }

    return 1;
}

/*-----------------------------------------------------------------------------------------
 * 函数:    move_order()
 *
 * 描述:    调单
**---------------------------------------------------------------------------------------*/
BOOL move_order(void)
{
    BOOL retval;
    int  loop, i;
    struct ___struct t;
    order_struct order;
    extern gui_widget * main_order_view;

    t.old_wkno = get_first_workno();
    i = gui_view_get_selected(main_order_view);
    if(1 == LoadOrder(&order, i, 1)){
        t.old_wkno = order.WORKNO;
    }
    t.new_wkno = t.old_wkno;
    
    gui_show_widget(move_order_dialog);

    putkey(ENTER); /* 将光标跳到: "新的工号" */

    for(loop=1; loop;){
        retval = input_dialog_method(&__move_order_dialog, ___prepare_move, ___finish_move, (void*)&t, 0);
        if(retval){
            i = FindOrder(t.old_wkno);
            if(i < 0){
                input_dialog_alert(&__move_order_dialog, pick_string("订单不存在", "Order Not Exist!"), COLOR_WARNING_236);
                continue;
            }
            i = FindOrder(t.new_wkno);
            if(i >= 0){
                input_dialog_alert(&__move_order_dialog, 
                                   pick_string("新的工号已经存在, 请另外输入一个", 
                                               "New S/N already Exist, Choose Another One!"), 
                                   COLOR_WARNING_236);
                continue;
            }
            if(t.new_wkno < get_first_workno()){
                input_dialog_alert(&__move_order_dialog, 
                                   pick_string("新的工号太小, 请另外输入一个", 
                                               "New S/N is too small, Choose Another One!"),
                                   COLOR_WARNING_236);
                continue;
            }
            lock_kernel();
            retval = MoveOrder(t.old_wkno, t.new_wkno);
            unlock_kernel();
            if(!retval){
                input_dialog_alert(&__move_order_dialog, pick_string("调单失败!", "Failed!"), COLOR_WARNING_236);
                continue;
            } else {
                loop = 0;
                retval = ok;
            }
        } else {
            loop = 0;
        }
    }

    gui_hide_widget(move_order_dialog);

    return retval;
}


/*=========================================================================================
 * 
 * 本文件结束: slc/ord_edit.c
 * 
**=======================================================================================*/


