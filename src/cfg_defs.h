/*************************************************************************************
 * �ļ�:    slc/cfg_defs.h
 *
 * ����:    Jun
 *
 * ʱ��:    2012-10-16
 *
 * ����:    SLCϵͳ�����ļ�
*************************************************************************************/
#ifndef FAMES_SLC_CFG_DEFS_H
#define FAMES_SLC_CFG_DEFS_H

#include "plc_defs\plc_list.h"
#define CONFIG_SLC_PLC_TYPE             _PLC_TYPE_JY524CE2

/*
 * ������ص��趨
*/

#define CONFIG_SLC_KNIFE_NUMBER         0       /* �������趨, 0=�������趨 */
#define CONFIG_SLC_WHEEL_NUMBER         0       /* �������趨, 0=�������趨 */

#define CONFIG_PLC_CONNECT_TIMEOUT      50      /* ��λΪ0.1��, PLC���ߵĳ�ʱֵ */

#define CONFIG_NOACTION_ON_QUIT         1       /* �˳�ʱ, �����κζ��� */


enum {                                          /* ����ʱ���͵�ģʽ */
    _SendOrderMode_None = 0,
    _SendOrderMode_SendOnly = 1,
    _SendOrderMode_SendLastRunning = 2,
    _SendOrderMode_SendDefault = 3,
    _SendOrderMode_SendOnly_Control = 11,
    _SendOrderMode_SendLastRunning_Control = 12,
    _SendOrderMode_SendDefault_Control = 13
};

#define CONFIG_DEFAULT_SEND_ONSTARTUP   _SendOrderMode_SendOnly



/*
 * ������ص��趨
*/

#define CONFIG_INPUT_ORDER_FONT24       1       /* �䵥��������24������ */
#define CONFIG_ORDER_AREA_BITFONT       1       /* ��������ʹ�ô����� */
#define CONFIG_PREVIEW_WIDER            1       /* Ԥ�����ڸ���һЩ */
#define CONFIG_FUNC_BUTTON_BIGFONT      1       /* ���ܼ�ʹ�ô�����, ֻӰ����������CTRL���� */


/*
 * ע�����
*/

#define CONFIG_REG_PROMPT_TIME1         10UL    /* 10����֮��, ��һ������(��һ��) */
#define CONFIG_REG_PROMPT_TIME2         21606UL /* ������6Сʱ֮�� [+6��] */
#define CONFIG_REG_PROMPT_TIMEx         600UL   /* �´����Ѽ��ʱ��: 10���� */


/*
 * �������
*/



/*
 * ����, �˶���Ҫ�뺯��load_fonts()һ��
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
 * ���ļ�����: slc/cfg_defs.h
 * 
**==================================================================================*/

