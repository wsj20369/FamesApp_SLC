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

/*
 * 控制相关的设定
*/

#define CONFIG_SEND_ORDER_ON_STARTUP    1       /* 启动时发送定单 */
#define CONFIG_NO_START_PLC_ON_STARTUP  1       /* 启动时发送定单, 但不发上下刀线及启动信号 */

#define CONFIG_SLC_KNIFE_NUMBER         0       /* 刀数的设定, 0=可任意设定 */
#define CONFIG_SLC_WHEEL_NUMBER         0       /* 线数的设定, 0=可任意设定 */

#define CONFIG_PLC_CONNECT_TIMEOUT      50      /* 单位为0.1秒, PLC断线的超时值 */

#define CONFIG_NOACTION_ON_QUIT         1       /* 退出时, 不做任何动作 */


/*
 * 画面相关的设定
*/

#define CONFIG_INPUT_ORDER_FONT24       1       /* 输单画面中用24号字体 */
#define CONFIG_ORDER_AREA_BITFONT       1       /* 定单区域使用大字体 */
#define CONFIG_PREVIEW_WIDER            1       /* 预览窗口更宽一些 */
#define CONFIG_FUNC_BUTTON_BIGFONT      1       /* 功能键使用大字体, 只影响主功能与CTRL功能 */




/*
 * 计算相关
*/



/*
 * 字体, 此定义要与函数load_fonts()一致
*/
#define CONFIG_FONT_SYSTEM              0
#define CONFIG_FONT_16                  1
#define CONFIG_FONT_48                  2
#define CONFIG_FONT_24                  3
#define CONFIG_FONT_20                  4
#define CONFIG_FONT_MNTR                5



#endif /* #ifndef FAMES_SLC_CFG_DEFS_H */

/*====================================================================================
 * 
 * 本文件结束: slc/cfg_defs.h
 * 
**==================================================================================*/

