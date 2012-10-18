/*************************************************************************************
 * 文件:    slc/slc_ctrl.c
 *
 * 描述:    分压机控制例程
 *
 * 作者:    Jun
 *
 * 时间:    2011-4-8
*************************************************************************************/
#define  SLC_CONTROL_C
#include <includes.h>
#include "common.h"


BOOL slc_is_fixed_ok(int slc_index);

/*------------------------------------------------------------------------------------
 * 函数:    slc_control_service()
 *
 * 描述:    SLC的主要控制过程(SLC服务器)
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
    OrdChg_Flag = 0; /* 换单标志复位 */

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
    ** 生管换单信号 ------------------------------------------------------------
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
        if((ORD_USE[0]==1) && (ORD_USE[1]!=1)){  /* 机1刀线上 */
            ____rw(1, PLC_ADDR_ST_CNT, 1);
            ____rw(1, PLC_ADDR_K_UP,   1);
            ____rw(1, PLC_ADDR_L_UP,   1);
            CIMUP = 1;
        }
        if((ORD_USE[0]==2) && (ORD_USE[1]!=2)){  /* 机2刀线上 */
            ____rw(2, PLC_ADDR_ST_CNT, 1);
            ____rw(2, PLC_ADDR_K_UP,   1);
            ____rw(2, PLC_ADDR_L_UP,   1);
            CIMUP = 2;
        }
        if((ORD_USE[0]!=1) && (ORD_USE[1]==1)){  /* 机1刀线下 */
            ____rw(1, PLC_ADDR_ST_CNT, 1);
            ____rw(1, PLC_ADDR_K_DN,   1);
            ____rw(1, PLC_ADDR_L_DN,   1);
        }
        if((ORD_USE[0]!=2) && (ORD_USE[1]==2)){  /* 机2刀线下 */
            ____rw(2, PLC_ADDR_ST_CNT, 1);
            ____rw(2, PLC_ADDR_K_DN,   1);
            ____rw(2, PLC_ADDR_L_DN,   1);
        }        
    } else if(CimChg_Flag && (CIMUP==1)){      /* 应该是机1换单 */
        if((!state1->k_down)&&(!state1->l_down)){
            OrdChg_Flag = 1;
            CimChg_Step = 10;
            CimChg_Flag = 0;
            CIMUP = 0;
        }
    } else if(CimChg_Flag && (CIMUP==2)){      /* 应该是机2换单 */
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
    ** 机1 + 机2 ---------------------------------------------------------------
    */
    for(slc_index = 0; slc_index < 2; slc_index++){
        struct slc_run_state_s * old;
        
        if((slc_index == 0) && ((config.slc_used & 0x1) == 0))
            continue; /* 机1没有使用 */
        if((slc_index == 1) && ((config.slc_used & 0x2) == 0))
            continue; /* 机2没有使用 */
        
        slc = &config.slc[slc_index];
        old = &old_state[slc_index];

        state = &slc->state;
        if(edge_up(state->fixed, old->fixed)){ /* 定位信号 */
            if(tmp_config.auto_kl){
                slc_kl_down_set(slc_index);
            }
        }
        if(edge_up(state->order_chg, old->order_chg)){ /* 手动换单信号 */
            do {
                if(ORD_USE[0] != (slc_index+1))
                    break; /* 只有第1笔单是本机的, 才能换 */
                slc_kl_up_set(slc_index+1);
                if((!state->k_down)&&(!state->l_down)){
                    /* 刀线都已上来 */
                    OrdChg_Flag = slc_index+1;
                    slc_kl_up_reset(slc_index+1);
                    break;
                }
                TaskDelay(50);
            } while(1);
        }
        if(state->start){ /* 启动信号 */
            Start_Flag_delay[slc_index] = 1;
        }
        if(Start_Flag_delay[slc_index] && !state->start){
            Start_off_time[slc_index]++;
            if(Start_off_time[slc_index] >= 20) {
                Start_Flag_delay[slc_index] = 0;
                Start_off_time[slc_index]   = 0 ;
            }
            if(!slc_is_fixed_ok(slc_index+1)){
                slc_send_start(slc_index+1); /* 如果没有真正定位, 那就再启动一次 */
            }
        }
        if(state->regulate){ /* 手调信号 */
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
        if(state->stop){ /* 急停信号 */
            CimChg_Flag = 0;
            CimChg_Step = 0;
            CIMUP       = 0;
            Start_Flag_delay[slc_index] = 0;
        }
        if(Start_Flag_delay[slc_index] || Regu_Flag_delay[slc_index]){
            static int delay = 0;
            if(++delay > 5){ /* 不需要读的太频繁 */
                delay = 0;
                slc_read_act_value(slc_index+1);
            }
        }
    }

    /*
    ** 换单过程 ----------------------------------------------------------------
    */
    if((OrdChg_Flag == 1)||(OrdChg_Flag == 2)){
        int  i, last, next;
        os_mutex_lock(ord_chg_lock);
        if(get_order_nr()>=0){
            DeleteOrder(0); /* 删除第一笔订单 */
            last = OrdChg_Flag; /* 刚才就在工作的机器 */
            next = OrdChg_Flag; /* 将被选中的机器     */
            i = 0;
            if((OrdChg_Flag == 1) && (config.slc_used == 3)){
                next = 2;
                i = 1; /* 从第2笔单开始检查 */
            }
            if((OrdChg_Flag == 2) && (config.slc_used == 3)){
                next = 1;
                i = 1; /* 从第2笔单开始检查 */
            }
            for(; i < 5; i++){
                if(!GetOrderForOrdChg(&order, i))
                    break;
                if(similar_order(&order, &(config.slc[last-1].working)))
                    break; /* 与刚才做的单一样, 不需要再发送了 */
                if(config.slc_used == 3){
                    if(similar_order(&order, &(config.slc[next-1].working)))
                        continue; /* 与马上要工作的订单一样, 跳过 */
                }
                /* 
                 * 到这里, 我们已经找到了一笔需要发送的订单, 这笔单应该被发送到 "刚才在工作, 而现在已起刀, 并需要作好准备" 的机器上
                 * 也就是last!
                */
                if(!slc_send_order(last, &order, 0)){
                    /* 发送失败 */
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
 * 函数:    slc_control_service_initialize()
 *
 * 描述:    SLC服务器初始化
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
 * 函数:    slc_is_fixed_ok()
 *
 * 描述:    检查SLC的实际值与设定值, 以确认是否定位完成
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
 * 本文件结束: slc/slc_ctrl.c
 * 
**==================================================================================*/

