/*************************************************************************************
 * 文件:    slc/cfg_defs.h
 *
 * 作者:    Jun
 *
 * 时间:    2012-10-16
 *
 * 描述:    SLC系统配置文件
*************************************************************************************/
#ifndef FAMES_SLC_CFG_DEFS_H
#define FAMES_SLC_CFG_DEFS_H

#include "plc_defs\plc_list.h"
#define CONFIG_SLC_PLC_TYPE             _PLC_TYPE_JY524CE2

#define CONFIG_SEND_ORDER_ON_STARTUP    0       /* 启动时发送定单 */

#define CONFIG_SLC_KNIFE_NUMBER         0       /* 刀数的设定, 0=可任意设定 */
#define CONFIG_SLC_WHEEL_NUMBER         0       /* 线数的设定, 0=可任意设定 */

#define CONFIG_PLC_CONNECT_TIMEOUT      50      /* 单位为0.1秒, PLC断线的超时值 */


#endif /* #ifndef FAMES_SLC_CFG_DEFS_H */

/*====================================================================================
 * 
 * 本文件结束: slc/cfg_defs.h
 * 
**==================================================================================*/

