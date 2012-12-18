/******************************************************************************************
 * �ļ�:    slc/slc_send.c
 *
 * ����:    �������ͽ���
 *
 * ����:    Jun
******************************************************************************************/
#define  SLC_SEND_ORDER_C
#include <FamesOS.h>
#include "common.h"

/*-----------------------------------------------------------------------------------------
 *          
 *      �ؼ�����������
 * 
**---------------------------------------------------------------------------------------*/
gui_widget        * send_screen   = NULL;       /* ���ؼ�      */
static gui_widget * status_bar    = NULL;       /* ״̬��      */
static gui_widget * send_message  = NULL;       /* �͵���Ϣ    */

gui_widget * slc_send_movement    = NULL;       /* ������      */

extern BMPINFO  icon;                           /* ͼ��        */

#define  ___bkcolor  236                        /* ����ɫ      */


/*-----------------------------------------------------------------------------------------
 *          
 *      ����Ķ���(���ʼ��)
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

    /* ������   */
    send_screen = gui_create_widget(GUI_WIDGET_FORM, x, y, width, height, 0, bkcolor, 1, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE);
    if(!send_screen)
        goto some_error;
    gui_form_init_private(send_screen, 128);
    gui_form_set_icon(send_screen, &icon);
    gui_form_set_caption(send_screen, pick_string("��������", "Send Order"));
    
    /* ������   */
    status_bar = gui_create_widget(GUI_WIDGET_LABEL, 5, (height-35), (width-10), 30, 0, bkcolor, 1, LABEL_STYLE_CLIENT_BDR);
    if(!status_bar)
        goto some_error;
    gui_widget_link(send_screen, status_bar);
    gui_label_init_private(status_bar, 128);
    gui_label_set_text(status_bar, "");

    /* �͵���Ϣ */
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
 * ����:    slc_send_order()
 *
 * ����:    slc_index       ��ѹ�����, 1 = SLC1, 2 = SLC2
 *          order           ��������
 *          no_control      1 = �����͵���̧���ź��������ź�, 0 = ����
 *
 * ����:    ���Ͷ�������ѹ��(�趨ֵ)
 *
 * ˵��:    ���ڴ˺����п��ܱ��������ͬʱִ��, ������Ҫ���⴦��(___lock)
**---------------------------------------------------------------------------------------*/
BOOL slc_send_order(int slc_index, order_struct * order, int no_control)
{
    slc_descriptor_t * slc;
    char x_buf[1024], ___s[128];
    int  i, cuts;
    INT16U k_mask, l_mask, kl_mask; /* ����ѡ��״̬ */
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

    os_mutex_lock(___lock); /* �������뻥��ִ�� */

    old = gui_get_root_widget();
    gui_put_root_widget();
    sprintf(___s, "%s [%d]", pick_string("�� �� �� �� �� ��", "Sending Order to"), slc_index);
    gui_label_set_text(send_message, ___s);
    gui_label_set_text(status_bar, pick_string("���������ݷ��͵�PLC", "Sending Order"));
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

        /* ������ */
        if (!no_control) {
            slc_clear_fix_ok(slc_index);
            slc_kl_up_set(slc_index); /* ������ */
        }

        /* �����趨ֵ��PLC */
        slc_clear_fix_ok(slc_index); /* �����λ����ź� */

        /* ���㵶�ߵ�ѡ�����, ����ŵ�k_mask, l_mask�� */
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
        /* FIXME: ����ģʽӦ�û��������Ĺ���, ����ֻ�ǿ����ŵ� */
        if ((config.slc_start_mode == 0) || (k_mask != 0)) {
            for(i=0; i<slc->k_number; i++){
                long temp;
                temp = slc->kl_set.k_location[i];
                ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_SET_VALUE_K[i], &temp, 1);
                ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M,  PLC_ADDR_SELECTED_K[i],  &(slc->kl_set.k_selected[i]), 1);
            }
        } else { /* �趨ֵ <- ʵ��ֵ */
            for(i=0; i<slc->k_number; i++){
                int temp;
                temp = slc->kl_act.k_location[i];
                slc->kl_set.k_location[i] = temp;
            }
        }
        /* FIXME: ����ģʽӦ�û��������Ĺ���, ����ֻ�ǿ����ŵ� */
        if ((config.slc_start_mode == 0) || (l_mask != 0)) {
            for(i=0; i<slc->l_number; i++){
                long temp;
                temp = slc->kl_set.l_location[i];
                ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_SET_VALUE_L[i], &temp, 1);
                ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M,  PLC_ADDR_SELECTED_L[i],  &(slc->kl_set.l_selected[i]), 1);
            }
        } else { /* �趨ֵ <- ʵ��ֵ */
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
        slc_set_yx(slc_index, order->YX);     /* ѹ���趨ֵ */
        deep = get_deep_value(order->FLUTE, order->YX);
        slc->kl_set.press_1_location = deep;
        slc->kl_set.press_2_location = deep;
        slc_send_deep_value(slc_index, deep); /* �����ǳ�趨ֵ */
        for(i=0; i<slc->k_number; i++){
            if(slc->kl_set.k_selected[i]){
                fan = slc->kl_set.k_location[i];
                slc->kl_set.fan_location = fan;
                if(tmp_config.auto_fan) {
                    slc_send_fan_value(slc_index, fan); /* �����λ���趨ֵ */
                }
                break;
            }
        }

        if (!no_control) {
            slc_kl_up_set(slc_index); /* ������ */
            slc_send_start(slc_index); /* ���� */
        }

        /* ���涩���������� */
        slc->working = *order;

        /* ��ʼ��ʵ��ֵΪ�趨ֵ */
        slc->kl_act = slc->kl_set;

        if(plc_connected[slc_index-1]){
            retval = ok;
        } else {
            sprintf(___s, pick_string("����ʧ��: PLCû������!", "Failed: PLC is Off-Line!"));
            gui_label_set_text(status_bar, ___s);
            gui_set_widget_bkcolor(status_bar, COLOR_WARNING_236);
            waitkey(2000);
            gui_set_widget_bkcolor(status_bar, ___bkcolor);
        }

        save_config(); /* ���涩����Ϣ��ȫ�������ļ� */
    } else { /* ��ʾ������Ϣ */
        sprintf(___s, pick_string("�����д���: %s[%08lX]", "Error: %s[%08lX]"), 
                      slc_error_message(error), error);
        gui_label_set_text(status_bar, ___s);
        gui_set_widget_bkcolor(status_bar, COLOR_WARNING_236);
        waitkey(2000);
        gui_set_widget_bkcolor(status_bar, ___bkcolor);
        /* ��ʹ�����д���, Ҳ��Ҫ���涩����������
         * ��Ϊ����������Ļ�, ���޷���������.
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
 * ����:    reset_kl_on_quit()
 *
 * ����:    �����˳�ʱ, ������Ҫ��λ
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
 * ����:    slc_read_act_value()
 *
 * ����:    ��PLC��ȡ����ʵ��ֵ
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

    for(i = 0; i<slc->k_number; i++)    /* ��ʵ��ֵ */
        tmp[i] = (long)slc->kl_act.k_location[i];
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_ACT_VALUE_K, tmp, slc->k_number);
    for(i = 0; i<slc->k_number; i++)
        slc->kl_act.k_location[i] = (int)tmp[i];

    for(i = 0; i<slc->l_number; i++)    /* ��ʵ��ֵ */
        tmp[i] = (long)slc->kl_act.l_location[i];
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_ACT_VALUE_L, tmp, slc->l_number);
    for(i = 0; i<slc->l_number; i++)
        slc->kl_act.l_location[i] = (int)tmp[i];

    tmp[0] = (long)slc->kl_act.press_1_location; /* ǰѹ */
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_ACT_PRESS1, tmp, 1);
    slc->kl_act.press_1_location = (int)tmp[0];

    if (config.slc[slc_index-1].slc_type == SLC_TYPE_DOUBLE) {
        tmp[0] = (long)slc->kl_act.press_2_location; /* ��ѹ */
        ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_ACT_PRESS2, tmp, 1);
        slc->kl_act.press_2_location = (int)tmp[0];
    }

    tmp[0] = (long)slc->kl_act.fan_location;     /* ����� */
    ___slc_plc_rw(slc_index, FATEK_PLC_READ_DR, PLC_ADDR_ACT_FAN, tmp, 1);
    slc->kl_act.fan_location = (int)tmp[0];
}

/*-----------------------------------------------------------------------------------------
 * ����:    slc_send_kl_act_value()
 *
 * ����:    ����У��ֵ��PLC
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

    for(i = 0; i < 32; i++)  /* ����1  */
        ttt[i] = 1;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_ADJ_K_CFM, ttt, slc->k_number);
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_ADJ_L_CFM, ttt, slc->l_number);

    #if 0
    TaskDelay(200);
    for(i = 0; i < 32; i++)  /* ������ */
        ttt[i] = 0;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_ADJ_K_CFM, ttt, slc->k_number);
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_ADJ_L_CFM, ttt, slc->l_number);
    #endif
}

/*-----------------------------------------------------------------------------------------
 * ����:    slc_read_regress_value()
 *
 * ����:    ��PLC��ȡ���߹���ֵ
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
 * ����:    slc_send_regress_value()
 *
 * ����:    ���͹���ֵ��PLC
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
 * ����:    slc_read_kl_unit_value()
 *
 * ����:    ��PLC��ȡ���߹���ֵ
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
 * ����:    slc_send_kl_unit_value()
 *
 * ����:    ���͵��ߵ�λֵ��PLC
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

    /*FIXME: дһ���ܵ�λֵ, ����Ҫ��Ϊ����JY524���� */
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
 * ����:    slc_read_kl_fix_value()
 *
 * ����:    ��PLC��ȡ���߶�λֵ
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
 * ����:    slc_send_kl_fix_value()
 *
 * ����:    ���͵��߶�λֵ��PLC
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
 * ����:    slc_set_yx()
 *
 * ����:    ѹ���趨ֵ
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
 * ����:    slc_send_deep_value()
 *
 * ����:    ����ѹ���趨ֵ
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
 * ����:    slc_send_fan_value()
 *
 * ����:    ����������趨ֵ
**---------------------------------------------------------------------------------------*/
void slc_send_fan_value(int slc_index, int fan)
{
    INT32S tmp;

    tmp = (INT32S)fan;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_DR, PLC_ADDR_SET_FAN, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * ����:    slc_kl_down_set()
 *
 * ����:    ���͵���������
**---------------------------------------------------------------------------------------*/
void slc_kl_down_set(int slc_index)
{
    int tmp = 1;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_KL_DN, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * ����:    slc_kl_down_reset()
 *
 * ����:    ��λ����������
**---------------------------------------------------------------------------------------*/
void slc_kl_down_reset(int slc_index)
{
    int tmp = 0;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_KL_DN, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * ����:    slc_kl_up_set()
 *
 * ����:    ���͵���������
**---------------------------------------------------------------------------------------*/
void slc_kl_up_set(int slc_index)
{
    int tmp = 1;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_KL_UP, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * ����:    slc_kl_up_reset()
 *
 * ����:    ��λ����������
**---------------------------------------------------------------------------------------*/
void slc_kl_up_reset(int slc_index)
{
    int tmp = 0;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_KL_UP, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * ����:    slc_send_start()
 *
 * ����:    ������������
**---------------------------------------------------------------------------------------*/
void slc_send_start(int slc_index)
{
    int tmp = 1;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_START, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * ����:    slc_clear_fix_ok()
 *
 * ����:    �����λ����ź�
**---------------------------------------------------------------------------------------*/
void slc_clear_fix_ok(int slc_index)
{
    int tmp = 0;

    #if 0 /* ���� */
    tmp = 1;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, "X0063", &tmp, 1);
    tmp = 0;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, "X0063", &tmp, 1);
    #endif

    tmp = 0;
    ___slc_plc_rw_ensure(slc_index, FATEK_PLC_WRITE_M, PLC_ADDR_CLR_FIX, &tmp, 1);
}

/*-----------------------------------------------------------------------------------------
 * ����:    set_plc_on_startup()
 *
 * ����:    ��ѹ������ʱ��PLC���趨
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

    slc_clear_fix_ok(slc_index); /* ����ʱ�����λ����ź� */
}


/*=========================================================================================
 * 
 * ���ļ�����: slc/slc_send.c
 * 
**=======================================================================================*/


