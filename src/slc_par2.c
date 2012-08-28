/******************************************************************************************
 * �ļ�:    slc/slc_par2.c
 *
 * ����:    ��ѹ����������(2), ȫ�ֲ���
 *
 * ����:    Jun
******************************************************************************************/
#define  SLC_GLOBAL_PARAMETER_C
#include <FamesOS.h>
#include "common.h"


gui_widget * slc_global_param_dlg = NULL; /* ��ѹ��ϵͳ�����Ի���  */

extern BMPINFO  icon;                     /* ͼ��  */

/*-----------------------------------------------------------------------------------------
 *          
 *      ��ѹ��ϵͳ��������ض���
 * 
**---------------------------------------------------------------------------------------*/
enum slc_global_param_id {
    __slc_1,
    __slc_1_type,
    __slc_1_k_nr,
    __slc_1_l_nr,
    __slc_1_used,
    
    __slc_2,
    __slc_2_type,
    __slc_2_k_nr,
    __slc_2_l_nr,
    __slc_2_used,

    __cim,
    __cim_delayed,
    __cim_proto_type,
    __cim_link_point,

    __1_port,
    __1_port_base,
    __1_port_rate,
    __1_port_data,
    __1_port_stop,
    __1_port_parity,
    __1_port_irq,

    __2_port,
    __2_port_base,
    __2_port_rate,
    __2_port_data,
    __2_port_stop,
    __2_port_parity,
    __2_port_irq,

    __cim_port,
    __cim_port_base,
    __cim_port_rate,
    __cim_port_data,
    __cim_port_stop,
    __cim_port_parity,
    __cim_port_irq,

    __slc_ctrl,
    __slc_start_mode,

    __misc,
    __slc_language,
};

static char __note_slc_type_zh[]    = "��ѹ������: 0=��ͨ��, 1=��ѹ��.";
static char __note_slc_k_nr_zh[]    = "����: 3 ~ 9";
static char __note_slc_l_nr_zh[]    = "����: 4 ~ 16";
static char __note_slc_used_zh[]    = "1=ʹ��, 0=����";

static char __note_slc_type_en[]    = "SLC Type: 0=Single, 1=Dual.";
static char __note_slc_k_nr_en[]    = "K Number: 3 ~ 9";
static char __note_slc_l_nr_en[]    = "L Number: 4 ~ 16";
static char __note_slc_used_en[]    = "1=Use, 0=No";

static char __note_port_base_zh[]   = "I/O��ַ, ����3F8, 2F8��";
static char __note_port_rate_zh[]   = "������, 9600 ~ 115200";
static char __note_port_data_zh[]   = "����λ, 5 ~ 8";
static char __note_port_stop_zh[]   = "ֹͣλ, 1 ~ 2";
static char __note_port_irq_zh[]    = "�жϺ�(IRQ), 3 ~ 15";
static char __note_port_parity_zh[] = "У��λ, N=��У��, E=żУ��, O=��У��";

static char __note_port_base_en[]   = "I/O Port Address, For example: 3F8, 2F8";
static char __note_port_rate_en[]   = "Baudrate: 9600 ~ 115200";
static char __note_port_data_en[]   = "Data Bits: 5 ~ 8";
static char __note_port_stop_en[]   = "Stop Bits: 1 ~ 2";
static char __note_port_irq_en[]    = "IRQ: 3 ~ 15";
static char __note_port_parity_en[] = "Parity: N=None, E=Even, O=Odd";

static char * __err_port_base_zh    = "I/O��ַ����̫��, ����������һ�ΰ�";
static char * __err_port_rate_zh    = "�����ʲ�Ӧ�������ֵ, ����ȷ�ķ�ΧӦ����: 2400 ~ 115200";
static char * __err_port_data_zh    = "����λ��Ч, ����ȷ�ķ�ΧӦ����: 5 ~ 8";
static char * __err_port_stop_zh    = "ֹͣλ��Ч, ����ȷ�ķ�ΧӦ����: 1 ~ 2";
static char * __err_port_irq_zh     = "�жϺ�(IRQ)��Ч, ����ȷ�ķ�ΧӦ����: 3 ~ 15";
static char * __err_port_parity_zh  = "У��λӦ��������֮һ: N, E, O";

static char * __err_port_base_en    = "I/O Port Address Invalid, Try again!";
static char * __err_port_rate_en    = "Baudrate Invalid! [2400 ~ 115200]";
static char * __err_port_data_en    = "Data Bits Invalid! [5 ~ 8]";
static char * __err_port_stop_en    = "Stop Bits Invalid! [1 ~ 2]";
static char * __err_port_irq_en     = "IRQ Invalid! [3 ~ 15]";
static char * __err_port_parity_en  = "Parity Invalid! [N, E, O]";

#define  __err_port_base    (pick_string(__err_port_base_zh,   __err_port_base_en))
#define  __err_port_rate    (pick_string(__err_port_rate_zh,   __err_port_rate_en))
#define  __err_port_data    (pick_string(__err_port_data_zh,   __err_port_data_en))
#define  __err_port_stop    (pick_string(__err_port_stop_zh,   __err_port_stop_en))
#define  __err_port_irq     (pick_string(__err_port_irq_zh,    __err_port_irq_en))
#define  __err_port_parity  (pick_string(__err_port_parity_zh, __err_port_parity_en))

#define ___STYLE  0
#define ___FONT   1  /* font16 */
/* ��-1�Ļ���ѡ�� */
#undef  ___X
#undef  ___Y
#define ___X  28
#define ___Y  45
DECLARE_INPUT_DIALOG_START(slc_global_param_dialog)
INPUT_DIALOG_GROUP(" ��-1 ",     __slc_1, ___X, ___Y, 216, 159, ___FONT, GROUPBOX_STYLE_CAPTION)
INPUT_DIALOG_ITEM("��  ��: ",    __slc_1_type,    1,  ___X+10,  ___Y+24,  180, 28, ___FONT, ___STYLE,  __note_slc_type_zh)
INPUT_DIALOG_ITEM("��  ��: ",    __slc_1_k_nr,    2,  ___X+10,  ___Y+56,  180, 28, ___FONT, ___STYLE,  __note_slc_k_nr_zh)
INPUT_DIALOG_ITEM("��  ��: ",    __slc_1_l_nr,    2,  ___X+10,  ___Y+88,  180, 28, ___FONT, ___STYLE,  __note_slc_l_nr_zh)
INPUT_DIALOG_ITEM("ʹ  ��: ",    __slc_1_used,    1,  ___X+10,  ___Y+120, 180, 28, ___FONT, ___STYLE,  __note_slc_used_zh)
/* ��-2�Ļ���ѡ�� */
#undef  ___X
#undef  ___Y
#define ___X  260
#define ___Y  45
INPUT_DIALOG_GROUP(" ��-2 ",     __slc_2, ___X, ___Y, 216, 159, ___FONT, GROUPBOX_STYLE_CAPTION)
INPUT_DIALOG_ITEM("��  ��: ",    __slc_2_type,    1,  ___X+10,  ___Y+24,  180, 28, ___FONT, ___STYLE,  __note_slc_type_zh)
INPUT_DIALOG_ITEM("��  ��: ",    __slc_2_k_nr,    2,  ___X+10,  ___Y+56,  180, 28, ___FONT, ___STYLE,  __note_slc_k_nr_zh)
INPUT_DIALOG_ITEM("��  ��: ",    __slc_2_l_nr,    2,  ___X+10,  ___Y+88,  180, 28, ___FONT, ___STYLE,  __note_slc_l_nr_zh)
INPUT_DIALOG_ITEM("ʹ  ��: ",    __slc_2_used,    1,  ___X+10,  ___Y+120, 180, 28, ___FONT, ___STYLE,  __note_slc_used_zh)
/* �������ߵĻ������� */
#undef  ___X
#undef  ___Y
#define ___X  492
#define ___Y  45
INPUT_DIALOG_GROUP(" �������� ", __cim, ___X, ___Y, 216, 159, ___FONT, GROUPBOX_STYLE_CAPTION)
INPUT_DIALOG_ITEM("�ӳ�ʱ��: ",  __cim_delayed,     4,  ___X+10,  ___Y+24,  180, 28, ___FONT, ___STYLE,  "���յ����ݰ���ת��Ϊ�����ļ��ʱ��, ��λ����")
INPUT_DIALOG_ITEM("Э������: ",  __cim_proto_type,  2,  ___X+10,  ___Y+56,  180, 28, ___FONT, ___STYLE,  "����Э�������, 0=�����׼Э��")
INPUT_DIALOG_ITEM("�źŽӵ�: ",  __cim_link_point,  1,  ___X+10,  ___Y+88,  180, 28, ___FONT, ___STYLE,  "���ܻ����źŵ����ӵ�, 1=��1, 2=��2")
/* ��-1�Ĵ������� */
#undef  ___X
#undef  ___Y
#define ___X  28
#define ___Y  213
INPUT_DIALOG_GROUP(" ��-1���� ", __1_port, ___X, ___Y, 216, 223, ___FONT, GROUPBOX_STYLE_CAPTION)
INPUT_DIALOG_ITEM("����ַ: ",    __1_port_base,     4,  ___X+10,  ___Y+24,  180, 28, ___FONT, ___STYLE, __note_port_base_zh)
INPUT_DIALOG_ITEM("������: ",    __1_port_rate,     6,  ___X+10,  ___Y+56,  180, 28, ___FONT, ___STYLE, __note_port_rate_zh)
INPUT_DIALOG_ITEM("����λ: ",    __1_port_data,     1,  ___X+10,  ___Y+88,  180, 28, ___FONT, ___STYLE, __note_port_data_zh)
INPUT_DIALOG_ITEM("ֹͣλ: ",    __1_port_stop,     1,  ___X+10,  ___Y+120, 180, 28, ___FONT, ___STYLE, __note_port_stop_zh)
INPUT_DIALOG_ITEM("У��λ: ",    __1_port_parity,   1,  ___X+10,  ___Y+152, 180, 28, ___FONT, ___STYLE, __note_port_parity_zh)
INPUT_DIALOG_ITEM("�жϺ�: ",    __1_port_irq,      2,  ___X+10,  ___Y+184, 180, 28, ___FONT, ___STYLE, __note_port_irq_zh)
/* ��-2�Ĵ������� */
#undef  ___X
#undef  ___Y
#define ___X  260
#define ___Y  213
INPUT_DIALOG_GROUP(" ��-2���� ", __2_port, ___X, ___Y, 216, 223, ___FONT, GROUPBOX_STYLE_CAPTION)
INPUT_DIALOG_ITEM("����ַ: ",    __2_port_base,     4,  ___X+10,  ___Y+24,  180, 28, ___FONT, ___STYLE, __note_port_base_zh)
INPUT_DIALOG_ITEM("������: ",    __2_port_rate,     6,  ___X+10,  ___Y+56,  180, 28, ___FONT, ___STYLE, __note_port_rate_zh)
INPUT_DIALOG_ITEM("����λ: ",    __2_port_data,     1,  ___X+10,  ___Y+88,  180, 28, ___FONT, ___STYLE, __note_port_data_zh)
INPUT_DIALOG_ITEM("ֹͣλ: ",    __2_port_stop,     1,  ___X+10,  ___Y+120, 180, 28, ___FONT, ___STYLE, __note_port_stop_zh)
INPUT_DIALOG_ITEM("У��λ: ",    __2_port_parity,   1,  ___X+10,  ___Y+152, 180, 28, ___FONT, ___STYLE, __note_port_parity_zh)
INPUT_DIALOG_ITEM("�жϺ�: ",    __2_port_irq,      2,  ___X+10,  ___Y+184, 180, 28, ___FONT, ___STYLE, __note_port_irq_zh)
/* �������ߵĴ������� */
#undef  ___X
#undef  ___Y
#define ___X  492
#define ___Y  213
INPUT_DIALOG_GROUP(" �������ߴ��� ", __cim_port, ___X, ___Y, 216, 223, ___FONT, GROUPBOX_STYLE_CAPTION)
INPUT_DIALOG_ITEM("����ַ: ",    __cim_port_base,   4,  ___X+10,  ___Y+24,  180, 28, ___FONT, ___STYLE, __note_port_base_zh)
INPUT_DIALOG_ITEM("������: ",    __cim_port_rate,   6,  ___X+10,  ___Y+56,  180, 28, ___FONT, ___STYLE, __note_port_rate_zh)
INPUT_DIALOG_ITEM("����λ: ",    __cim_port_data,   1,  ___X+10,  ___Y+88,  180, 28, ___FONT, ___STYLE, __note_port_data_zh)
INPUT_DIALOG_ITEM("ֹͣλ: ",    __cim_port_stop,   1,  ___X+10,  ___Y+120, 180, 28, ___FONT, ___STYLE, __note_port_stop_zh)
INPUT_DIALOG_ITEM("У��λ: ",    __cim_port_parity, 1,  ___X+10,  ___Y+152, 180, 28, ___FONT, ___STYLE, __note_port_parity_zh)
INPUT_DIALOG_ITEM("�жϺ�: ",    __cim_port_irq,    2,  ___X+10,  ___Y+184, 180, 28, ___FONT, ___STYLE, __note_port_irq_zh)
/* ��ѹ������ѡ�� */
#undef  ___X
#undef  ___Y
#define ___X  724
#define ___Y  45
INPUT_DIALOG_GROUP(" ��ѹ������ѡ�� ", __slc_ctrl, ___X, ___Y, 216, 159, ___FONT, GROUPBOX_STYLE_CAPTION)
INPUT_DIALOG_ITEM("����ģʽ:",  __slc_start_mode,   3,  ___X+10,  ___Y+56,  180, 28, ___FONT, ___STYLE,  "��ѹ��������ģʽ")
/* ����ѡ�� */
#undef  ___X
#undef  ___Y
#define ___X  724
#define ___Y  213
INPUT_DIALOG_GROUP(" ����ѡ�� ", __misc, ___X, ___Y, 216, 223, ___FONT, GROUPBOX_STYLE_CAPTION)
INPUT_DIALOG_ITEM("����ѡ��:",   __slc_language,    1,  ___X+10,  ___Y+56,  180, 28, ___FONT, ___STYLE,  "����: 0=��������(GB2312), 1=ENGLISH")
/* ��ɫ����
INPUT_DIALOG_SET(slc_global_param_dialog, "��ѹ��ϵͳ����", &icon, 22, 96, 972, 540, 189, 66, 1, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE)
*/
INPUT_DIALOG_SET(slc_global_param_dialog, "��ѹ��ϵͳ����", &icon, 22, 96, 972, 540, 70, WIDGET_BKCOLOR, 1, FORM_STYLE_XP_BORDER|FORM_STYLE_TITLE)
DECLARE_INPUT_DIALOG_ENDED(slc_global_param_dialog);

gui_widget * init_slc_global_param_dlg(void)
{
    char * s;

    input_dialog_preset_title(&slc_global_param_dialog, pick_string(NULL, "SLC Global Parameters"));

    #define ____do_set(id, name_en, comment_en) \
                  do { \
                    s = pick_string(NULL, name_en); \
                    if(s) \
                        input_dialog_preset_item_name(&slc_global_param_dialog, id, s); \
                    s = pick_string(NULL, comment_en); \
                    if(s) \
                        input_dialog_preset_item_comment(&slc_global_param_dialog, id, s); \
                  } while(0)

    ____do_set(__slc_1,             " M-1 ",          NULL);
    ____do_set(__slc_1_type,        "SLC-Type: ",     __note_slc_type_en);
    ____do_set(__slc_1_k_nr,        "K Number: ",     __note_slc_k_nr_en);
    ____do_set(__slc_1_l_nr,        "L Number: ",     __note_slc_l_nr_en);
    ____do_set(__slc_1_used,        "Use This: ",     __note_slc_used_en);

    ____do_set(__slc_2,             " M-2 ",          NULL);
    ____do_set(__slc_2_type,        "SLC-Type: ",     __note_slc_type_en);
    ____do_set(__slc_2_k_nr,        "K Number: ",     __note_slc_k_nr_en);
    ____do_set(__slc_2_l_nr,        "L Number: ",     __note_slc_l_nr_en);
    ____do_set(__slc_2_used,        "Use This: ",     __note_slc_used_en);

    ____do_set(__cim,               " CIM-LINK ",     NULL);
    ____do_set(__cim_delayed,       "Delay Time: ",   "The Delay Time(ms)");
    ____do_set(__cim_proto_type,    "Proto Type: ",   "Protocol Type, 0=KaDar Standard Prototol");
    ____do_set(__cim_link_point,    "Link Point: ",   "Link Point, 1=M1, 2=M2");

    ____do_set(__1_port,            " M-1 Port ",     NULL);
    ____do_set(__1_port_base,       "PortAddr: ",     __note_port_base_en);
    ____do_set(__1_port_rate,       "Baudrate: ",     __note_port_rate_en);
    ____do_set(__1_port_data,       "DataBits: ",     __note_port_data_en);
    ____do_set(__1_port_stop,       "StopBits: ",     __note_port_stop_en);
    ____do_set(__1_port_parity,     "Parity:   ",     __note_port_parity_en);
    ____do_set(__1_port_irq,        "IRQ:      ",     __note_port_irq_en);

    ____do_set(__2_port,            " M-2 Port ",     NULL);
    ____do_set(__2_port_base,       "PortAddr: ",     __note_port_base_en);
    ____do_set(__2_port_rate,       "Baudrate: ",     __note_port_rate_en);
    ____do_set(__2_port_data,       "DataBits: ",     __note_port_data_en);
    ____do_set(__2_port_stop,       "StopBits: ",     __note_port_stop_en);
    ____do_set(__2_port_parity,     "Parity:   ",     __note_port_parity_en);
    ____do_set(__2_port_irq,        "IRQ:      ",     __note_port_irq_en);

    ____do_set(__cim_port,          " CIM Port ",     NULL);
    ____do_set(__cim_port_base,     "PortAddr: ",     __note_port_base_en);
    ____do_set(__cim_port_rate,     "Baudrate: ",     __note_port_rate_en);
    ____do_set(__cim_port_data,     "DataBits: ",     __note_port_data_en);
    ____do_set(__cim_port_stop,     "StopBits: ",     __note_port_stop_en);
    ____do_set(__cim_port_parity,   "Parity:   ",     __note_port_parity_en);
    ____do_set(__cim_port_irq,      "IRQ:      ",     __note_port_irq_en);

    ____do_set(__slc_ctrl,          " SLC Control ",  NULL);
    ____do_set(__slc_start_mode,    "StartMode: ",    "Start Mode(Not Used)");

    ____do_set(__misc,              " MISC ",         NULL);
    ____do_set(__slc_language,      "Language:  ",    "Language: 0=��������(GB2312), 1=ENGLISH");

    #undef ____do_set

    slc_global_param_dlg = input_dialog_initialize(&slc_global_param_dialog);
    input_dialog_set_buttons_caption(&slc_global_param_dialog, 
                                     pick_string("ȷ��[F10]", "OK.[F10]"), 
                                     pick_string("ȡ��[ESC]", "No.[ESC]"));
    input_dialog_set_ok_comment(&slc_global_param_dialog, 
                                     pick_string("ȷ��, �밴F10��س���", "Press F10/Enter to Confirm"));
    input_dialog_set_cancel_comment(&slc_global_param_dialog, 
                                     pick_string("ȡ��, �밴ESC��س���", "Press ESC/Enter to Cancel"));
    gui_widget_link(NULL, slc_global_param_dlg);

    return slc_global_param_dlg;
}

static void ____slc_global_param_alert(char * s)
{
    input_dialog_alert(&slc_global_param_dialog, s, COLOR_WARNING);
}

static char * __parity_str[] = { "None", "Even", "Odd" };

void slc_global_param_prepare(int id, char *buf, void * data, INT16U opt)
{
    struct slc_config_s * __config;

    FamesAssert(buf);
    FamesAssert(data);
    if(!buf || !data)
        return;

    __config = (struct slc_config_s *)data;

    switch(id){
        case __slc_1_type:
            if(opt & INPUT_DIALOG_PREPARE_OPT_EDITING){
                if(__config->slc[0].slc_type == SLC_TYPE_DOUBLE)
                    buf[0] = '1';
                else
                    buf[0] = '0';
                buf[1] = 0;
            } else {
                if(__config->slc[0].slc_type == SLC_TYPE_DOUBLE){
                    STRCPY(buf, pick_string("��ѹ��", "Dual"));
                } else {
                    STRCPY(buf, pick_string("��ͨ��", "Single"));
                }
            }
            break;
        case __slc_1_k_nr:
            sprintf(buf, "%d", __config->slc[0].k_number);
            break;
        case __slc_1_l_nr:
            sprintf(buf, "%d", __config->slc[0].l_number);
            break;
        case __slc_1_used:
            sprintf(buf, "%d", (__config->slc_used & 0x1)?1:0);
            break;
        case __slc_2_type:
            if(opt & INPUT_DIALOG_PREPARE_OPT_EDITING){
                if(__config->slc[1].slc_type == SLC_TYPE_DOUBLE)
                    buf[0] = '1';
                else
                    buf[0] = '0';
                buf[1] = 0;
            } else {
                if(__config->slc[1].slc_type == SLC_TYPE_DOUBLE){
                    STRCPY(buf, pick_string("��ѹ��", "Dual"));
                } else {
                    STRCPY(buf, pick_string("��ͨ��", "Single"));
                }
            }
            break;
        case __slc_2_k_nr:
            sprintf(buf, "%d", __config->slc[1].k_number);
            break;
        case __slc_2_l_nr:
            sprintf(buf, "%d", __config->slc[1].l_number);
            break;
        case __slc_2_used:
            sprintf(buf, "%d", (__config->slc_used & 0x2)?1:0);
            break;
        case __cim_delayed:
            sprintf(buf, "%d", __config->cim_data_delayed);
            break;
        case __cim_proto_type:
            if(opt & INPUT_DIALOG_PREPARE_OPT_EDITING){
                sprintf(buf, "%d", __config->cim_protocol_type);
            } else {
                switch(__config->cim_protocol_type){
                    case 1:
                        sprintf(buf, "FMS-300");
                        break;
                    case 0:
                    default:
                        sprintf(buf, pick_string("��׼Э��", "Std."));
                        break;
                }
            }
            break;
        case __cim_link_point:
            sprintf(buf, "%d", __config->cim_link_point+1);
            break;
        case __1_port_base:
            sprintf(buf, "%X", __config->slc_1_port.base);
            break;
        case __1_port_rate:
            sprintf(buf, "%ld", __config->slc_1_port.baudrate);
            break;
        case __1_port_data:
            sprintf(buf, "%d", __config->slc_1_port.data);
            break;
        case __1_port_stop:
            sprintf(buf, "%d", __config->slc_1_port.stop);
            break;
        case __1_port_parity:
            sprintf(buf, "%s", __parity_str[__config->slc_1_port.parity]);
            break;
        case __1_port_irq:
            sprintf(buf, "%d", __config->slc_1_port.int_no);
            break;
        case __2_port_base:
            sprintf(buf, "%X", __config->slc_2_port.base);
            break;
        case __2_port_rate:
            sprintf(buf, "%ld", __config->slc_2_port.baudrate);
            break;
        case __2_port_data:
            sprintf(buf, "%d", __config->slc_2_port.data);
            break;
        case __2_port_stop:
            sprintf(buf, "%d", __config->slc_2_port.stop);
            break;
        case __2_port_parity:
            sprintf(buf, "%s", __parity_str[__config->slc_2_port.parity]);
            break;
        case __2_port_irq:
            sprintf(buf, "%d", __config->slc_2_port.int_no);
            break;
        case __cim_port_base:
            sprintf(buf, "%X", __config->cim_port.base);
            break;
        case __cim_port_rate:
            sprintf(buf, "%ld", __config->cim_port.baudrate);
            break;
        case __cim_port_data:
            sprintf(buf, "%d", __config->cim_port.data);
            break;
        case __cim_port_stop:
            sprintf(buf, "%d", __config->cim_port.stop);
            break;
        case __cim_port_parity:
            sprintf(buf, "%s", __parity_str[__config->cim_port.parity]);
            break;
        case __cim_port_irq:
            sprintf(buf, "%d", __config->cim_port.int_no);
            break;
        case __slc_start_mode:
            sprintf(buf, "%d", __config->slc_start_mode);
            break;
        case __slc_language:
            if(opt & INPUT_DIALOG_PREPARE_OPT_EDITING){            
                sprintf(buf, "%d", __config->language);
            } else {
                switch(__config->language){
                    case 0:
                        STRCPY(buf, "��������");
                        break;
                    case 1:
                    default:
                        STRCPY(buf, "ENGLISH");
                        break;
                }
            }
            break;
        default:
            sprintf(buf, "?");
            break;
    } /* switch(id) */

    return;
}

/***************************************************************************************
** ����ֵ: 0=������Ч, 1=������Ч, -1=����������ˢ��
*/
int slc_global_param_finish(int id, char *buf, void * data, KEYCODE key)
{
    INT16U  chg_flag;
    struct slc_config_s * __config;
    int  temp;
    INT32S t32;

    FamesAssert(buf);
    FamesAssert(data);
    if(!buf || !data)
        return 1; /* ���������Ч, ��ô����Ϊ������������Ч�� */

    key = key;

    __config = (struct slc_config_s *)data;

    chg_flag = CHG_OPT_DEC;

    temp = STRtoINT16(buf, chg_flag);
    
    switch(id){
        case __slc_1_type:
            if(temp)
                __config->slc[0].slc_type = SLC_TYPE_DOUBLE;
            else
                __config->slc[0].slc_type = SLC_TYPE_SINGLE;
            break;
        case __slc_1_k_nr:
            if(temp < 3 || temp > 9){
                ____slc_global_param_alert(pick_string("��������, ��ȷ�ķ�Χ��: [3, 9]",
                                                       "K Number Invalid! [3, 9]"));
                return 0;
            }
            __config->slc[0].k_number = temp;
            break;
        case __slc_1_l_nr:
            if(temp < 4 || temp > 16 || (temp & 0x1)){
                ____slc_global_param_alert(pick_string("��������, ��ȷ�ķ�Χ��: [4, 16], ��ֻ����ż��",
                                                       "L Number Invalid! [4, 16], Only Even Number"));
                return 0;
            }
            __config->slc[0].l_number = temp;
            break;
        case __slc_1_used:
            if(temp){
                __config->slc_used |= 0x1;
            } else {
                __config->slc_used &= ~0x1;
            }
            break;
        case __slc_2_type:
            if(temp)
                __config->slc[1].slc_type = SLC_TYPE_DOUBLE;
            else
                __config->slc[1].slc_type = SLC_TYPE_SINGLE;
            break;
        case __slc_2_k_nr:
            if(temp < 3 || temp > 9){
                ____slc_global_param_alert(pick_string("��������, ��ȷ�ķ�Χ��: [3, 9]",
                                                       "K Number Invalid! [3, 9]"));
                return 0;
            }
            __config->slc[1].k_number = temp;
            break;
        case __slc_2_l_nr:
            if(temp < 4 || temp > 16 || (temp & 0x1)){
                ____slc_global_param_alert(pick_string("��������, ��ȷ�ķ�Χ��: [4, 16], ��ֻ����ż��",
                                                       "L Number Invalid! [4, 16], Only Even Number"));
                return 0;
            }
            __config->slc[1].l_number = temp;
            break;
        case __slc_2_used:
            if(temp){
                __config->slc_used |= 0x2;
            } else {
                __config->slc_used &= ~0x2;
            }
            break;
        case __cim_delayed:
            if(temp < 0){
                ____slc_global_param_alert(pick_string("��Ӧ���Ǹ���, ����������!",
                                                       "Invalid Value, Try Again!"));
                return 0;
            }
            __config->cim_data_delayed = temp;
            break;
        case __cim_proto_type:
            __config->cim_protocol_type = temp;
            break;
        case __cim_link_point:
            if(temp == 1 || temp == 2){
                __config->cim_link_point = temp-1;
            } else {
                ____slc_global_param_alert(pick_string("�˲���ֻ������\"1\"����\"2\", ����������!",
                                                       "Only '1' or '2' is Valid, Try Again!"));
                return 0;
            }
            break;
        case __1_port_base:
            temp = STRtoINT16(buf, 0);
            if(temp < 0x100){
                ____slc_global_param_alert(__err_port_base);
                return 0;
            }
            __config->slc_1_port.base = temp;
            break;
        case __1_port_rate:
            t32 = STRtoINT32(buf, CHG_OPT_DEC);
            if(t32 < 2400L || t32 > 115200L){
                ____slc_global_param_alert(__err_port_rate);
                return 0;
            }
            __config->slc_1_port.baudrate = t32;
            break;
        case __1_port_data:
            if(temp < 5 || temp > 8){
                ____slc_global_param_alert(__err_port_data);
                return 0;
            }
            __config->slc_1_port.data = temp;
            break;
        case __1_port_stop:
            if(temp < 1 || temp > 2){
                ____slc_global_param_alert(__err_port_stop);
                return 0;
            }
            __config->slc_1_port.stop = temp;
            break;
        case __1_port_parity:
            switch(buf[0]){
                case 'n':
                case 'N':
                    __config->slc_1_port.parity = COM_PARITY_NONE;
                    break;
                case 'e':
                case 'E':
                    __config->slc_1_port.parity = COM_PARITY_EVEN;
                    break;
                case 'o':
                case 'O':
                    __config->slc_1_port.parity = COM_PARITY_ODD;
                    break;
                default:
                    ____slc_global_param_alert(__err_port_parity);
                    return 0;
            }
            break;
        case __1_port_irq:
            if(temp < 3 || temp > 15){
                ____slc_global_param_alert(__err_port_irq);
                return 0;
            }
            __config->slc_1_port.int_no = temp;
            break;
        case __2_port_base:
            temp = STRtoINT16(buf, 0);
            if(temp < 0x100){
                ____slc_global_param_alert(__err_port_base);
                return 0;
            }
            __config->slc_2_port.base = temp;
            break;
        case __2_port_rate:
            t32 = STRtoINT32(buf, CHG_OPT_DEC);
            if(t32 < 2400L || t32 > 115200L){
                ____slc_global_param_alert(__err_port_rate);
                return 0;
            }
            __config->slc_2_port.baudrate = t32;
            break;
        case __2_port_data:
            if(temp < 5 || temp > 8){
                ____slc_global_param_alert(__err_port_data);
                return 0;
            }
            __config->slc_2_port.data = temp;
            break;
        case __2_port_stop:
            if(temp < 1 || temp > 2){
                ____slc_global_param_alert(__err_port_stop);
                return 0;
            }
            __config->slc_2_port.stop = temp;
            break;
        case __2_port_parity:
            switch(buf[0]){
                case 'n':
                case 'N':
                    __config->slc_2_port.parity = COM_PARITY_NONE;
                    break;
                case 'e':
                case 'E':
                    __config->slc_2_port.parity = COM_PARITY_EVEN;
                    break;
                case 'o':
                case 'O':
                    __config->slc_2_port.parity = COM_PARITY_ODD;
                    break;
                default:
                    ____slc_global_param_alert(__err_port_parity);
                    return 0;
            }
            break;
        case __2_port_irq:
            if(temp < 3 || temp > 15){
                ____slc_global_param_alert(__err_port_irq);
                return 0;
            }
            __config->slc_2_port.int_no = temp;
            break;
        case __cim_port_base:
            temp = STRtoINT16(buf, 0);
            if(temp < 0x100){
                ____slc_global_param_alert(__err_port_base);
                return 0;
            }
            __config->cim_port.base = temp;
            break;
        case __cim_port_rate:
            t32 = STRtoINT32(buf, CHG_OPT_DEC);
            if(t32 < 2400L || t32 > 115200L){
                ____slc_global_param_alert(__err_port_rate);
                return 0;
            }
            __config->cim_port.baudrate = t32;
            break;
        case __cim_port_data:
            if(temp < 5 || temp > 8){
                ____slc_global_param_alert(__err_port_data);
                return 0;
            }
            __config->cim_port.data = temp;
            break;
        case __cim_port_stop:
            if(temp < 1 || temp > 2){
                ____slc_global_param_alert(__err_port_stop);
                return 0;
            }
            __config->cim_port.stop = temp;
            break;
        case __cim_port_parity:
            switch(buf[0]){
                case 'n':
                case 'N':
                    __config->cim_port.parity = COM_PARITY_NONE;
                    break;
                case 'e':
                case 'E':
                    __config->cim_port.parity = COM_PARITY_EVEN;
                    break;
                case 'o':
                case 'O':
                    __config->cim_port.parity = COM_PARITY_ODD;
                    break;
                default:
                    ____slc_global_param_alert(__err_port_parity);
                    return 0;
            }
            break;
        case __cim_port_irq:
            if(temp < 3 || temp > 15){
                ____slc_global_param_alert(__err_port_irq);
                return 0;
            }
            __config->cim_port.int_no = temp;
            break;
        case __slc_start_mode:
            __config->slc_start_mode = temp;
            break;
        case __slc_language:
            if(temp)
                __config->language = 1;
            else 
                __config->language = 0;
            break;            
        default:
            break;
    } /* switch(id) */
    
    return 1;
}

BOOL setup_slc_global_param(void)
{
    BOOL retval;
    struct slc_config_s t;
    extern struct slc_config_s config;
    extern struct slc_tmp_config_s tmp_config;
     
    FamesAssert(slc_global_param_dlg);
    if(!slc_global_param_dlg)
        return fail;

    t = config;    
    gui_show_widget(slc_global_param_dlg);
    retval = input_dialog_method(&slc_global_param_dialog, slc_global_param_prepare, slc_global_param_finish, (void *)&t, 0);
    if(retval){
        lock_kernel();
        copy_to_config(&t);
        save_config();
        unlock_kernel();
        /* �����������´� */
        if(tmp_config.cim_link)
            reopen_cim_link(); 
        /* PLC���´� */
        slc_close_plc();
        slc_open_plc();
    }
    gui_hide_widget(slc_global_param_dlg);
    
    return retval;
}


/*=========================================================================================
 * 
 * ���ļ�����: slc/slc_para.c
 * 
**=======================================================================================*/



