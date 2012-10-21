/*************************************************************************************
 * 文件:    slc/cim_link.c
 *
 * 描述:    生管连线
 *
 * 作者:    Jun
 *
 * 时间:    2011-4-5
*************************************************************************************/
#define  SLC_CIM_LINK_C
#include <includes.h>
#include "common.h"

#define  DEBUG  0

/*------------------------------------------------------------------------------------
 *
 *          当前的生管连线协议
 *
**----------------------------------------------------------------------------------*/
cim_link_type * cim_link_protocol = NULL;

static int cim_link_handle = InvalidHandle;

static int cim_link_is_opened = 0;

static SERIAL_PORT cim_link_port;

static INT08U * cim_link_port_rx_buf;
static INT08U * cim_link_port_tx_buf;

/*------------------------------------------------------------------------------------
 *
 *          卡达标准协议定义
 *
**----------------------------------------------------------------------------------*/
typedef struct __proto_kd_std_data_s {
    char  cuts[1];                      /* 剖数 */
    char  cutwidth[4];                  /* 裁宽 */
    char  speccut[5][4];                /* 压线 */
    char  edge[1];                      /* 修边 */
} __proto_kd_std_data_t;

typedef struct  __proto_kd_std_order_s {
    char  orderno[7];                   /* 订单编号 */
    char  flute[1];                     /* 楞别 */
    char  press[1];                     /* 压型 */
    char  cutlen[4];                    /* 切长 */
    __proto_kd_std_data_t speccut[3];   /* 压线资料 */	
} __proto_kd_std_order_t;

typedef struct __proto_kd_std_s{        /* 卡达标准协议 */
    INT16U stx;                         /* 开头的两个字节，0x02, 0x4f, 即0x4f02 */
    __proto_kd_std_order_t order[5];
    char checksum[2];                   /* 校验码 */
    char etx[1];
} __proto_kd_std_t;

static BOOL kd_std_is_stx(INT08S ch)
{
    if(ch == 0x2)
        return ok;
    return fail;
}
static BOOL kd_std_is_etx(INT08S ch)
{
    if(ch == 0x3)
        return ok;
    return fail;
}
static BOOL kd_std_received_notifier(void)
{
    return ok;
}
static BOOL kd_std_is_received(void)
{
    BOOL rxed;
    prepare_atomic()

    in_atomic();
    rxed = cim_link_protocol->received;
    out_atomic();

    if(rxed)
        return ok;
    return fail;
}
static BOOL kd_std_checksum_ok(void * data)
{
    int i;
    INT08U ab, a, b, * t;

    t = (INT08U*)data;

    ab = 0;
    for(i=1; i<457; i++)
        ab += t[i];
    ab = -ab; /*取补*/
    a = HEXtoCHAR(ab>>4);
    b = HEXtoCHAR(ab&0xf);
    #if DEBUG == 1
    lock_kernel();
    printf("a=%c, b=%c, t[457]=%c, t[458]=%c\n", a,b,t[457],t[458]);
    unlock_kernel();
    #endif
    if(t[457] == a &&
       t[458] == b){
        return ok;
    } else {
        return fail;
    }
}
static BOOL fms300_checksum_ok(void * data)
{
    int i;
    INT08U ab, a, b, * t;

    t = (INT08U*)data;

    ab = 0;
    for(i=1; i<457; i++)
        ab += t[i];
    a = ~ab; /*取反*/
    b = 0x20;
    a = a-b;
    if(a < b){
        a -= b;
        b += b;
    }
    #if DEBUG == 1
    lock_kernel();
    printf("a=%c, b=%c, t[457]=%c, t[458]=%c\n", a,b,t[457],t[458]);
    unlock_kernel();
    #endif
    if(t[457] == a &&
       t[458] == b){
        return ok;
    } else {
        return fail;
    }
}

void ____combine_speccut_data(char * buf, int buf_len, char * s1, int cuts1, char * s2, int cuts2)
{
    char x_buf[128];
    int  i, len;

    FamesAssert(buf);
    FamesAssert(s1 && s2);
    FamesAssert(cuts1 > 0 && cuts2 > 0);
    if(!buf || !s1 || !s2)
        return;
    if(cuts1 <= 0 || cuts2 <= 0)
        return;
    
    MEMSET(x_buf, 0, sizeof(x_buf));
    STRCPY(x_buf, s1);
    for(i=1; i<cuts1; i++){
        len = STRLEN(x_buf);
        x_buf[len]=SLC_K_TOKEN;
        len++;
        STRCPY(&x_buf[len], s1);
        if(STRLEN(x_buf)+STRLEN(s1) >= 120)
            goto out;
    }
    for(i=0; i<cuts2; i++){
        len = STRLEN(x_buf);
        x_buf[len]=SLC_K_TOKEN;
        len++;
        STRCPY(&x_buf[len], s2);
        if(STRLEN(x_buf)+STRLEN(s2) >= 120)
            goto out;
    }
out:
    x_buf[buf_len-1] = 0;
    STRCPY(buf, x_buf);    
}

static BOOL kd_std_action(void * data)
{
    int i, j, k, v, t, len;
    __proto_kd_std_t * rx;
    __proto_kd_std_order_t * ro;
    order_struct order;
    struct ___ts{
        int cuts;
        int width;
        int data[6];
    } speccut[3];
    char ___s[3][128];

    FamesAssert(data);
    if(!data)
        return fail;
    rx = (__proto_kd_std_t *)data;
    for(i=0; i<5; i++){
        MEMSET((INT08S *)&order, 0, sizeof(order));
        ro = &(rx->order[i]);
        order.WORKNO = 10*(i+1);
        MEMCPY(order.ORDERNO, ro->orderno, 7);
        switch(ro->flute[0]){
            case '1':
                order.FLUTE[0] = '3';
                break;
            case '2':
                order.FLUTE[0] = '5';
                break;
            case '3':
            default:
                order.FLUTE[0] = '7';
                break;
        }
        order.YX = CHARtoDEC(ro->press[0]);
        order.TRIM = (INT16U)(ro->speccut[0].edge[0] == 'Y');
        if (slc_is_trim_forced())
            order.TRIM = 1;
        for(k=0; k<3; k++){
            speccut[k].cuts = CHARtoDEC(ro->speccut[k].cuts[0]);
            v = STRtoINT16(ro->speccut[k].cutwidth, CHG_OPT_DEC|4);
            if(v == 0){
                speccut[k].cuts = 0;
                continue;
            }
            speccut[k].width = v;
            for(j=0; j<5; j++){
                speccut[k].data[j] = STRtoINT16(ro->speccut[k].speccut[j], CHG_OPT_DEC|4);
                v -= speccut[k].data[j];
            }
            speccut[k].data[j] = v;
        }
        v = 0;
        for(k=0; k<3; k++){
            MEMSET(___s[k], 0, sizeof(___s[k]));
            if(speccut[k].cuts > 0){
                for(j=0; j<6; j++){
                    t = speccut[k].data[j];
                    if(t>0){
                        len = STRLEN(___s[k]);
                        INT16toSTR(&___s[k][len], t, CHG_OPT_DEC|4);
                        len = STRLEN(___s[k]);
                        ___s[k][len] = SLC_L_TOKEN;
                    }
                }
                len = STRLEN(___s[k]);
                if(len > 0){
                    ___s[k][len-1] = 0; /* 去掉最后面的那个加号 */
                    v |= (1<<k);
                }
            }
        }
        #if 0
        printf("cut1=%d, s1=%s\n", speccut[0].cuts, ___s[0]);
        printf("cut2=%d, s2=%s\n", speccut[1].cuts, ___s[1]);
        printf("cut3=%d, s3=%s\n", speccut[2].cuts, ___s[2]);
        #endif
        switch(v){
            case 0:
                order.CUTS = 0;
                order.SPECCUT[0] = 0;
                break;
            case 1:
                order.CUTS = speccut[0].cuts;
                STRCPY(order.SPECCUT, ___s[0]);
                break;
            case 2:
                order.CUTS = speccut[1].cuts;
                STRCPY(order.SPECCUT, ___s[1]);
                break;
            case 3:
                if(!STRCMP(___s[0], ___s[1])){
                    order.CUTS = (speccut[0].cuts + speccut[1].cuts);
                    STRCPY(order.SPECCUT, ___s[0]);
                } else {
                    order.CUTS = 1;
                    ____combine_speccut_data(order.SPECCUT, sizeof(order.SPECCUT),
                                             ___s[0], speccut[0].cuts,
                                             ___s[1], speccut[1].cuts);
                }
                break;
            case 4:
                order.CUTS = speccut[2].cuts;
                STRCPY(order.SPECCUT, ___s[2]);
                break;
            case 5:
                if(!STRCMP(___s[0], ___s[2])){
                    order.CUTS = (speccut[0].cuts + speccut[2].cuts);
                    STRCPY(order.SPECCUT, ___s[0]);
                } else {
                    order.CUTS = 1;
                    ____combine_speccut_data(order.SPECCUT, sizeof(order.SPECCUT),
                                             ___s[0], speccut[0].cuts,
                                             ___s[2], speccut[2].cuts);
                }
                break;
            case 6:
                if(!STRCMP(___s[1], ___s[2])){
                    order.CUTS = (speccut[1].cuts + speccut[2].cuts);
                    STRCPY(order.SPECCUT, ___s[1]);
                } else {
                    order.CUTS = 1;
                    ____combine_speccut_data(order.SPECCUT, sizeof(order.SPECCUT),
                                             ___s[1], speccut[1].cuts,
                                             ___s[2], speccut[2].cuts);
                }
                break;
            case 7:
                if(!STRCMP(___s[0], ___s[1])){ /* 前两个相同 */
                    speccut[0].cuts += speccut[1].cuts;
                    if(!STRCMP(___s[0], ___s[2])){ /* 全相同 */
                        order.CUTS = (speccut[0].cuts + speccut[2].cuts);
                        STRCPY(order.SPECCUT, ___s[0]);
                    } else { /* 第三个不同 */
                        order.CUTS = 1;
                        ____combine_speccut_data(order.SPECCUT, sizeof(order.SPECCUT),
                                                 ___s[0], speccut[0].cuts,
                                                 ___s[2], speccut[2].cuts);
                    }
                } else { /* 前两个不相同 */
                    if(!STRCMP(___s[1], ___s[2])){ /* 后两个相同 */
                        speccut[1].cuts += speccut[2].cuts;
                        order.CUTS = 1;
                        ____combine_speccut_data(order.SPECCUT, sizeof(order.SPECCUT),
                                                 ___s[0], speccut[0].cuts,
                                                 ___s[1], speccut[1].cuts);
                    } else { /* 后两个也不相同 */
                        if(!STRCMP(___s[0], ___s[2])){ /* 第一个和第三个相同 */
                            speccut[0].cuts += speccut[2].cuts;
                            order.CUTS = 1;
                            ____combine_speccut_data(order.SPECCUT, sizeof(order.SPECCUT),
                                                     ___s[0], speccut[0].cuts,
                                                     ___s[1], speccut[1].cuts);
                        } else { /* 三个都不同 */
                            order.CUTS = 1;
                            ____combine_speccut_data(___s[0], sizeof(___s[0]),
                                                     ___s[0], speccut[0].cuts,
                                                     ___s[1], speccut[1].cuts);
                            ____combine_speccut_data(order.SPECCUT, sizeof(order.SPECCUT),
                                                     ___s[0], speccut[0].cuts,
                                                     ___s[2], speccut[2].cuts);
                        }
                    }
                }
                break;
            default:
                FamesAssert(!"kd_std_action() error: v has a invalid value!");
                break;
        }
        InsertOrder(&order);
    }
    return ok;
}
static BOOL kd_std_response(void * buf, int code)
{
    char * t;

    FamesAssert(buf);
    if(!buf)
        return fail;

    t = (char *)buf;
    
    if(code == RESPONSE_ACK){
        t[0] = 2;
        t[1] = 6;
        t[2] = 1;
        t[3] = 1;
        t[4] = 3;
        t[5] = 0;
        return ok;
    } else {
        t[0] = 0;
        return fail;
    }
}

static cim_link_type kadar_std_protocol = {
        kd_std_is_stx,
        kd_std_is_etx,
        kd_std_received_notifier,
        kd_std_is_received,
        kd_std_checksum_ok,
        kd_std_action,
        kd_std_response,
        512,
};

static cim_link_type fms300_protocol = {
        kd_std_is_stx,
        kd_std_is_etx,
        kd_std_received_notifier,
        kd_std_is_received,
        fms300_checksum_ok,
        kd_std_action,
        kd_std_response,
        512,
};

/*------------------------------------------------------------------------------------
 * 函数:    init_cim_link()
 *
 * 描述:    初始化生管连线功能
 *
 * 返回:    ok/fail
 *
 * 说明:    具体动作: 1) 创建生管连线后台任务, 用于刷新订单及响应生管
 *                    2) 打开生管连线功能(如果可能的话)
**----------------------------------------------------------------------------------*/
#if DEBUG == 1
const char slcdata[]="\x2O00000101108001100002000600020000000000Y1070002"
                         "000300020000000000Y0000000000000000000000000Y00000"
                         "201108901070005000200000000000000Y10500000 000 000"
                         " 00000000Y0000000000000000000000000Y00000301123194"
                         "040001000200010000000000Y0000000000000000000000000"
                         "Y0000000000000000000000000Y00000401109002050001500"
                         "200015000000000Y2020000000000000000000000Y00000000"
                         "00000000000000000Y00000501116704040002000200000000"
                         "000000Y0000000000000000000000000Y00000000000000000"
                         "00000000Y71";
void cim_link_test(void)
{
    prepare_atomic()
    in_atomic();
    STRCPY((INT08S *)cim_link_port_rx_buf, (INT08S *)slcdata);
    cim_link_protocol->received = 1;
    out_atomic();
}
#endif

BOOL init_cim_link(void)
{
    void __daemon cim_link_daemon(void * data);
    extern struct slc_config_s config;
    extern struct slc_tmp_config_s tmp_config;
    BOOL retval;

    cim_link_protocol = &kadar_std_protocol;
    cim_link_handle = InvalidHandle;
    
    cim_link_port_rx_buf = mem_alloc(1024L);
    if(!cim_link_port_rx_buf)
        return fail;
    cim_link_port_tx_buf = mem_alloc(1024L);
    if(!cim_link_port_tx_buf)
        return fail;

    MEMSET((INT08S *)cim_link_port_rx_buf, 0, 1024);
    MEMSET((INT08S *)cim_link_port_tx_buf, 0, 1024);

    #if DEBUG == 1
    RegisterSpecialKey('v', cim_link_test);
    RegisterSpecialKey('V', cim_link_test);
    #endif

    retval = fail;
    lock_kernel();
    cim_link_is_opened = 0;
    cim_link_handle = TaskCreate(cim_link_daemon, NULL, "cim-link", NULL, 2048, PRIO_SHARE, TASK_CREATE_OPT_NONE);
    if(cim_link_handle != InvalidHandle){
        TaskSuspend(cim_link_handle);
        if(tmp_config.cim_link)
            open_cim_link();
        retval = ok;
    }
    InitSerialPort(&cim_link_port);
    unlock_kernel();

    return retval;
}

/*------------------------------------------------------------------------------------
 * 函数:    open_cim_link()
 *
 * 描述:    打开生管连线功能
 *
 * 返回:    ok/fail
 *
 * 说明:    打开生管连线所用的串口, 启动后台任务
**----------------------------------------------------------------------------------*/
BOOL open_cim_link(void)
{
    __isr__ cim_link_isr(void);
    
    if(cim_link_handle == InvalidHandle)
        return fail;
    
    lock_kernel();
    if(cim_link_is_opened)
        goto out;
    switch(config.cim_protocol_type){
        case CIM_PROTOCOL_FMS300:
            cim_link_protocol = &fms300_protocol;
            break;
        case CIM_PROTOCOL_STANDARD:
            cim_link_protocol = &kadar_std_protocol;
            break;
        default:
            break;
    }
    SetSerialPort(&cim_link_port, config.cim_port.base, config.cim_port.int_no, 
                   config.cim_port.baudrate, config.cim_port.parity, 
                   config.cim_port.data, config.cim_port.stop, COM_INT_BOTH, COM_FIFO_TL_4, cim_link_isr);
    OpenSerialPort(&cim_link_port);
    PreRxSerialPort(&cim_link_port, cim_link_port_rx_buf, cim_link_protocol->max_bytes_to_receive);
    TaskResume(cim_link_handle);
    cim_link_is_opened = 1;
out:
    unlock_kernel();
    return ok;
}

/*------------------------------------------------------------------------------------
 * 函数:    close_cim_link()
 *
 * 描述:    关闭生管连线功能
 *
 * 返回:    ok/fail
 *
 * 说明:    关闭生管连线所用的串口, 挂起后台任务
**----------------------------------------------------------------------------------*/
BOOL close_cim_link(void)
{
    if(cim_link_handle == InvalidHandle)
        return fail;
    
    lock_kernel();
    if(!cim_link_is_opened)
        goto out;
    TaskSuspend(cim_link_handle);
    CloseSerialPort(&cim_link_port);
    cim_link_is_opened = 0;
out:
    unlock_kernel();
    return ok;
}

/*------------------------------------------------------------------------------------
 * 函数:    reopen_cim_link()
 *
 * 描述:    重新打开生管连线功能
 *
 * 返回:    ok/fail
**----------------------------------------------------------------------------------*/
BOOL reopen_cim_link(void)
{
    BOOL retval;
    
    retval = close_cim_link();
    retval = open_cim_link();

    return retval;
}

/*------------------------------------------------------------------------------------
 * 函数:    cim_link_isr()
 *
 * 描述:    中断服务程序
**----------------------------------------------------------------------------------*/
static __isr__ cim_link_isr(void)
{
    INT08U iir, rx_char;
    SERIAL_PORT * comx;

    enter_isr();

    comx = &cim_link_port;
        
    if(comx->status!=COM_STATUS_OPEN){
        for(;;){
            iir=inportbyte(comx->base+UART_IIR);
            if(iir&0x1)break;
            switch(iir&UART_IIR_INT){
                case UART_IIR_RX:
                case UART_IIR_RXTO:
                    while(UART_RX_READY(comx->base)){
                        inportbyte(comx->base+UART_RDR);
                    }
                    break;
                default:
                    break;
            }
        }
        isr_return_false();
    }
    
    for(;;){
        iir=inportbyte(comx->base+UART_IIR);
        #if 0
        textprintstr(1, 7, "cim_link_isr: UART_IIR_INT", 0);
        if(iir&1); else textprinthex16(1, 8, iir, 0);
        textprintdec16(6, 8, comx->rx_num, 0, 0);
        textprintdec16(12, 8, comx->rx_buf_len, 0, 0);
        #endif
        if(iir&1){                            /* No Interrupt Pending               */
            isr_return_false();
        }
        switch(iir&UART_IIR_INT){
            case UART_IIR_THR:                /* Interrupt from TX(THR empty)       */
                while(UART_TX_READY(comx->base)){
                    if(comx->tx_num<comx->tx_buf_len){
                        outportbyte(comx->base+UART_THR, comx->tx_buf[comx->tx_num]);
                        comx->tx_num++;
                        if(comx->tx_num>=comx->tx_buf_len){
                            EventSet(&comx->tx_event);
                        }
                    } else {
                        break;
                    }
                }
                break;
            case UART_IIR_RX:                 /* Interrupt from RX(RDR ready/FIFO)  */
            case UART_IIR_RXTO:
                while(UART_RX_READY(comx->base)){
                    rx_char=inportbyte(comx->base+UART_RDR);
                    if(comx->rx_num>=comx->rx_buf_len){
                        comx->rx_num=0;
                    }
                    if(cim_link_protocol->is_stx(rx_char)){
                        comx->rx_num=0;
                    }
                    comx->rx_buf[comx->rx_num]=rx_char;
                    if(cim_link_protocol->is_etx(rx_char)){
                        comx->rx_num++;
                        comx->rx_buf[comx->rx_num]=0;
                        comx->rx_packet++;
                        EventSet(&comx->rx_event);
                        cim_link_protocol->received = 1;
                        cim_link_protocol->received_notifier();
                    }
                    comx->rx_num++;
                }
                break;
            default:                          /* Other interrupt                    */
                break;
        }
    }
}

/*------------------------------------------------------------------------------------
 * 函数:    cim_link_daemon()
 *
 * 描述:    生管连线后台服务
**----------------------------------------------------------------------------------*/
static void __daemon cim_link_daemon(void * data)
{
    __proto_kd_std_t rx;
    order_struct order;
    int delayed;
    extern int ord_chg_lock;
    prepare_atomic()

    data = data;

    for(;;){
        if(cim_link_protocol->is_received()){
            in_atomic();
            cim_link_protocol->received = 0;
            cim_link_protocol->rx_packets_nr++;
            MEMCPY((INT08S *)&rx, (INT08S *)cim_link_port_rx_buf, sizeof(rx));
            out_atomic();
            if(cim_link_protocol->checksum_ok((void *)&rx)){
                cim_link_protocol->response(cim_link_port_tx_buf, RESPONSE_ACK);
                SendSerialPort(&cim_link_port, cim_link_port_tx_buf, STRLEN((INT08S *)cim_link_port_tx_buf));
                lock_kernel();
                cim_link_protocol->tx_packets_nr++;
                delayed = config.cim_data_delayed;
                unlock_kernel();
                if(delayed > 0){ /* 延迟时间不为0 */
                    TaskDelay((INT16U)delayed);
                }
                os_mutex_lock(ord_chg_lock);
                lock_kernel();
                ZapOrder();
                cim_link_protocol->action((void *)&rx);
                unlock_kernel();
                /*
                ** 添加订单后的动作......
                */
                if(config.slc_used == 3){
                    int  i, sendport;
                    if(get_order_nr()>=0) {
                        for(i = 0; i < 5; i++){
                            if(!GetOrderForOrdChg(&order, i))
                                break;
                            if(i == 0){
                                if(similar_order(&order, &(config.slc[0].working)))
                                    sendport = 2;
                                else
                                    sendport = 1;
                            }
                            if(i >= 1){
                                if(similar_order(&order, &(config.slc[sendport-1].working)))
                                    break; /* 本来就是一样的, 不需要发送 */
                                if(slc_send_order(sendport, &order, 0)){
                                    slc_kl_up_set(sendport);
                                    slc_send_start(sendport);
                                }
                                break;
                            }
                        }
                    }
                }
                os_mutex_unlock(ord_chg_lock);
            } /* checksum_ok() */
            PreRxSerialPort(&cim_link_port, cim_link_port_rx_buf, cim_link_protocol->max_bytes_to_receive);
        }     /* is_received() */
        TaskDelay(20L);
    }
}

/*------------------------------------------------------------------------------------
 *
 *    下面为生管连线串口的监控部分
 *
**----------------------------------------------------------------------------------*/
static gui_widget * main_form = NULL;
static gui_widget * rx_text[5], * rx_area, * rx_caption;
static gui_widget * tx_text[1], * tx_area, * tx_caption;

#define  bytes_per_row  106

gui_widget * init_cim_link_monitor_screen(void)
{
    int x, y, w, h, i;

    #define ___X          32
    #define ___BK_COLOR   236
    
    /* 主界面   */
    main_form = gui_create_widget(GUI_WIDGET_FORM, 10, 422, 998, 226, 0, ___BK_COLOR, 0, FORM_STYLE_THIN_BDR);
    if(!main_form)
        goto some_error;
    gui_hide_widget(main_form);
    gui_form_init_private(main_form, 8);

    /* 接收部分 */
    rx_caption = gui_create_widget(GUI_WIDGET_LABEL, ___X, 9, 832, 16, 0, ___BK_COLOR, 1, LABEL_STYLE_TRANSPARENT);
    if(!rx_caption)
        goto some_error;
    gui_widget_link(main_form, rx_caption);
    gui_label_init_private(rx_caption, 64);
    
    x = ___X;
    y = 30;
    w = 832;
    h = 112;
    rx_area = gui_create_widget(GUI_WIDGET_EDIT, x, y, w, h, 0, WIDGET_BKCOLOR, 1, 0);
    if(!rx_area)
        goto some_error;
    gui_widget_link(main_form, rx_area);
    gui_edit_init_private(rx_area, 8);

    x  = 5;
    y  = 7;
    w -= 8;
    h  = 18;

    for(i=0; i<5; i++){
        rx_text[i] = gui_create_widget(GUI_WIDGET_EDIT, x, y, w, h, 0, WIDGET_BKCOLOR, 1, EDIT_STYLE_TRANSPARENT);
        if(!rx_text[i])
            goto some_error;
        gui_widget_link(rx_area, rx_text[i]);
        gui_edit_init_private(rx_text[i], bytes_per_row);
        y += (h+2);
    }

    /* 发送部分 */
    tx_caption = gui_create_widget(GUI_WIDGET_LABEL, ___X, 150, 832, 16, 0, ___BK_COLOR, 1, LABEL_STYLE_TRANSPARENT);
    if(!tx_caption)
        goto some_error;
    gui_widget_link(main_form, tx_caption);
    gui_label_init_private(tx_caption, 64);

    x = ___X;
    y = 171;
    w = 832;
    h = 31;
    tx_area = gui_create_widget(GUI_WIDGET_EDIT, x, y, w, h, 0, WIDGET_BKCOLOR, 1, 0);
    if(!tx_area)
        goto some_error;
    gui_widget_link(main_form, tx_area);
    gui_edit_init_private(tx_area, 8);

    x  = 5;
    y  = 7;
    w -= 8;
    h  = 18;
    tx_text[0] = gui_create_widget(GUI_WIDGET_EDIT, x, y, w, h, 0, WIDGET_BKCOLOR, 1, EDIT_STYLE_TRANSPARENT);
    if(!tx_text[0])
        goto some_error;
    gui_widget_link(tx_area, tx_text[0]);
    gui_edit_init_private(tx_text[0], bytes_per_row);
    y += (h+2);

    return main_form;

some_error:
    sys_print("init_cim_link_monitor_screen(): failed to create widgets!\n");
    ExitApplication();
    return NULL;
}

void cim_link_monitor(void)
{
    int  i, j, k;
    char ___s[128];
    INT08U x[1024], buf[bytes_per_row], ch;
    
    FamesAssert(main_form);

    if(!main_form)
        return;

    gui_show_widget(main_form);

    for(;;){
        /* 接收部分 */
        lock_kernel();
        MEMCPY((INT08S *)x, (INT08S *)cim_link_port_rx_buf, 1020);
        unlock_kernel();
        for(i = 0, k = 0; k < 5; k++){
            for(j = 0; j < bytes_per_row-6; j++, i++){
                ch = x[i];
                if(ch == 0)
                    break;
                if(ch > 0 && ch < 0x10){
                    buf[j++] = '[';
                    buf[j++] = HEXCHAR[ch];
                    buf[j]   = ']';
                } else if(ch >= 0x10 && ch < 0x20){
                    buf[j++] = '[';
                    buf[j++] = '1';
                    buf[j++] = HEXCHAR[ch-0x10];
                    buf[j]   = ']';
                } else {
                    buf[j]  = ch;
                }            
            }
            buf[j] = 0;
            gui_edit_set_text(rx_text[k], (INT08S *)buf);
        }
        lock_kernel();
        sprintf(___s, pick_string("接收数据区: 本次接收: %3d 字节, 已接收的数据包个数: %u", 
                                  "Rx Area: Rxed: %3d Bytes, Packets: %u"), 
                      i, cim_link_protocol->rx_packets_nr);
        unlock_kernel();
        gui_label_set_text(rx_caption, ___s);
        /* 发送部分 */
        lock_kernel();
        MEMCPY((INT08S *)x, (INT08S *)cim_link_port_tx_buf, 1020);
        unlock_kernel();
        for(i = 0, k = 0; k < 1; k++){
            for(j = 0; j < bytes_per_row-6; j++, i++){
                ch = x[i];
                if(ch == 0)
                    break;
                if(ch > 0 && ch < 0x10){
                    buf[j++] = '[';
                    buf[j++] = HEXCHAR[ch];
                    buf[j]   = ']';
                } else if(ch >= 0x10 && ch < 0x20){
                    buf[j++] = '[';
                    buf[j++] = '1';
                    buf[j++] = HEXCHAR[ch-0x10];
                    buf[j]   = ']';
                } else {
                    buf[j]  = ch;
                }            
            }
            buf[j] = 0;
            gui_edit_set_text(tx_text[k], (INT08S *)buf);
        }
        lock_kernel();
        sprintf(___s, pick_string("发送数据区: 本次发送: %3d 字节, 已发送的数据包个数: %u", 
                                  "Tx Area: Txed: %3d Bytes, Packets: %u"),
                      i, cim_link_protocol->tx_packets_nr);
        unlock_kernel();
        gui_label_set_text(tx_caption, ___s);

        if(ESC == waitkey(20L))
            break;
    }

    gui_hide_widget(main_form);
}


/*====================================================================================
 * 
 * 本文件结束: slc/cim_link.c
 * 
**==================================================================================*/

