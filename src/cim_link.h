/*************************************************************************************
 * 文件:    slc/cim_link.h
 *
 * 描述:    生管连线
 *
 * 作者:    Jun
 *
 * 时间:    2011-4-5
*************************************************************************************/
#ifndef SLC_CIM_LINK_H
#define SLC_CIM_LINK_H


/*------------------------------------------------------------------------------------
 * 
 *      支持的协议种类
 * 
**----------------------------------------------------------------------------------*/
#define CIM_PROTOCOL_STANDARD 0  /* 卡达标准协议(460B)         */
#define CIM_PROTOCOL_FMS300   1  /* 卡达FMS300所用的协议(460B) */


/*------------------------------------------------------------------------------------
 * 
 *      生管连线协议定义
 * 
**----------------------------------------------------------------------------------*/
struct cim_link_struct {
    BOOL (*is_stx)(INT08S ch);                 /* 是否是STX */
    BOOL (*is_etx)(INT08S ch);                 /* 是否是ETX */
    BOOL (*received_notifier)(void);           /* 收到完整数据包后的通知函数 */
    BOOL (*is_received)(void);                 /* 收到数据包了吗 */
    BOOL (*checksum_ok)(void * data);          /* 校验是正确的吗 */
    BOOL (*action)(void * data);               /* 数据包收到后的动作 */
    BOOL (*response)(void * buf, int code);    /* 给生管的回应 */
    int    max_bytes_to_receive;
    BOOL   received;
    int    rx_packets_nr;
    int    tx_packets_nr;
    void * data1;
    void * data2;
};
typedef struct cim_link_struct cim_link_type;

#define RESPONSE_ACK  0
#define RESPONSE_NAK  1

/*------------------------------------------------------------------------------------
 * 
 *      函数声明
 * 
**----------------------------------------------------------------------------------*/
BOOL   init_cim_link(void);         /* 初始化生管连线功能 */
BOOL   open_cim_link(void);         /* 打开生管连线功能   */
BOOL   close_cim_link(void);        /* 关闭生管连线功能   */
BOOL   reopen_cim_link(void);       /* 重新打开           */

void         cim_link_monitor(void);
gui_widget * init_cim_link_monitor_screen(void);


#endif /* #ifndef SLC_CIM_LINK_H */

/*====================================================================================
 * 
 * 本文件结束: slc/cim_link.h
 * 
**==================================================================================*/

