/******************************************************************************************
 * 文件:    slc/slc_send.c
 *
 * 描述:    订单发送界面
 *
 * 作者:    Jun
******************************************************************************************/
#define  SLC_SEND_ORDER_C
#include <FamesOS.h>
#include "common.h"

/*-----------------------------------------------------------------------------------------
 *          
 *      控件及其它定义
 * 
**---------------------------------------------------------------------------------------*/
gui_widget        * send_screen   = NULL;       /* 主控件      */
static gui_widget * status_bar    = NULL;       /* 状态条      */
static gui_widget * send_message  = NULL;       /* 送单信息    */

gui_widget * slc_send_movement    = NULL;       /* 进度条      */

extern BMPINFO  icon;                           /* 图标        */

#define  ___bkcolor  236                        /* 背景色      */


/*-----------------------------------------------------------------------------------------
 *          
 *      画面的定义(或初始化)
 * 
**---------------------------------------------------------------------------------------*/
void init_send_screen(void)
{
    int x, y, width, height;
    COLOR bkcolor;

    x = 220;
    y = 296;
    width = 500;
    height = 168;
    
    bkcolor = ___bkcolor;

    /* 主界面   */
    send_screen = gui_create_widget(GUI_WIDGET_FORM, x, y, width, height, 0, bkcolor, 1, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE);
    if(!send_screen)
        goto some_error;
    gui_form_init_private(send_screen, 128);
    gui_form_set_icon(send_screen, &icon);
    gui_form_set_caption(send_screen, pick_string("订单传送", "Send Order"));
    
    /* 工具条   */
    status_bar = gui_create_widget(GUI_WIDGET_LABEL, 5, (height-35), (width-10), 30, 0, bkcolor, 1, LABEL_STYLE_CLIENT_BDR);
    if(!status_bar)
        goto some_error;
    gui_widget_link(send_screen, status_bar);
    gui_label_init_private(status_bar, 128);
    gui_label_set_text(status_bar, "");

    /* 送单信息 */
    send_message = gui_create_widget(GUI_WIDGET_LABEL, 75, 50, 256, 52, 0, bkcolor, font24, 0);
    if(!send_message)
        goto some_error;
    gui_widget_link(send_screen, send_message);
    gui_label_init_private(send_message, 64);
    gui_label_set_text(send_message, "");

    /* ......   */
    slc_send_movement = gui_create_widget(GUI_WIDGET_LABEL, 333, 50, 96, 52, 0, bkcolor, font24, 0);
    if(!slc_send_movement)
        goto some_error;
    gui_widget_link(send_screen, slc_send_movement);
    gui_label_init_private(slc_send_movement, 16);
    gui_label_set_text(slc_send_movement, "");

    return;

some_error:
    sys_print("init_send_screen(): failed to create widgets!\n");
    ExitApplication();
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_send_order()
 *
 * 参数:    slc_index       分压机序号, 1 = SLC1, 2 = SLC2
 *          order           订单数据
 *          no_control      1 = 不发送刀线抬落信号与启动信号, 0 = 发送
 *
 * 描述:    传送订单到分压机(设定值)
 *
 * 说明:    由于此函数有可能被多个任务同时执行, 所以需要互斥处理(___lock)
**---------------------------------------------------------------------------------------*/
BOOL slc_send_order(int slc_index, order_struct * order, int no_control)
{
    slc_descriptor_t * slc;
    char x_buf[1024], ___s[128];
    int  i, cuts;
    INT16U k_mask, l_mask, kl_mask; /* 刀线选中状态 */
    INT32U error;
    BOOL   retval;
    INT16U locate_flag = 0;
    static gui_widget * old, * old2;
    extern int plc_connected[];
    static int ___lock = 0;

    FamesAssert(slc_index == 1 || slc_index == 2);
    FamesAssert(order);

    if(slc_index != 1 && slc_index != 2)
        return fail;
    if(!order)
        return fail;

    retval = fail;

    os_mutex_lock(___lock); /* 本函数须互斥执行 */

    old = gui_get_root_widget();
    gui_put_root_widget();
    sprintf(___s, "%s [%d]", pick_string("正 在 传 送 到 机", "Sending Order to"), slc_index);
    gui_label_set_text(send_message, ___s);
    gui_label_set_text(status_bar, pick_string("将订单数据发送到PLC", "Sending Order"));
    ___gui_set_root_widget(send_screen);

    slc = &config.slc[slc_index-1];

    cuts = order->CUTS;
    if(cuts<0)
        cuts = 1;
    MEMSET(x_buf, 0, sizeof(x_buf));
    STRCPY(x_buf, order->SPECCUT);
    for(i=1; i<cuts; i++){
        x_buf[strlen(x_buf)]=SLC_K_TOKEN;
        strcat(x_buf, order->SPECCUT);
        if(strlen(x_buf)+strlen(order->SPECCUT) >= 1024){
            goto out;
        }
    }
    locate_flag = (config.slc_reverse_mode ? SLC_FLAG_RVSE : 0);
    error = slc_locate(slc, x_buf, locate_flag|((order->TRIM)?SLC_FLAG_TRIM:0));
    if(error == SLC_ERR_NONE){
        int  deep, fan;

        /* 先起刀线 */
        if (!no_control) {
            slc_clear_fix_ok(slc_index);
            slc_kl_up_set(slc_index); /* 刀线上 */
        }

        /* 发送设定值到PLC */
        slc_clear_fix_ok(slc_index); /* 清除定位完成信号 */

        /* 计算刀线的选中情况, 并存放到k_mask, l_mask中 */
        k_mask = 0;
        l_mask = 0;
        for(i=0; i<slc->k_number; i++){
            if (slc->kl_set.k_selected[i])
                k_mask |= (1 << i);
        }
        for(i=0; i<slc->l_number; i++){
            if (slc->kl_set.l_selected[i])
                l_mask |= (1 << i);
        }
        /* FIXME: 启动模式应该还有其它的功能, 而不只是控制排单 */
        if ((config.slc_start_mode == 0) || (k_mask != 0)) {
            for(i=0; i<slc->k_number; i++){
                long temp;
                temp = slc->kl_set.k_location[i];
                ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_SET_VALUE_K[i], &temp, 1);
                ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M,  PLC_ADDR_SELECTED_K[i],  &(slc->kl_set.k_selected[i]), 1);
            }
        } else { /* 设定值 <- 实际值 */
            for(i=0; i<slc->k_number; i++){
                int temp;
                temp = slc->kl_act.k_location[i];
                slc->kl_set.k_location[i] = temp;
            }
        }
        /* FIXME: 启动模式应该还有其它的功能, 而不只是控制排单 */
        if ((config.slc_start_mode == 0) || (l_mask != 0)) {
            for(i=0; i<slc->l_number; i++){
                long temp;
                temp = slc->kl_set.l_location[i];
                ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_SET_VALUE_L[i], &temp, 1);
                ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M,  PLC_ADDR_SELECTED_L[i],  &(slc->kl_set.l_selected[i]), 1);
            }
        } else { /* 设定值 <- 实际值 */
            for(i=0; i<slc->l_number; i++){
                int temp;
                temp = slc->kl_act.l_location[i];
                slc->kl_set.l_location[i] = temp;
            }
        }
#ifdef USE__PLC_ADDR_SELECTED_KL
        kl_mask  = (l_mask << slc->k_number);
        kl_mask |= (k_mask);
        ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_R, PLC_ADDR_SELECTED_KL, &kl_mask, 1);
#endif

        slc->kl_set.press_type = order->YX;
        slc_set_yx(slc_index, order->YX);     /* 压型设定值 */
        deep = get_deep_value(order->FLUTE, order->YX);
        slc->kl_set.press_1_location = deep;
        slc->kl_set.press_2_location = deep;
        slc_send_deep_value(slc_index, deep); /* 楞别深浅设定值 */
        for(i=0; i<slc->k_number; i++){
            if(slc->kl_set.k_selected[i]){
                fan = slc->kl_set.k_location[i];
                slc->kl_set.fan_location = fan;
                if(tmp_config.auto_fan) {
                    slc_send_fan_value(slc_index, fan); /* 吸风口位置设定值 */
                }
                break;
            }
        }

        if (!no_control) {
            slc_kl_up_set(slc_index); /* 刀线上 */
            slc_send_start(slc_index); /* 启动 */
        }

        /* 保存订单到描述符 */
        slc->working = *order;

        /* 初始化实际值为设定值 */
        slc->kl_act = slc->kl_set;

        if(plc_connected[slc_index-1]){
            retval = ok;
        } else {
            sprintf(___s, pick_string("传送失败: PLC没有联线!", "Failed: PLC is Off-Line!"));
            gui_label_set_text(status_bar, ___s);
            gui_set_widget_bkcolor(status_bar, COLOR_WARNING_236);
            waitkey(2000);
            gui_set_widget_bkcolor(status_bar, ___bkcolor);
        }

        save_config(); /* 保存订单信息到全局配置文件 */
    } else { /* 显示错误信息 */
        sprintf(___s, pick_string("订单有错误: %s[%08lX]", "Error: %s[%08lX]"), 
                      slc_error_message(error), error);
        gui_label_set_text(status_bar, ___s);
        gui_set_widget_bkcolor(status_bar, COLOR_WARNING_236);
        waitkey(2000);
        gui_set_widget_bkcolor(status_bar, ___bkcolor);
        /* 即使订单有错误, 也需要保存订单到描述符
         * 因为如果不这样的话, 将无法继续换单.
         */
        slc->working = *order;
    }

out:
    old2 = gui_get_root_widget();
    gui_put_root_widget();
    if(old2 == send_screen){
        ___gui_set_root_widget(old);
    }

    os_mutex_unlock(___lock);

    return retval;
}

/*-----------------------------------------------------------------------------------------
 * 函数:    reset_kl_on_quit()
 *
 * 描述:    程序退出时, 刀线需要复位
**---------------------------------------------------------------------------------------*/
void reset_kl_on_quit(void)
{
    int i, slc_index, tmp;
    slc_descriptor_t * slc;

    tmp = 0;

    for(slc_index = 1; slc_index <= 2; slc_index++){
        slc = &config.slc[slc_index-1];

        for(i=0; i<slc->k_number; i++){
            ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_SELECTED_K[i], &tmp, 1);
        }
        for(i=0; i<slc->l_number; i++){
            ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_SELECTED_L[i], &tmp, 1);
        }
    }
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_read_act_value()
 *
 * 描述:    从PLC读取刀线实际值
**---------------------------------------------------------------------------------------*/
void slc_read_act_value(int slc_index)
{
    slc_descriptor_t * slc;
    int   i;
    long  tmp[32];

    FamesAssert(slc_index == 1 || slc_index == 2);

    if(slc_index != 1 && slc_index != 2)
        return;

    slc = &config.slc[slc_index-1];

    for(i = 0; i<slc->k_number; i++)    /* 刀实际值 */
        tmp[i] = (long)slc->kl_act.k_location[i];
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_ACT_VALUE_K, tmp, slc->k_number);
    for(i = 0; i<slc->k_number; i++)
        slc->kl_act.k_location[i] = (int)tmp[i];

    for(i = 0; i<slc->l_number; i++)    /* 线实际值 */
        tmp[i] = (long)slc->kl_act.l_location[i];
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_ACT_VALUE_L, tmp, slc->l_number);
    for(i = 0; i<slc->l_number; i++)
        slc->kl_act.l_location[i] = (int)tmp[i];

    tmp[0] = (long)slc->kl_act.press_1_location; /* 前压 */
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_ACT_PRESS1, tmp, 1);
    slc->kl_act.press_1_location = (int)tmp[0];

    if (config.slc[slc_index-1].slc_type == SLC_TYPE_DOUBLE) {
        tmp[0] = (long)slc->kl_act.press_2_location; /* 后压 */
        ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_ACT_PRESS2, tmp, 1);
        slc->kl_act.press_2_location = (int)tmp[0];
    }

    tmp[0] = (long)slc->kl_act.fan_location;     /* 吸风口 */
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_ACT_FAN, tmp, 1);
    slc->kl_act.fan_location = (int)tmp[0];
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_send_kl_act_value()
 *
 * 描述:    发送校正值给PLC
**---------------------------------------------------------------------------------------*/
void slc_send_kl_act_value(int slc_index)
{
    slc_descriptor_t * slc;
    int   i;
    int   ttt[32];
    long  tmp[32];

    FamesAssert(slc_index == 1 || slc_index == 2);

    if(slc_index != 1 && slc_index != 2)
        return;

    slc = &config.slc[slc_index-1];

    for(i = 0; i<slc->k_number; i++) {
        tmp[i] = (long)slc->kl_adjust.k_location[i];
        ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_ADJUST_K[i], &tmp[i], 1);
    }

    for(i = 0; i<slc->l_number; i++) {
        tmp[i] = (long)slc->kl_adjust.l_location[i];
        ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_ADJUST_L[i], &tmp[i], 1);
    }

    for(i = 0; i < 32; i++)  /* 先置1  */
        ttt[i] = 1;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_ADJ_K_CFM, ttt, slc->k_number);
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_ADJ_L_CFM, ttt, slc->l_number);

    #if 0
    TaskDelay(200);
    for(i = 0; i < 32; i++)  /* 再清零 */
        ttt[i] = 0;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_ADJ_K_CFM, ttt, slc->k_number);
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_ADJ_L_CFM, ttt, slc->l_number);
    #endif
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_read_regress_value()
 *
 * 描述:    从PLC读取刀线归零值
**---------------------------------------------------------------------------------------*/
void slc_read_regress_value(int slc_index)
{
    slc_descriptor_t * slc;
    int   i;
    long  tmp[32];

    FamesAssert(slc_index == 1 || slc_index == 2);

    if(slc_index != 1 && slc_index != 2)
        return;

    slc = &config.slc[slc_index-1];

    for(i = 0; i<slc->k_number; i++)
        tmp[i] = (long)slc->kl_regress.k_location[i];
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_REGRESS_K, tmp, slc->k_number);
    for(i = 0; i<slc->k_number; i++)
        slc->kl_regress.k_location[i] = (int)tmp[i];

    for(i = 0; i<slc->l_number; i++)
        tmp[i] = (long)slc->kl_regress.l_location[i];
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_REGRESS_L, tmp, slc->l_number);
    for(i = 0; i<slc->l_number; i++)
        slc->kl_regress.l_location[i] = (int)tmp[i];
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_send_regress_value()
 *
 * 描述:    发送归零值给PLC
**---------------------------------------------------------------------------------------*/
void slc_send_regress_value(int slc_index)
{
    slc_descriptor_t * slc;
    int   i;
    long  tmp[32];

    FamesAssert(slc_index == 1 || slc_index == 2);

    if(slc_index != 1 && slc_index != 2)
        return;

    slc = &config.slc[slc_index-1];

    for(i = 0; i<slc->k_number; i++)
        tmp[i] = (long)slc->kl_regress.k_location[i];
    ___slc_plc_rw(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_REGRESS_K, tmp, slc->k_number);

    for(i = 0; i<slc->l_number; i++)
        tmp[i] = (long)slc->kl_regress.l_location[i];
    ___slc_plc_rw(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_REGRESS_L, tmp, slc->l_number);
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_read_kl_unit_value()
 *
 * 描述:    从PLC读取刀线归零值
**---------------------------------------------------------------------------------------*/
void slc_read_kl_unit_value(int slc_index)
{
    slc_descriptor_t * slc;
    int   i;
    long  tmp[32];

    FamesAssert(slc_index == 1 || slc_index == 2);

    if(slc_index != 1 && slc_index != 2)
        return;

    slc = &config.slc[slc_index-1];

    for(i = 0; i<slc->k_number; i++)
        tmp[i] = (long)slc->unit.k_unit[i];
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_UNIT_K, tmp, slc->k_number);
    for(i = 0; i<slc->k_number; i++)
        slc->unit.k_unit[i] = (int)tmp[i];

    for(i = 0; i<slc->l_number; i++) {
        tmp[i] = (long)slc->unit.l_unit[i];
        ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_UNIT_L[i], &tmp[i], 1);
    }
    for(i = 0; i<slc->l_number; i++)
        slc->unit.l_unit[i] = (int)tmp[i];
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_send_kl_unit_value()
 *
 * 描述:    发送刀线单位值给PLC
**---------------------------------------------------------------------------------------*/
void slc_send_kl_unit_value(int slc_index)
{
    slc_descriptor_t * slc;
    int   i;
    long  tmp[32];

    FamesAssert(slc_index == 1 || slc_index == 2);

    if(slc_index != 1 && slc_index != 2)
        return;

    slc = &config.slc[slc_index-1];

    /*FIXME: 写一个总单位值, 这主要是为了与JY524兼容 */
    ___slc_plc_rw(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_UNIT_ALL, tmp, slc->k_number);

    for(i = 0; i<slc->k_number; i++)
        tmp[i] = (long)slc->unit.k_unit[i];
    ___slc_plc_rw(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_UNIT_K, tmp, slc->k_number);

    for(i = 0; i<slc->l_number; i++) {
        tmp[i] = (long)slc->unit.l_unit[i];
        ___slc_plc_rw(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_UNIT_L[i], &tmp[i], 1);
    }
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_read_kl_fix_value()
 *
 * 描述:    从PLC读取刀线定位值
**---------------------------------------------------------------------------------------*/
void slc_read_kl_fix_value(int slc_index)
{
    slc_descriptor_t * slc;
    int   i;
    long  tmp[32];
    extern PLC * slc_plc[];

    FamesAssert(slc_index == 1 || slc_index == 2);

    if(slc_index != 1 && slc_index != 2)
        return;

    slc_index--;
    
    slc = &config.slc[slc_index];

    if(slc_plc[slc_index]){
        for(i = 0; i<slc->k_number; i++)
            tmp[i] = 0L;
        plc_rw(slc_plc[slc_index], FATEK_PLC_READ_DR, PLC_ADDR_FIX_K, tmp, slc->k_number);
        for(i = 0; i<slc->k_number; i++)
            slc->fix_set.k_fix_set[i] = (int)tmp[i];

        for(i = 0; i<slc->l_number; i++) {
            tmp[i] = 0L;
            plc_rw(slc_plc[slc_index], FATEK_PLC_READ_DR, PLC_ADDR_FIX_L[i], &tmp[i], 1);
        }
        for(i = 0; i<slc->l_number; i++)
            slc->fix_set.l_fix_set[i] = (int)tmp[i];
    }
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_send_kl_fix_value()
 *
 * 描述:    发送刀线定位值给PLC
**---------------------------------------------------------------------------------------*/
void slc_send_kl_fix_value(int slc_index)
{
    slc_descriptor_t * slc;
    int   i;
    long  tmp[32];
    extern PLC * slc_plc[];

    FamesAssert(slc_index == 1 || slc_index == 2);

    if(slc_index != 1 && slc_index != 2)
        return;

    slc_index--;
    
    slc = &config.slc[slc_index];

    if(slc_plc[slc_index]){
        for(i = 0; i<slc->k_number; i++)
            tmp[i] = (long)slc->fix_set.k_fix_set[i];
        plc_rw(slc_plc[slc_index], FATEK_PLC_WRITE_DR, PLC_ADDR_FIX_K, tmp, slc->k_number);

        for(i = 0; i<slc->l_number; i++) {
            tmp[i] = (long)slc->fix_set.l_fix_set[i];
            plc_rw(slc_plc[slc_index], FATEK_PLC_WRITE_DR, PLC_ADDR_FIX_L[i], &tmp[i], 1);
        }
    }
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_set_yx()
 *
 * 描述:    压型设定值
**---------------------------------------------------------------------------------------*/
void slc_set_yx(int slc_index, int yx)
{
    int i, tmp[16];
    extern struct slc_tmp_config_s tmp_config;
    
    for(i = 0; i < 16; i++){
        if((yx-1) == i)
            tmp[i] = 1;
        else
            tmp[i] = 0;
    }
    if(tmp_config.auto_yx && (get_yx_types() > 0))
        ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_YX, tmp, get_yx_types());
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_send_deep_value()
 *
 * 描述:    发送压深设定值
**---------------------------------------------------------------------------------------*/
void slc_send_deep_value(int slc_index, int deep)
{
    INT32S tmp;

    if (slc_index != 1 && slc_index != 2)
        return;

    tmp = (INT32S)deep;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_SET_PRESS1, &tmp, 1);
    if (config.slc[slc_index-1].slc_type == SLC_TYPE_DOUBLE) {
        ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_SET_PRESS2, &tmp, 1);
    }
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_send_fan_value()
 *
 * 描述:    发送吸风口设定值
**---------------------------------------------------------------------------------------*/
void slc_send_fan_value(int slc_index, int fan)
{
    INT32S tmp;

    tmp = (INT32S)fan;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_SET_FAN, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_kl_down_set()
 *
 * 描述:    发送刀线下命令
**---------------------------------------------------------------------------------------*/
void slc_kl_down_set(int slc_index)
{
    int tmp = 1;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_KL_DN, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_kl_down_reset()
 *
 * 描述:    复位刀线下命令
**---------------------------------------------------------------------------------------*/
void slc_kl_down_reset(int slc_index)
{
    int tmp = 0;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_KL_DN, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_kl_up_set()
 *
 * 描述:    发送刀线上命令
**---------------------------------------------------------------------------------------*/
void slc_kl_up_set(int slc_index)
{
    int tmp = 1;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_KL_UP, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_kl_up_reset()
 *
 * 描述:    复位刀线上命令
**---------------------------------------------------------------------------------------*/
void slc_kl_up_reset(int slc_index)
{
    int tmp = 0;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_KL_UP, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_send_start()
 *
 * 描述:    发送启动命令
**---------------------------------------------------------------------------------------*/
void slc_send_start(int slc_index)
{
    int tmp = 1;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_START, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * 函数:    slc_clear_fix_ok()
 *
 * 描述:    清除定位完成信号
**---------------------------------------------------------------------------------------*/
void slc_clear_fix_ok(int slc_index)
{
    int tmp = 0;

    #if 0 /* 测试 */
    tmp = 1;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, "X0063", &tmp, 1);
    tmp = 0;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, "X0063", &tmp, 1);
    #endif

    tmp = 0;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_CLR_FIX, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * 函数:    set_plc_on_startup()
 *
 * 描述:    分压机启动时对PLC的设定
**---------------------------------------------------------------------------------------*/
void set_plc_on_startup(int slc_index)
{
    INT32S tmp;
    extern struct slc_config_s config;

    tmp = (INT32S)(config.slc[slc_index-1].slc_spg);
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_SPG, &tmp, 1);
    tmp = (INT32S)(config.slc[slc_index-1].misc.yx_big);
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_YXBIG, &tmp, 1);
    tmp = (INT32S)(config.slc[slc_index-1].misc.yx_small);
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_YXSMALL, &tmp, 1);

    slc_clear_fix_ok(slc_index); /* 开机时清除定位完成信号 */
}


/*=========================================================================================
 * 
 * 本文件结束: slc/slc_send.c
 * 
**=======================================================================================*/


