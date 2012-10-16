/***********************************************************************************************
 * �ļ�:    slc/plc_def.h
 *
 * ˵��:    PLC�ݴ�������
 *
 * ����:    Jun
 *
 * ʱ��:    2011-4-9 ~ 2011-4-21
***********************************************************************************************/
#ifndef SLC_PLC_DEF_H
#define SLC_PLC_DEF_H

/*----------------------------------------------------------------------------------------------
 * 
 *      PLC��ַ
 * 
**--------------------------------------------------------------------------------------------*/

#define PLC_ADDR_START      PLC_ADDR_CONTROL[0]     /* ����     */
#define PLC_ADDR_KL_UP      PLC_ADDR_CONTROL[1]     /* ������   */
#define PLC_ADDR_KL_DN      PLC_ADDR_CONTROL[2]     /* ������   */
#define PLC_ADDR_ST_CNT     PLC_ADDR_CONTROL[3]     /* �Ƴ���ʼ */
#define PLC_ADDR_K_UP       PLC_ADDR_CONTROL[4]     /* ����     */
#define PLC_ADDR_K_DN       PLC_ADDR_CONTROL[5]     /* ����     */
#define PLC_ADDR_L_UP       PLC_ADDR_CONTROL[6]     /* ����     */
#define PLC_ADDR_L_DN       PLC_ADDR_CONTROL[7]     /* ����     */
#define PLC_ADDR_ST_REG     PLC_ADDR_CONTROL[8]     /* �������� */
#define PLC_ADDR_OIL_ON     PLC_ADDR_CONTROL[9]     /* PC���Ϳ� */
#define PLC_ADDR_CLR_FIX    PLC_ADDR_CONTROL[10]    /* ��λ��� */

#ifdef  SLC_PLC_C

char   PLC_ADDR_RUN_STATE[]   = "WM0352";   /* ��ѹ������״̬:
                                               ��λ���:   m352    :0
                                               �ֵ�:       m353    :1
                                               ����:       m354    :2
                                               ����:       m355    :3
                                               ����:       m356    :4
                                               ��ͣ:       m357    :5
                                               ����:       m359    :7
                                               �������:   m362    :10
                                               ���ܻ���:   m366    :14
                                               �����Զ�:   m367    :15
                                            */
char * PLC_ADDR_CONTROL[]     = { /* ��ѹ������ */
                                  "M0224",  /* ����     */
                                  "M0184",  /* ������   */
                                  "M0183",  /* ������   */
                                  "M0312",  /* �Ƴ���ʼ */
                                  "M0317",  /* ����     */
                                  "M0318",  /* ����     */
                                  "M0319",  /* ����     */
                                  "M0320",  /* ����     */
                                  "M0328",  /* �������� */
                                  "M0329",  /* PC���Ϳ� */
                                  "M0452",  /* ��λ��� */
};

char   PLC_ADDR_TIME_TICK[]   = "R04151";  /* PLCʱ�ӵδ�           */

char * PLC_ADDR_SET_VALUE_K[] = { /* ���趨ֵ */
                                  "R01914", "R01916", "R01918", "R01920", "R01922", 
                                  "R02350", "R02352", "R02354", "R02356", "R02358",
};
char * PLC_ADDR_SELECTED_K[]  = { /* ��ѡ��״̬ */
                                  "M0185", "M0186", "M0187", "M0188", "M0189", 
                                  "M0190", "M0191", "M0192", "M0193", "M0194",
};
char * PLC_ADDR_FINE_TUNE_K[] = { /* ��΢������ */
                                  "M0820", "M0821", "M0822", "M0823", "M0824", 
                                  "M0825", "M0826", "M0827", "M0828", "M0829",
};
char * PLC_ADDR_SET_VALUE_L[] = { /* ���趨ֵ */
                                  "R02080", "R02082", "R02084", "R02086", "R02088", 
                                  "R02090", "R02092", "R02140", "R02354", "R02356",
                                  "R02358", "R02360", "R02104", "R02106", "R02108",
};
char * PLC_ADDR_SELECTED_L[]  = { /* ��ѡ��״̬ */
                                  "M0190", "M0191",
                                  "M0217", "M0218", "M0219", "M0220", "M0221", 
                                  "M0222", "M0223", "M0224", "M0225", "M0226", 
                                  "M0227", "M0228", "M0229", "M0230", "M0231", 
};
char * PLC_ADDR_FINE_TUNE_L[] = { /* ��΢������ */
                                  "M0825", "M0826", "M0827", "M0828", "M0829",
                                  "M0830", "M0831", "M0832", "M0833", "M0834",
                                  "M0835", "M0836", "M0837", "M0838", "M0839",
};
char   PLC_ADDR_FINE_TUNE[]   = "M0680";   /* ����΢��������        */
char   PLC_ADDR_YX[]          = "M0750";   /* ѹ����ʼ��ַ          */
char   PLC_ADDR_SET_PRESS1[]  = "R01926";  /* ǰѹ�趨ֵ            */
char   PLC_ADDR_SET_PRESS2[]  = "R01926";  /* ��ѹ�趨ֵ            */
char   PLC_ADDR_SET_FAN[]     = "R01924";  /* ������趨ֵ          */

char   PLC_ADDR_ACT_VALUE_K[] = "R02750";  /* ��ʵ��ֵ��ʼ��ַ      */
char   PLC_ADDR_ACT_VALUE_L[] = "R02764";  /* ��ʵ��ֵ��ʼ��ַ      */
char   PLC_ADDR_ACT_PRESS1[]  = "R01912";  /* ǰѹʵ��ֵ            */
char   PLC_ADDR_ACT_PRESS2[]  = "R01912";  /* ��ѹʵ��ֵ            */
char   PLC_ADDR_ACT_FAN[]     = "R01910";  /* �����ʵ��ֵ          */

char   PLC_ADDR_ADJ_K_CFM[]   = "M0167";   /* ��У��ȷ��ֵ��ʼ��ַ  */
char * PLC_ADDR_ADJUST_K[]    = {          /* ��У��ֵ��ʼ��ַ      */
                                  "R01956", "R01958", "R01960", "R01962", "R01964", 
                                  "R01964", "R01964", "R01964", "R01964", "R01964",
};
char   PLC_ADDR_ADJ_L_CFM[]   = "M0200";   /* ��У��ȷ��ֵ��ʼ��ַ  */
char * PLC_ADDR_ADJUST_L[]    = {          /* ��У��ֵ��ʼ��ַ      */
                                  "R02122", "R02124", "R02126", "R02128", "R02130", 
                                  "R02132", "R02134", "R02146", "R02148", "R02148",
                                  "R02148", "R02148", "R02148", "R02148", "R02148",
};
char   PLC_ADDR_ADJ_PRS1[]    = "R01966";  /* ѹ��1У��ֵ��ַ       */
char   PLC_ADDR_ADJ_P1_CFM[]  = "M0172";   /* ѹ��1У��ȷ��ֵ��ַ   */
char   PLC_ADDR_ADJ_PRS2[]    = "R01966";  /* ѹ��2У��ֵ��ַ       */
char   PLC_ADDR_ADJ_P2_CFM[]  = "M0172";   /* ѹ��1У��ȷ��ֵ��ַ   */
char   PLC_ADDR_ADJ_FAN[]     = "R01968";  /* ����У��ֵ��ַ        */
char   PLC_ADDR_ADJ_FAN_CFM[] = "M0173";   /* ����У��ȷ��ֵ��ַ    */

char   PLC_ADDR_ADJ_YX[]      = "R01970";  /* ѹ��У��ֵ��ַ        */ /*FIXME: ��ʱû���õ�  */
char   PLC_ADDR_ADJ_YX_CFM[]  = "M0174";   /* ѹ��У��ȷ��ֵ��ַ    */

char   PLC_ADDR_REGRESS_K[]   = "R03000";  /* ������ֵ��ʼ��ַ      */
char   PLC_ADDR_REGRESS_L[]   = "R03030";  /* �߹���ֵ��ʼ��ַ      */

char   PLC_ADDR_UNIT_ALL[]    = "R01980";  /* ���ߵ�λֵͳһֵ, �뵶1��ͬ */
char   PLC_ADDR_UNIT_K[]      = "R01942";  /* ����λֵ��ʼ��ַ      */
char * PLC_ADDR_UNIT_L[]      = {          /* �ߵ�λֵ��ʼ��ַ      */
                                  "R02108", "R02110", "R02112", "R02114", "R02116", 
                                  "R02118", "R02120", "R02144", "R02148", "R02148",
                                  "R02148", "R02148", "R02148", "R02148", "R02148",
};
char   PLC_ADDR_UNIT_P1[]     = "R01954";  /* ѹ��1��λֵ��ַ       */
char   PLC_ADDR_UNIT_P2[]     = "R01954";  /* ѹ��2��λֵ��ַ       */
char   PLC_ADDR_UNIT_FAN[]    = "R01952";  /* ���絥λֵ��ַ        */

char   PLC_ADDR_FIX_K[]       = "R01928";  /* ��+/-ֵ��ʼ��ַ       */
char * PLC_ADDR_FIX_L[]       = {          /* ��+/-ֵ��ʼ��ַ       */
                                  "R02094", "R02096", "R02098", "R02100", "R02102", 
                                  "R02104", "R02106", "R02142", "R02148", "R02148",
                                  "R02148", "R02148", "R02148", "R02148", "R02148",
};
char   PLC_ADDR_FIX_P1[]      = "R01940";  /* ѹ��1 +/-ֵ��ַ       */
char   PLC_ADDR_FIX_P2[]      = "R01940";  /* ѹ��2 +/-ֵ��ַ       */
char   PLC_ADDR_FIX_FAN[]     = "R01938";  /* ����  +/-ֵ��ַ       */
char   PLC_ADDR_FIX_BIG[]     = "R02260";  /* ������+/-ֵ��ַ       */

char   PLC_ADDR_K_BIG_FIX[]   = "R02000";  /* ���ֶ�λ��ַ          */
char   PLC_ADDR_K_SML_FIX[]   = "R02002";  /* ��ϸ��λ��ַ          */
char   PLC_ADDR_L_BIG_FIX[]   = "R02004";  /* �ߴֶ�λ��ַ          */
char   PLC_ADDR_L_SML_FIX[]   = "R02006";  /* ��ϸ��λ��ַ          */
char   PLC_ADDR_P_BIG_FIX[]   = "R02008";  /* ѹ��ֶ�λ��ַ        */
char   PLC_ADDR_P_SML_FIX[]   = "R02010";  /* ѹ��ϸ��λ��ַ        */
char   PLC_ADDR_F_BIG_FIX[]   = "R02012";  /* ����ֶ�λ��ַ        */
char   PLC_ADDR_F_SML_FIX[]   = "R02014";  /* ����ϸ��λ��ַ        */

char   PLC_ADDR_MALFUNCTION[] = "WM1000";  /* ���ϴ�����ʼ��ַ      */

char   PLC_ADDR_WHET_RUN[]    = "R01814";  /* ĥ��: ��������(16λ)  */
char   PLC_ADDR_WHET_SET[]    = "R01807";  /* ĥ��: �趨����(16λ)  */
char   PLC_ADDR_WHET_WHT[]    = "R01800";  /* ĥ��: ĥ������(16λ)  */
char   PLC_ADDR_WHET_ACC[]    = "R01820";  /* ĥ��: �ۼ�����(32λ)  */
char   PLC_ADDR_OIL_LENG[]    = "R01850";  /* ĥ��: ��������(16λ)  */
char   PLC_ADDR_OIL_TIME[]    = "R01860";  /* ĥ��: ����ʱ��(16λ)  */

char   PLC_ADDR_CTRL_K_DN[]   = "R01700";  /* ����: ��������        */
char   PLC_ADDR_CTRL_K_UP[]   = "R01710";  /* ����: ��������        */
char   PLC_ADDR_CTRL_L_DN[]   = "R01702";  /* ����: ��������        */
char   PLC_ADDR_CTRL_L_UP[]   = "R01712";  /* ����: ��������        */
char   PLC_ADDR_CTRL_L2_DN[]  = "R01706";  /* ����: ��2������       */
char   PLC_ADDR_CTRL_L2_UP[]  = "R01708";  /* ����: ��2������       */
char   PLC_ADDR_CTRL_K_DN_DLY[] 
                              = "R01714";  /* ����: �����ӳ�        */
char   PLC_ADDR_CTRL_K_UP_DLY[] 
                              = "R01716";  /* ����: �����ӳ�        */
char   PLC_ADDR_CTRL_L_DN_DLY[] 
                              = "R01982";  /* ����: �����ӳ�        */
char   PLC_ADDR_CTRL_L_UP_DLY[] 
                              = "R01984";  /* ����: �����ӳ�        */
char   PLC_ADDR_CTRL_L2_DN_DLY[] 
                              = "R01992";  /* ����: ��2���ӳ�       */
char   PLC_ADDR_CTRL_L2_UP_DLY[] 
                              = "R01994";  /* ����: ��2���ӳ�       */

char   PLC_ADDR_SPG[]         = "R00314";  /* ����SPG(32λ)         */
char   PLC_ADDR_YXBIG[]       = "R00310";  /* YXBIG                 */
char   PLC_ADDR_YXSMALL[]     = "R00312";  /* YXSMALL               */

#else
extern char   PLC_ADDR_RUN_STATE[];
extern char * PLC_ADDR_CONTROL[];

extern char   PLC_ADDR_TIME_TICK[];

extern char * PLC_ADDR_SET_VALUE_K[];
extern char * PLC_ADDR_SET_VALUE_L[];
extern char * PLC_ADDR_SELECTED_K[];
extern char * PLC_ADDR_SELECTED_L[];
extern char * PLC_ADDR_FINE_TUNE_K[];
extern char * PLC_ADDR_FINE_TUNE_L[];
extern char   PLC_ADDR_FINE_TUNE[];
extern char   PLC_ADDR_YX[];
extern char   PLC_ADDR_SET_PRESS1[];
extern char   PLC_ADDR_SET_PRESS2[];
extern char   PLC_ADDR_SET_FAN[];

extern char   PLC_ADDR_ACT_VALUE_K[];
extern char   PLC_ADDR_ACT_VALUE_L[];
extern char   PLC_ADDR_ACT_PRESS1[];
extern char   PLC_ADDR_ACT_PRESS2[];
extern char   PLC_ADDR_ACT_FAN[];

extern char * PLC_ADDR_ADJUST_K[];
extern char * PLC_ADDR_ADJUST_L[];
extern char   PLC_ADDR_ADJ_K_CFM[];
extern char   PLC_ADDR_ADJ_L_CFM[];
extern char   PLC_ADDR_ADJ_PRS1[];
extern char   PLC_ADDR_ADJ_P1_CFM[];
extern char   PLC_ADDR_ADJ_PRS2[];
extern char   PLC_ADDR_ADJ_P2_CFM[];
extern char   PLC_ADDR_ADJ_FAN[];
extern char   PLC_ADDR_ADJ_FAN_CFM[];

extern char   PLC_ADDR_REGRESS_K[];
extern char   PLC_ADDR_REGRESS_L[];

extern char   PLC_ADDR_UNIT_ALL[];
extern char   PLC_ADDR_UNIT_K[];
extern char * PLC_ADDR_UNIT_L[];
extern char   PLC_ADDR_UNIT_P1[];
extern char   PLC_ADDR_UNIT_P2[];
extern char   PLC_ADDR_UNIT_FAN[];

extern char   PLC_ADDR_FIX_K[];
extern char * PLC_ADDR_FIX_L[];
extern char   PLC_ADDR_FIX_P1[];
extern char   PLC_ADDR_FIX_P2[];
extern char   PLC_ADDR_FIX_FAN[];
extern char   PLC_ADDR_FIX_BIG[];

extern char   PLC_ADDR_K_BIG_FIX[];
extern char   PLC_ADDR_K_SML_FIX[];
extern char   PLC_ADDR_L_BIG_FIX[];
extern char   PLC_ADDR_L_SML_FIX[];
extern char   PLC_ADDR_P_BIG_FIX[];
extern char   PLC_ADDR_P_SML_FIX[];
extern char   PLC_ADDR_F_BIG_FIX[];
extern char   PLC_ADDR_F_SML_FIX[];

extern char   PLC_ADDR_MALFUNCTION[];

extern char   PLC_ADDR_WHET_RUN[];
extern char   PLC_ADDR_WHET_SET[];
extern char   PLC_ADDR_WHET_WHT[];
extern char   PLC_ADDR_WHET_ACC[];
extern char   PLC_ADDR_OIL_LENG[];
extern char   PLC_ADDR_OIL_TIME[];

extern char   PLC_ADDR_CTRL_K_DN[];
extern char   PLC_ADDR_CTRL_K_UP[];
extern char   PLC_ADDR_CTRL_L_DN[];
extern char   PLC_ADDR_CTRL_L_UP[];
extern char   PLC_ADDR_CTRL_L2_DN[];
extern char   PLC_ADDR_CTRL_L2_UP[];
extern char   PLC_ADDR_CTRL_K_DN_DLY[];
extern char   PLC_ADDR_CTRL_K_UP_DLY[];
extern char   PLC_ADDR_CTRL_L_DN_DLY[];
extern char   PLC_ADDR_CTRL_L_UP_DLY[];
extern char   PLC_ADDR_CTRL_L2_DN_DLY[];
extern char   PLC_ADDR_CTRL_L2_UP_DLY[];

extern char   PLC_ADDR_SPG[];
extern char   PLC_ADDR_YXBIG[];
extern char   PLC_ADDR_YXSMALL[];

#endif


/*----------------------------------------------------------------------------------------------
 * 
 *      PLC-ACTION ID
 * 
**--------------------------------------------------------------------------------------------*/
enum ___plc_action_id {
    id_plc_tick,
    id_state,
    id_io_x,
    id_io_y,
    id_read_act_k,
    id_read_act_l,
    id_read_act_p1,
    id_read_act_p2,
    id_read_act_fan,
    id_malfunction,
    id_whet_run,
    id_whet_acc,
};

#ifdef ___USE_PLC_DEF___

static INT16S r4151 = 0;
static INT16S plc_state = 0;
static INT16S __plc_io_x[2];
static INT16S __plc_io_y[2];
static INT32S __slc_malfunction[1];
static INT32S __act_k[SLC_K_MAX_NR];
static INT32S __act_l[SLC_L_MAX_NR];
static INT32S __act_p1[1];
static INT32S __act_p2[1];
static INT32S __act_fan[1];
static INT16S __whet_run[SLC_K_MAX_NR];
static INT32S __whet_acc[SLC_K_MAX_NR];

static void ___finish(int id, BOOL success)
{
    slc_descriptor_t * slc;
    extern int plc_tick[];
    extern INT32S slc_malfunction[];
    extern struct slc_config_s config;

    slc = &(config.slc[___THIS_SLC___ - 1]);

    switch(id){
        case id_state:
            if(success){
                struct slc_run_state_s * state;
                struct bits16 * bits;
                state = &slc->state;
                bits  = (struct bits16 *)&plc_state;
                lock_kernel();
                state->fixed       = bits->b0;  /* m352 */
                state->regulate    = bits->b1;  /* m353 */
                state->start       = bits->b2;  /* m354 */
                state->k_down      = bits->b3;  /* m355 */
                state->l_down      = bits->b4;  /* m356 */
                state->stop        = bits->b5;  /* m357 */
                state->order_chg   = bits->b7;  /* m359 */
                state->regressed   = bits->b10; /* m362 */
                state->cim_ord_chg = bits->b14; /* m366 */
                state->cim_is_link = bits->b15; /* m367 */
                unlock_kernel();
            }
            break;
        case id_plc_tick:
            if(success){
                plc_tick[___THIS_SLC___ - 1] = r4151;
            }
            break;
        case id_malfunction:
            if(success){
                slc_malfunction[___THIS_SLC___ - 1] = __slc_malfunction[0];
            }
            break;
        case id_read_act_k:
            if(success){
                int i;
                for(i = 0; i < slc->k_number; i++)
                    slc->kl_act.k_location[i] = (int)__act_k[i];
            }
            break;
        case id_read_act_l:
            if(success){
                int i;
                for(i = 0; i < slc->l_number; i++)
                    slc->kl_act.l_location[i] = (int)__act_l[i];
            }
            break;
        case id_read_act_p1:
            if(success){
                slc->kl_act.press_1_location = (int)__act_p1[0];
            }
            break;
        case id_read_act_p2:
            if(success){
                slc->kl_act.press_2_location = (int)__act_p2[0];
            }
            break;
        case id_read_act_fan:
            if(success){
                slc->kl_act.fan_location = (int)__act_fan[0];
            }
            break;
        case id_whet_run:
            if(success){
                int i;
                for(i = 0; i < slc->k_number; i++)
                    slc->k_whet.whet_run[i] = __whet_run[i];
            }
            break;
        case id_whet_acc:
            if(success){
                int i;
                for(i = 0; i < slc->k_number; i++)
                    slc->k_whet.whet_acc[i] = __whet_acc[i];
            }
            break;
        default:
            break;
    }

    return;
}

static BEGIN_PLC_ACTION_MAP(___actions)
PLC_MAP_LINK(id_state,          PLC_ADDR_RUN_STATE,     &plc_state,         1,  FATEK_PLC_READ_R,    0, ___finish, 0)
PLC_MAP_LINK(id_plc_tick,       PLC_ADDR_TIME_TICK,     &r4151,             1,  FATEK_PLC_READ_R,    0, ___finish, 0)
PLC_MAP_LINK(id_io_x,           "WX0000",               __plc_io_x,         2,  FATEK_PLC_READ_R,   20, ___finish, 1)
PLC_MAP_LINK(id_io_y,           "WY0000",               __plc_io_y,         2,  FATEK_PLC_READ_R,   20, ___finish, 1)
PLC_MAP_LINK(id_malfunction,    PLC_ADDR_MALFUNCTION,   __slc_malfunction,  1,  FATEK_PLC_READ_DR, 100, ___finish, 0)
PLC_MAP_LINK(id_read_act_k,     PLC_ADDR_ACT_VALUE_K,   __act_k,            1,  FATEK_PLC_READ_DR,  50, ___finish, 1)
PLC_MAP_LINK(id_read_act_l,     PLC_ADDR_ACT_VALUE_L,   __act_l,            1,  FATEK_PLC_READ_DR,  50, ___finish, 1)
PLC_MAP_LINK(id_read_act_p1,    PLC_ADDR_ACT_PRESS1,    __act_p1,           1,  FATEK_PLC_READ_DR,  50, ___finish, 1)
PLC_MAP_LINK(id_read_act_p2,    PLC_ADDR_ACT_PRESS2,    __act_p2,           1,  FATEK_PLC_READ_DR,  50, ___finish, 1)
PLC_MAP_LINK(id_read_act_fan,   PLC_ADDR_ACT_FAN,       __act_fan,          1,  FATEK_PLC_READ_DR,  50, ___finish, 1)
PLC_MAP_LINK(id_whet_run,       PLC_ADDR_WHET_RUN,      __whet_run,         1,  FATEK_PLC_READ_R,   25, ___finish, 1)
PLC_MAP_LINK(id_whet_acc,       PLC_ADDR_WHET_ACC,      __whet_acc,         1,  FATEK_PLC_READ_DR,  25, ___finish, 1)
END_PLC_ACTION_MAP();

static int ___started = 0;

static BOOL slc_start_plc(void)
{
    BOOL retval;
    int  index;
    PLC * plc;
    extern PLC * slc_plc[];
    extern struct slc_config_s config;
    char * svc_name[2] = {"plc-service [1]", "plc-service [2]"};

    if(___started)
        return ok;

    index = ___THIS_SLC___ - 1;
    
    plc = plc_alloc();

    if(!plc)
        goto out;

    #if ___THIS_SLC___ == 1
    retval = plc_set_param(plc,
                PLC_TYPE_FATEK,
                svc_name[index],
                NULL,
                1,  /* PLCվ�� */
                config.slc_1_port.base,
                config.slc_1_port.int_no,
                config.slc_1_port.baudrate,
                config.slc_1_port.parity,
                config.slc_1_port.data,
                config.slc_1_port.stop,
                COM_FIFO_TL_4
               );
    #elif ___THIS_SLC___ == 2
    retval = plc_set_param(plc,
                PLC_TYPE_FATEK,
                svc_name[index],
                NULL,
                1,  /* PLCվ�� */
                config.slc_2_port.base,
                config.slc_2_port.int_no,
                config.slc_2_port.baudrate,
                config.slc_2_port.parity,
                config.slc_2_port.data,
                config.slc_2_port.stop,
                COM_FIFO_TL_4
               );
    #else
    #error "___THIS_SLC___ has a invalid value, because it only can be 1 or 2"
    #endif

    if(!retval)
        goto out2;

    retval = open_plc(plc);

    if(!retval) 
        goto out2;

    retval = do_plc_action_map(plc, ___actions);
    if(!retval)
        goto out1;

    lock_kernel();
    ___started = 1;
    slc_plc[index] = plc;
    unlock_kernel();

    retval = ok;

out:
    return retval ;
    
out1:
    shut_plc(plc);

out2:
    plc_free(plc);
    retval = fail;
    goto out;    
}

static BOOL slc_stop_plc(void)
{
    int  index;
    extern PLC * slc_plc[];

    if(!___started)
        return ok;

    index = ___THIS_SLC___ - 1;

    if(slc_plc[index]){
        shut_plc(slc_plc[index]);
        plc_free(slc_plc[index]);
    }
    
    lock_kernel();
    ___started = 0;
    slc_plc[index] = NULL;
    unlock_kernel();

    return ok;
}

static void ___initialize_about_plc(void)
{
    int  index;
    extern int * plc_io_x[];
    extern int * plc_io_y[];

    index = ___THIS_SLC___ - 1;

    lock_kernel();
    plc_io_x[index] = __plc_io_x;
    plc_io_y[index] = __plc_io_y;


    unlock_kernel();
}


#endif /* #ifdef ___USE_PLC_DEF___ */

#endif /* #ifdef SLC_PLC_DEF_H     */

/*==============================================================================================
 * 
 * ���ļ�����: slc/plc_def.h
 * 
**============================================================================================*/

