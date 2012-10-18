/*************************************************************************************
 * �ļ�:    slc/slc_ctrl.c
 *
 * ����:    ��ѹ����������
 *
 * ����:    Jun
 *
 * ʱ��:    2011-4-8
*************************************************************************************/
#define  SLC_CONTROL_C
#include <includes.h>
#include "common.h"


BOOL slc_is_fixed_ok(int slc_index);

/*------------------------------------------------------------------------------------
 * ����:    slc_control_service()
 *
 * ����:    SLC����Ҫ���ƹ���(SLC������)
**----------------------------------------------------------------------------------*/
int ord_chg_lock = 0;
#define ____rw(index, addr, value) \
            do { int tmp; \
                 tmp = value; \
                 ___slc_plc_rw_ensure(index, FATEK_PLC_WRITE_M, addr, &tmp, 1); \
            } while(0)

void __daemon slc_control_service(void * data)
{
    struct slc_run_state_s *state, *state1, *state2, old_state[2];
    slc_descriptor_t * slc;
    int  slc_index, OrdChg_Flag;
    int  Start_Flag_delay[2], Start_off_time[2];
    int  Regu_Flag_delay[2], Regu_Off_delay[2]; /* Regu: Regulate */
    int  CimChg_Flag, CimChg_Step, CIM_COK, CIMUP;
    int  ORD_USE[2];
    order_struct order;

    data = data;

    CimChg_Flag = 0;
    CimChg_Step = 0;

    state1 = &config.slc[0].state;
    state2 = &config.slc[1].state;

    MEMSET((INT08S*)state1,        0, sizeof(old_state[0]));
    MEMSET((INT08S*)state2,        0, sizeof(old_state[0]));
    MEMSET((INT08S*)&old_state[0], 0, sizeof(old_state[0]));
    MEMSET((INT08S*)&old_state[1], 0, sizeof(old_state[0]));

again_and_again:
    OrdChg_Flag = 0; /* ������־��λ */

    ORD_USE[0] = 0;
    ORD_USE[1] = 0;
    if(GetOrderForOrdChg(&order, 0)){
        if((config.slc_used & 0x1) && similar_order(&order, &(config.slc[0].working)))
            ORD_USE[0] = 1;
        if((config.slc_used & 0x2) && similar_order(&order, &(config.slc[1].working)))
            ORD_USE[0] = 2;
    }
    if(GetOrderForOrdChg(&order, 1)){
        if((config.slc_used & 0x1) && similar_order(&order, &(config.slc[0].working)))
            ORD_USE[1] = 1;
        if((config.slc_used & 0x2) && similar_order(&order, &(config.slc[1].working)))
            ORD_USE[1] = 2;
    }

    /*
    ** ���ܻ����ź� ------------------------------------------------------------
    */
    if(config.cim_link_point == 0){
        state = state1;
    } else {
        state = state2;
    }
    if(!state->cim_is_link || !state->cim_ord_chg){
        CIM_COK = 0;
    }
    if(config.slc_used == 3){
        if(state->cim_ord_chg && state->cim_is_link){
            if((CimChg_Flag==0) && (CIM_COK==0)){
                CIM_COK = 1;
                CIMUP = 0;
                CimChg_Flag = 1;
                CimChg_Step = 1;
                ____rw(1, PLC_ADDR_K_UP, 0);
                ____rw(1, PLC_ADDR_K_DN, 0);
                ____rw(1, PLC_ADDR_L_UP, 0);
                ____rw(1, PLC_ADDR_L_DN, 0);
                ____rw(2, PLC_ADDR_K_UP, 0);
                ____rw(2, PLC_ADDR_K_DN, 0);
                ____rw(2, PLC_ADDR_L_UP, 0);
                ____rw(2, PLC_ADDR_L_DN, 0);
            }
        }
    }
    if(CimChg_Flag && (CimChg_Step==1)){
        CimChg_Step = 6; 
        if((ORD_USE[0]==1) && (ORD_USE[1]!=1)){  /* ��1������ */
            ____rw(1, PLC_ADDR_ST_CNT, 1);
            ____rw(1, PLC_ADDR_K_UP,   1);
            ____rw(1, PLC_ADDR_L_UP,   1);
            CIMUP = 1;
        }
        if((ORD_USE[0]==2) && (ORD_USE[1]!=2)){  /* ��2������ */
            ____rw(2, PLC_ADDR_ST_CNT, 1);
            ____rw(2, PLC_ADDR_K_UP,   1);
            ____rw(2, PLC_ADDR_L_UP,   1);
            CIMUP = 2;
        }
        if((ORD_USE[0]!=1) && (ORD_USE[1]==1)){  /* ��1������ */
            ____rw(1, PLC_ADDR_ST_CNT, 1);
            ____rw(1, PLC_ADDR_K_DN,   1);
            ____rw(1, PLC_ADDR_L_DN,   1);
        }
        if((ORD_USE[0]!=2) && (ORD_USE[1]==2)){  /* ��2������ */
            ____rw(2, PLC_ADDR_ST_CNT, 1);
            ____rw(2, PLC_ADDR_K_DN,   1);
            ____rw(2, PLC_ADDR_L_DN,   1);
        }        
    } else if(CimChg_Flag && (CIMUP==1)){      /* Ӧ���ǻ�1���� */
        if((!state1->k_down)&&(!state1->l_down)){
            OrdChg_Flag = 1;
            CimChg_Step = 10;
            CimChg_Flag = 0;
            CIMUP = 0;
        }
    } else if(CimChg_Flag && (CIMUP==2)){      /* Ӧ���ǻ�2���� */
        if((!state2->k_down)&&(!state2->l_down)){
            OrdChg_Flag = 2;
            CimChg_Step = 10;  
            CimChg_Flag = 0;
            CIMUP = 0;
        }
    } else {
        CimChg_Flag = 0;
        CimChg_Step = 0;
    }

    /*
    ** ��1 + ��2 ---------------------------------------------------------------
    */
    for(slc_index = 0; slc_index < 2; slc_index++){
        struct slc_run_state_s * old;
        
        if((slc_index == 0) && ((config.slc_used & 0x1) == 0))
            continue; /* ��1û��ʹ�� */
        if((slc_index == 1) && ((config.slc_used & 0x2) == 0))
            continue; /* ��2û��ʹ�� */
        
        slc = &config.slc[slc_index];
        old = &old_state[slc_index];

        state = &slc->state;
        if(edge_up(state->fixed, old->fixed)){ /* ��λ�ź� */
            if(tmp_config.auto_kl){
                slc_kl_down_set(slc_index);
            }
        }
        if(edge_up(state->order_chg, old->order_chg)){ /* �ֶ������ź� */
            do {
                if(ORD_USE[0] != (slc_index+1))
                    break; /* ֻ�е�1�ʵ��Ǳ�����, ���ܻ� */
                slc_kl_up_set(slc_index+1);
                if((!state->k_down)&&(!state->l_down)){
                    /* ���߶������� */
                    OrdChg_Flag = slc_index+1;
                    slc_kl_up_reset(slc_index+1);
                    break;
                }
                TaskDelay(50);
            } while(1);
        }
        if(state->start){ /* �����ź� */
            Start_Flag_delay[slc_index] = 1;
        }
        if(Start_Flag_delay[slc_index] && !state->start){
            Start_off_time[slc_index]++;
            if(Start_off_time[slc_index] >= 20) {
                Start_Flag_delay[slc_index] = 0;
                Start_off_time[slc_index]   = 0 ;
            }
            if(!slc_is_fixed_ok(slc_index+1)){
                slc_send_start(slc_index+1); /* ���û��������λ, �Ǿ�������һ�� */
            }
        }
        if(state->regulate){ /* �ֵ��ź� */
            Start_Flag_delay[slc_index] = 0;
            Regu_Flag_delay[slc_index]  = 1;
        }
        if(Regu_Flag_delay[slc_index] && !state->regulate){
            Regu_Off_delay[slc_index]++;
            if(Regu_Off_delay[slc_index] >= 20){
                Regu_Flag_delay[slc_index] = 0;
                Regu_Off_delay[slc_index]  = 0;
            }
        }
        if(state->stop){ /* ��ͣ�ź� */
            CimChg_Flag = 0;
            CimChg_Step = 0;
            CIMUP       = 0;
            Start_Flag_delay[slc_index] = 0;
        }
        if(Start_Flag_delay[slc_index] || Regu_Flag_delay[slc_index]){
            static int delay = 0;
            if(++delay > 5){ /* ����Ҫ����̫Ƶ�� */
                delay = 0;
                slc_read_act_value(slc_index+1);
            }
        }
    }

    /*
    ** �������� ----------------------------------------------------------------
    */
    if((OrdChg_Flag == 1)||(OrdChg_Flag == 2)){
        int  i, last, next;
        os_mutex_lock(ord_chg_lock);
        if(get_order_nr()>=0){
            DeleteOrder(0); /* ɾ����һ�ʶ��� */
            last = OrdChg_Flag; /* �ղž��ڹ����Ļ��� */
            next = OrdChg_Flag; /* ����ѡ�еĻ���     */
            i = 0;
            if((OrdChg_Flag == 1) && (config.slc_used == 3)){
                next = 2;
                i = 1; /* �ӵ�2�ʵ���ʼ��� */
            }
            if((OrdChg_Flag == 2) && (config.slc_used == 3)){
                next = 1;
                i = 1; /* �ӵ�2�ʵ���ʼ��� */
            }
            for(; i < 5; i++){
                if(!GetOrderForOrdChg(&order, i))
                    break;
                if(similar_order(&order, &(config.slc[last-1].working)))
                    break; /* ��ղ����ĵ�һ��, ����Ҫ�ٷ����� */
                if(config.slc_used == 3){
                    if(similar_order(&order, &(config.slc[next-1].working)))
                        continue; /* ������Ҫ�����Ķ���һ��, ���� */
                }
                /* 
                 * ������, �����Ѿ��ҵ���һ����Ҫ���͵Ķ���, ��ʵ�Ӧ�ñ����͵� "�ղ��ڹ���, ����������, ����Ҫ����׼��" �Ļ�����
                 * Ҳ����last!
                */
                if(!slc_send_order(last, &order, 0)){
                    /* ����ʧ�� */
                }
                break;
            }
        }
        OrdChg_Flag = 0;
        os_mutex_unlock(ord_chg_lock);
    }
    TaskDelay(50);

    goto again_and_again;
}

/*------------------------------------------------------------------------------------
 * ����:    slc_control_service_initialize()
 *
 * ����:    SLC��������ʼ��
**----------------------------------------------------------------------------------*/
void slc_control_service_initialize(void)
{
    HANDLE task;

    task = TaskCreate(slc_control_service, NULL, "slc-control-service", NULL, 2048, PRIO_NORMAL, TASK_CREATE_OPT_NONE);
    if(task == InvalidHandle){
        sys_print("slc_control_service_initialize() failed\n");
        ExitApplication();
    }
}

/*------------------------------------------------------------------------------------
 * ����:    slc_is_fixed_ok()
 *
 * ����:    ���SLC��ʵ��ֵ���趨ֵ, ��ȷ���Ƿ�λ���
**----------------------------------------------------------------------------------*/
static BOOL slc_is_fixed_ok(int slc_index)
{
    slc_descriptor_t * slc;
    int  i;
    int  * k_act, * l_act;
    int  * k_set, * l_set;
    extern struct slc_config_s config;

    FamesAssert(slc_index == 1 || slc_index == 2);

    if(!(slc_index == 1 || slc_index == 2))
        return ok;

    slc = &(config.slc[slc_index-1]);

    k_act = slc->kl_act.k_location;
    l_act = slc->kl_act.l_location;
    k_set = slc->kl_set.k_location;
    l_set = slc->kl_set.l_location;
    
    for(i=0; i<slc->k_number; i++){
        int tmp;
        tmp = k_act[i] - k_set[i];
        if(tmp < 0)
            tmp = -tmp;
        if(tmp > 5)
            return fail;
    }
    for(i=0; i<slc->l_number; i++){
        int tmp;
        tmp = l_act[i] - l_set[i];
        if(tmp < 0)
            tmp = -tmp;
        if(tmp > 5)
            return fail;
    }

    return ok;
}


/*====================================================================================
 * 
 * ���ļ�����: slc/slc_ctrl.c
 * 
**==================================================================================*/

