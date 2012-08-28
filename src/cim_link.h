/*************************************************************************************
 * �ļ�:    slc/cim_link.h
 *
 * ����:    ��������
 *
 * ����:    Jun
 *
 * ʱ��:    2011-4-5
*************************************************************************************/
#ifndef SLC_CIM_LINK_H
#define SLC_CIM_LINK_H


/*------------------------------------------------------------------------------------
 * 
 *      ֧�ֵ�Э������
 * 
**----------------------------------------------------------------------------------*/
#define CIM_PROTOCOL_STANDARD 0  /* �����׼Э��(460B)         */
#define CIM_PROTOCOL_FMS300   1  /* ����FMS300���õ�Э��(460B) */


/*------------------------------------------------------------------------------------
 * 
 *      ��������Э�鶨��
 * 
**----------------------------------------------------------------------------------*/
struct cim_link_struct {
    BOOL (*is_stx)(INT08S ch);                 /* �Ƿ���STX */
    BOOL (*is_etx)(INT08S ch);                 /* �Ƿ���ETX */
    BOOL (*received_notifier)(void);           /* �յ��������ݰ����֪ͨ���� */
    BOOL (*is_received)(void);                 /* �յ����ݰ����� */
    BOOL (*checksum_ok)(void * data);          /* У������ȷ���� */
    BOOL (*action)(void * data);               /* ���ݰ��յ���Ķ��� */
    BOOL (*response)(void * buf, int code);    /* �����ܵĻ�Ӧ */
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
 *      ��������
 * 
**----------------------------------------------------------------------------------*/
BOOL   init_cim_link(void);         /* ��ʼ���������߹��� */
BOOL   open_cim_link(void);         /* ���������߹���   */
BOOL   close_cim_link(void);        /* �ر��������߹���   */
BOOL   reopen_cim_link(void);       /* ���´�           */

void         cim_link_monitor(void);
gui_widget * init_cim_link_monitor_screen(void);


#endif /* #ifndef SLC_CIM_LINK_H */

/*====================================================================================
 * 
 * ���ļ�����: slc/cim_link.h
 * 
**==================================================================================*/

