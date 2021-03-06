/*************************************************************************************
 * 文件:    slc/config.h
 *
 * 描述:    slc配置文件
 *
 * 作者:    Jun
 *
 * 时间:    2011-2-20
*************************************************************************************/
#ifndef SLC_CONFIG_H
#define SLC_CONFIG_H

/*------------------------------------------------------------------------------------
 * 
 *          分压机配置数据结构
 * 
**----------------------------------------------------------------------------------*/
struct slc_config_s {                
    INT32U magic;               /* 文件有效标识 */
    int    size;                /* 文件大小     */
    struct __port {             /* 串口设置     */                   
        INT16S base;
        INT32S baudrate;
        INT16S data;
        INT16S stop;
        INT16S parity;
        INT16S int_no;
    } slc_1_port,
      slc_2_port,
      cim_port;
    slc_descriptor_t slc[2];    /* 分压机       */
    int slc_used;               /* 0=测试模式, 1=用机1, 2=用机2, 3=用机1&2 */
    int cim_data_delayed;       /* 生管数据包延迟时间, 单位毫秒, 默认为0 */
    int cim_protocol_type;      /* 生管连线协议类型 */
    int cim_link_point;         /* 生管换单信号连接点, 0=机1, 其它=机2 */
    int slc_start_mode;         /* 分压机启动模式 */
    int send_order_on_startup;  /* 启动时的送单模式 */
    int slc_reverse_mode;       /* 刀线反排模式, 1=打开, 0=关闭 */
    int language;               /* 语言, 0=中文, 1=英语 */
};


/*------------------------------------------------------------------------------------
 * 
 *      函数声明
 * 
**----------------------------------------------------------------------------------*/
BOOL  read_config(void);
BOOL  save_config(void);
BOOL  check_config(void);
BOOL  active_config(void);
BOOL  copy_to_config(struct slc_config_s * cfg);

int   slc_is_trim_forced(void);



#endif /* #ifndef SLC_CONFIG_H */

/*====================================================================================
 * 
 * 本文件结束: slc/config.h
 * 
**==================================================================================*/

