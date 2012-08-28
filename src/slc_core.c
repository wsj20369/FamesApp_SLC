/**************************************************************************************
 * 文件:    slc/core.c
 *
 * 说明:    SLC排单算法
 *
 * 作者:    Jun
 *
 * 时间:    2011-01-11
**************************************************************************************/
#define  SLC_CORE_C
#include <FamesOS.h>
#include "common.h"

/*lint -e534 -e717 -e563*/
#define SLC_DEBUGx
#define SLC_PAUSE  1

#ifdef  SLC_DEBUG
#define xxx_debug32(s,x)   printf("\n%s, "#x" = %08lX\n", s, x)
#define xxx_debug16(x)     printf("\n"#x" = %d", x)
static  int __i__;
#else
#define xxx_debug32(s,x)
#define xxx_debug16(x)
#endif

/*------------------------------------------------------------------------------------
 * 
 *                  ......
 * 
**----------------------------------------------------------------------------------*/
#define __slc_is_prepress(flag)  ((flag)&SLC_FLAG_PREP)


/*------------------------------------------------------------------------------------
 * 函数:    slc_setup_to_default()
 *
 * 描述:    将SLC设为默认状态
**----------------------------------------------------------------------------------*/
BOOL slc_setup_to_default(slc_descriptor_t * slc)
{
    FamesAssert(slc);

    if(!slc)
        return fail;
    
    *slc = __default_slc;

    return ok;
}

/*------------------------------------------------------------------------------------
 * 函数:    slc_check_setup()
 *
 * 描述:    检查参数设置
 *
 * 说明:    这里其实应该将错误码细分的, 不过以后再说吧
**----------------------------------------------------------------------------------*/
INT32U slc_check_setup(slc_descriptor_t * slc, INT16U flag)
{
    INT32U err;

    flag = flag;

    err = SLC_ERR_INVALID_SETUP;

    if(slc->l_number & 1){ /* 线数不能是奇数 */
        return err;
    }
    if(slc->k_number < 3){ /* 刀数不能小于3  */
        return err;
    }
    if(slc->l_number < 4){ /* 线数不能小于4  */
        return err;
    }
    if(slc->hw_width > 32000){ /* 机械幅宽不能大于3200mm(3米2) */
        return err;
    }

    return SLC_ERR_NONE;
}

/*------------------------------------------------------------------------------------
 * 
 *                  中心偏移数组的生成
 * 
 * 函数:    make_slc_departure()
 *
 * 描述:    根据刀线"不可动"状态生成中心偏移数组
 *
 * 参数:    slc     SLC描述符, 其中包括了刀线相关的信息, 及
 *                  订单中的刀线位置信息(已按中心对称).
 *          flag    选项
 * 
 * 输出:    >=0     中心偏移数组的大小
 *          <0      错误码
 *
 * 说明:    中心偏移是指订单的中心线与机台的中心线之偏差,
 *          在排单时，由于种种原因(如不规则订单, 某刀线不可动等),
 *          订单的中心线不能放在机台的中心线上, 这时候, 就需要
 *          确定偏差的大小(不止一个, 所以用数组来表示)
**----------------------------------------------------------------------------------*/
#define SLC_DEPARTURE_SIZE  64   /* 中心偏移数组的大小 */
static  int slc_departure[SLC_DEPARTURE_SIZE];
static  int slc_departure_nr=0;

int make_slc_departure(slc_descriptor_t * slc, INT16U flag)
{
    int k,l,i,v;
    order_kl_info_t t;
    order_kl_info_t * kl_data;

    kl_data = &slc->order_kl;
    t = *kl_data;

    k=l=i=v=0;

    for(k=0; k<slc->k_number; k++){
        if(slc->k_lmt_left[k] == slc->k_lmt_right[k]) {/* 不动 */
            for(v=0; v<t.k_pos_nr; v++){
                if(k==0 && t.k_pos[v] < slc->k_lmt_left[k]){ 
                    /* 最左边刀 */
                    slc_departure[i]= slc->k_lmt_left[k] - t.k_pos[k];
                    i++;
                }
                if(k>0 && t.k_pos[v] > (slc->k_lmt_left[k]-slc->k_distance[k-1])
                   && t.k_pos[v] < slc->k_lmt_left[k]){
                    /* 正好处于某刀的不可见区域(左) */
                    slc_departure[i]=(slc->k_lmt_left[k]-slc->k_distance[k-1])-t.k_pos[k];
                    i++;
                    slc_departure[i]= slc->k_lmt_left[k] - t.k_pos[k];
                    i++;
                }
                if(k==(slc->k_number-1) && t.k_pos[v] > slc->k_lmt_left[k]){
                    /* 最右边刀 */
                    slc_departure[i]= slc->k_lmt_left[k] - t.k_pos[k];
                    i++;
                }
                if(k<(slc->k_number-1) && t.k_pos[v] > slc->k_lmt_left[k]
                   && t.k_pos[v] < (slc->k_lmt_left[k]+slc->k_distance[k])){
                    /* 正好处于某刀的不可见区域(右) */
                    slc_departure[i]=(slc->k_lmt_left[k]+slc->k_distance[k])-t.k_pos[k];
                    i++;
                    slc_departure[i]= slc->k_lmt_left[k] - t.k_pos[k];
                    i++;
                }
                if(i >= SLC_DEPARTURE_SIZE-4)
                    break;
            }
        }
    }
    if(slc->slc_type == SLC_TYPE_SINGLE){ /* 单排型 */
        for(l=0; l<slc->l_number; l++){
            if(slc->l_lmt_left[l] == slc->l_lmt_right[l]) {/* 不动 */
                for(v=0; v<t.l_pos_nr; v++){
                    if(l==0 && t.l_pos[v] < slc->l_lmt_left[l]){
                        /* 最左边线 */
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;                    
                    }
                    if(l>=1 && t.l_pos[v] > (slc->l_lmt_left[l]-slc->l_distance[l-1])
                       && t.l_pos[v] < slc->l_lmt_left[l]){
                        /* 正好处于某线的不可见区域(左) */
                        slc_departure[i]=(slc->l_lmt_left[l]-slc->l_distance[l-1])-t.l_pos[l];
                        i++;
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;
                    }
                    if(l==(slc->l_number-1) && t.l_pos[v] >  slc->l_lmt_left[l]){
                        /* 最右边线 */
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;                    
                    }
                    if(l<=(slc->l_number-2) && t.l_pos[v] >  slc->l_lmt_left[l]
                       && t.l_pos[v] < (slc->l_lmt_left[l]+slc->l_distance[l])){
                        /* 正好处于某线的不可见区域(右) */
                        slc_departure[i]=(slc->l_lmt_left[l]+slc->l_distance[l])-t.l_pos[l];
                        i++;
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;
                    }
                    if(i >= SLC_DEPARTURE_SIZE-4)
                        break;
                }
            }
        }
    }
    if(slc->slc_type == SLC_TYPE_DOUBLE){ /* 双排型 */
        for(l=0; l<slc->l_number; l+=2){  /* 1,3,5,... */
            if(slc->l_lmt_left[l] == slc->l_lmt_right[l]) {/* 不动 */
                for(v=0; v<t.l_pos_nr; v++){
                    if(l==0 && t.l_pos[v] < slc->l_lmt_left[l]){
                        /* 最左边线 */
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;                    
                    }
                    if(l>=2 && t.l_pos[v] > (slc->l_lmt_left[l]-slc->l_distance[l-2])
                       && t.l_pos[v] < slc->l_lmt_left[l]){
                        /* 正好处于某线的不可见区域(左) */
                        slc_departure[i]=(slc->l_lmt_left[l]-slc->l_distance[l-2])-t.l_pos[l];
                        i++;
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;
                    }
                    if(l==(slc->l_number-2) && t.l_pos[v] >  slc->l_lmt_left[l]){
                        /* 最右边线 */
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;                    
                    }
                    if(l<(slc->l_number-2) && t.l_pos[v] >  slc->l_lmt_left[l]
                       && t.l_pos[v] < (slc->l_lmt_left[l]+slc->l_distance[l])){
                        /* 正好处于某线的不可见区域(右) */
                        slc_departure[i]=(slc->l_lmt_left[l]+slc->l_distance[l])-t.l_pos[l];
                        i++;
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;
                    }
                    if(i >= SLC_DEPARTURE_SIZE-4)
                        break;
                }
            }
        }
        if(__slc_is_prepress(flag)){ /* 预压时, 2,4,6对应为1,3,5 */
            goto out;
        }
        for(l=1; l<slc->l_number; l+=2){  /* 2,4,6,... */
            if(slc->l_lmt_left[l] == slc->l_lmt_right[l]) {/* 不动 */
                for(v=0; v<t.l_pos_nr; v++){
                    if(l==1 && t.l_pos[v] < slc->l_lmt_left[l]){
                        /* 最左边线 */
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;                    
                    }
                    if(l>=3 && t.l_pos[v] > (slc->l_lmt_left[l]-slc->l_distance[l-2])
                       && t.l_pos[v] < slc->l_lmt_left[l]){
                        /* 正好处于某线的不可见区域(左) */
                        slc_departure[i]=(slc->l_lmt_left[l]-slc->l_distance[l-2])-t.l_pos[l];
                        i++;
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;
                    }
                    if(l==(slc->l_number-1) && t.l_pos[v] >  slc->l_lmt_left[l]){
                        /* 最右边线 */
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;                    
                    }
                    if(l<(slc->l_number-1) && t.l_pos[v] >  slc->l_lmt_left[l]
                       && t.l_pos[v] < (slc->l_lmt_left[l]+slc->l_distance[l])){
                        /* 正好处于某线的不可见区域(右) */
                        slc_departure[i]=(slc->l_lmt_left[l]+slc->l_distance[l])-t.l_pos[l];
                        i++;
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;
                    }
                    if(i >= SLC_DEPARTURE_SIZE-4)
                        break;
                }
            }
        }
    }

out:    
    slc_departure_nr = i;

    return slc_departure_nr;
}

/*------------------------------------------------------------------------------------
 * 函数:    calc_k_number()/calc_l_number()
 *
 * 描述:    计算可用的刀/线数量
 *
 * 输入:    1) slc  分压机描述符
 *
 * 返回:    刀数/线数
**----------------------------------------------------------------------------------*/
int calc_k_number(slc_descriptor_t * slc)
{
    int i, v;
    for(i=0, v=0; i<slc->k_number; i++){
        if(!slc->k_disable[i])
            v++;
    }
    return v;
}

int calc_l_number(slc_descriptor_t * slc)
{
    int i, v;
    for(i=0, v=0; i<slc->l_number; i++){
        if(!slc->l_disable[i])
            v++;
    }
    return v;
}

/*------------------------------------------------------------------------------------
 * 函数:    slc_make_standard_location()
 *
 * 描述:    自动生成标准位置
 *
 * 输入:    1) slc       分压机描述符
 *          2) flag      排单选项(位图, 如: 是否修边等)
 *
 * 输出:    1) slc->kl_standard 刀线的标准位置
 *
 * 返回:    无
**----------------------------------------------------------------------------------*/
void slc_make_standard_location(slc_descriptor_t * slc, INT16U flag)
{
    int width;
    int k,l,i,j,v,w;

    flag = flag;

    width = slc->hw_width;
    k     = slc->k_number;
    l     = slc->l_number;

    width /= 10;
    
    v = ((width)/(k+1))*10;   
    w = (v)*(k+1)/2;
    for(i=0; i<k; i++){
        slc->kl_standard.k_location[i] = (v*(i+1))-w;
    }

    if(!__slc_is_prepress(flag)){
        v = ((width)/(l+1))*10;   
        w = (v)*(l+1)/2;
        for(i=0; i<l; i++){
            slc->kl_standard.l_location[i] = (v*(i+1))-w;
        }
    } else { /* 预压时 */
        int ll;
        ll = l/2;
        v = ((width)/(ll+1))*10;   
        w = (v)*(ll+1)/2;
        for(i=0,j=0; i<l; i+=2,j++){
            slc->kl_standard.l_location[i]   = (v*(j+1))-w;
            slc->kl_standard.l_location[i+1] = (v*(j+1))-w;
        }
    }
    #ifdef SLC_DEBUG
    printf("\n===== slc_make_standard_location () =====\n");
    printf("width=%d, k=%d, l=%d\n", width, k, l);
    for(i=0; i<k; i++){
            printf("[   K%d   ]", i);
    }
    printf("\n");
    printf(" ");
    for(i=0; i<k; i++){
        printf("  %+-5d   ", slc->kl_standard.k_location[i]);
    }
    printf("\n\n");
    for(i=0; i<l; i++){
            printf("[ L%-2d]", i);
    }
    printf("\n");
    for(i=0; i<l; i++){
        printf(" %+-5d", slc->kl_standard.l_location[i]);
    }
    if(SLC_PAUSE)getch();
    #endif
    
    return;
}

/*------------------------------------------------------------------------------------
 * 函数:    init_orderkl()
 *
 * 描述:    订单的刀线位置初始化
 *
 * 输入:    1) slc       分压机描述符, 主要是为了设置slc->order_kl
 *          2) data      压线资料(字符串, 如: 100+200+100*100+200+100)
 *          3) flag      排单选项(位图, 如: 是否修边等)
 *
 * 输出:    1) k_data    裁刀的订单位置
 *          2) k_data_nr k_data包含的数据个数
 *          3) l_data    压线的订单位置
 *          4) l_data_nr l_data包含的数据个数
 *
 * 返回:    >0 总裁宽
 *          -1 压线资料太长
 *          -2 刀线重合
 *          -3 数值太大
 *          -4 宽度不大于0
 *          -5 刀数不对
 *          -6 有非法字符
 *          -7 刀数太多
 *          -8 线数太多
 *          -9 压线资料太短
**----------------------------------------------------------------------------------*/
int init_orderkl(slc_descriptor_t * slc, 
                 char data[], 
                 int  **k_data, int *k_data_nr, 
                 int  **l_data, int *l_data_nr,
                 INT16U flag)
{
    int width;
    int i,v,k,l,loop;

    i=v=k=l=0;
    width = 0;

    if(STRLEN(data) < 3)
        return -9;

    slc->order_kl.k_pos[0]=0;
    k++;

 xxx_debug32("init_orderkl", 0);
 xxx_debug32(data, data);
 
    loop = 1;
    while(loop){
        switch(data[i]){
            case SLC_K_TOKEN:
                if(v==0)
                    return -2;
                if(k >= SLC_K_MAX_NR){
                    return -7;
                }
                width += (v*10);
                slc->order_kl.k_pos[k] = width;
                k++;
                v = 0;
                break;
            case SLC_L_TOKEN:
                if(v==0)
                    return -2;
                if(l >= SLC_L_MAX_NR){
                    return -8;
                }
                width += (v*10);
                slc->order_kl.l_pos[l] = width;
                l++;
                v = 0;
                break;
            case ' ':
                break;
            case '\t':
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if(v > 3000)
                    return -3;
                v *= 10;
                v += (data[i]-'0');
                break;
            case '\0': /* 结束 */
                if(v==0)
                    return -2;
                if(k >= SLC_K_MAX_NR){
                    return -7;
                }
                width += (v*10);
                slc->order_kl.k_pos[k] = width;
                k++;
                v = 0;
                loop = 0;
                break;
            default: /* 错误字符 */
                return -6;
        };
        i++;
        if(i > SLC_DATA_LEN)
            return -1; /* 压线资料太长了 */
    }
    if(width <= 0)
        return -4;

    FamesAssert(k >= 2);
    if(k < 2)
        return -5;

    v = width/2; /* width是10的倍数, 即肯定是个偶数 */
    for(i=0; i<k; i++){ /* 刀平衡 */
        slc->order_kl.k_pos[i]-=v;
    }
    for(i=0; i<l; i++){ /* 线平衡 */
        slc->order_kl.l_pos[i]-=v;    
    }

    if(!(flag & SLC_FLAG_TRIM)){ /* 不需修边 */
        for(i=1; i<k; i++){
            slc->order_kl.k_pos[i-1] = slc->order_kl.k_pos[i];    
        }
        k -= 2; /* 修边刀共有2个 */
        slc->order_kl.k_pos[k] = 0;
    }

    slc->order_kl.k_pos_nr = k;
    slc->order_kl.l_pos_nr = l;
    slc->order_kl.width    = width;

    *k_data = slc->order_kl.k_pos;
    *l_data = slc->order_kl.l_pos;
    *k_data_nr = k;
    *l_data_nr = l;

    #ifdef SLC_DEBUG    
        printf("===== init_orderkl() =====\n");
        printf("k=%d, l=%d\n", k, l);
        for(i=0; i<k; i++)
            printf("%-5d ", (*k_data)[i]);
        printf("\n");
        for(i=0; i<l; i++)
            printf("%-5d ", (*l_data)[i]);
        printf("\n");
    #endif

    return width;
}

/*------------------------------------------------------------------------------------
 * 函数:    slc_orderkl_relocate()
 *
 * 描述:    订单刀线位置的重定位
 *
 * 输入:    1) dst        重定位目标
 *          2) slc        SLC
 *          3) departure  偏差
 *
 * 返回:    无
**----------------------------------------------------------------------------------*/
void slc_orderkl_relocate(order_kl_info_t * dst, slc_descriptor_t * slc, int departure)
{
    int k, l, i;
    int *kd, *ld;

xxx_debug32("before slc_orderkl_relocate", 0L);

    kd = slc->order_kl.k_pos;
    ld = slc->order_kl.l_pos;

    k = slc->order_kl.k_pos_nr;
    l = slc->order_kl.l_pos_nr;

    for(i=0; i<k; i++)
        dst->k_pos[i] = (kd[i]+departure);

    for(i=0; i<l; i++)
        dst->l_pos[i] = (ld[i]+departure);

    dst->k_pos_nr = k;
    dst->l_pos_nr = l;
    dst->width = slc->order_kl.width;

xxx_debug32("after slc_orderkl_relocate", 0L);

    return;
}

/*------------------------------------------------------------------------------------
 * 函数:    slc_locate_error()
 *
 * 描述:    检查刀线定位错误
 *
 * 输入:    1) slc        SLC
 *          2) locate     定位数据
 *          3) flag       选项
 *
 * 返回:    错误码
**----------------------------------------------------------------------------------*/
INT32U slc_locate_error(slc_descriptor_t * slc, slc_locate_t * locate, INT16U flag)
{
    INT32U error;
    int i, k, l;

    k = slc->k_number;
    l = slc->l_number;

knife:
    error = SLC_ERR_K_LIMIT;    /* 是否有机械限位 */
    for(i=0; i<k; i++){
        if(locate->k_location[i] < slc->k_lmt_left[i] ||
           locate->k_location[i] > slc->k_lmt_right[i]){
            error |= (INT32U)(unsigned)i;
            return error;
        }
    }
    error = SLC_ERR_K_DISABLED; /* 是否选中了不可用刀线 */
    for(i=0; i<k; i++){
        if(locate->k_selected[i] && slc->k_disable[i]){
            error |= (INT32U)(unsigned)i;
            return error;
        }
    }
    error = SLC_ERR_KK_LMT;   /* 是否有间距限位 */
    for(i=1; i<k; i++){
        if((locate->k_location[i]-locate->k_location[i-1])<slc->k_distance[i-1]){
            error |= (INT32U)(unsigned)i;
            return error;
        }
    }

single_line:
    if(slc->slc_type == SLC_TYPE_SINGLE){
        error = SLC_ERR_L_LIMIT;
        for(i=0; i<l; i++){            
            if(locate->l_location[i] < slc->l_lmt_left[i] ||
               locate->l_location[i] > slc->l_lmt_right[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_L_DISABLED;
        for(i=0; i<l; i++){
            if(locate->l_selected[i] && slc->l_disable[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_LL_LMT;
        for(i=1; i<l; i++){
            if((locate->l_location[i]-locate->l_location[i-1])<slc->l_distance[i-1]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
    }
    
double_line:
    if(slc->slc_type == SLC_TYPE_DOUBLE){
        error = SLC_ERR_L_LIMIT;
        for(i=0; i<l; i++){
            if(__slc_is_prepress(flag)){
                if(i&1) /* 预压时, 不检查后排线 */
                    continue;
            }
            if(locate->l_location[i] < slc->l_lmt_left[i] ||
               locate->l_location[i] > slc->l_lmt_right[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_L_DISABLED;
        for(i=0; i<l; i++){
            if(__slc_is_prepress(flag)){
                if(i&1) /* 预压时, 不检查后排线 */
                    continue;
            }
            if(locate->l_selected[i] && slc->l_disable[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_LL_LMT;
        for(i=2; i<l; i++){
            if(__slc_is_prepress(flag)){
                if(i&1) /* 预压时, 不检查后排线 */
                    continue;
            }
            if((locate->l_location[i]-locate->l_location[i-2])<slc->l_distance[i-2]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
    } 

    return SLC_ERR_NONE;
}

/*------------------------------------------------------------------------------------
 * 函数:    slc_adjust_k()/slc_adjust_l()
 *
 * 描述:    刀线位置调整
 *
 * 输入:    1) slc        SLC
 *          2) locate     定位数据
 *          3) flag       选项
 *
 * 返回:    错误码
**----------------------------------------------------------------------------------*/
INT32U slc_adjust_k(slc_descriptor_t * slc, slc_locate_t * locate, INT16U flag)
{
    INT32U error;
    int i, j, N, t;
    int *sel, *loc;

    N = slc->k_number;

    flag = flag; /* 目前不用 */

    #ifdef SLC_DEBUG    
        printf("\n===== slc_adjust_k() start =====\n");
        printf("N=%d\n", N);
        for(i=0; i<N; i++){
            if(locate->k_selected[i])
                printf("[ %d  ]", i);
            else 
                printf("------");
        }
        printf("\n");
        for(i=0; i<N; i++)
            printf("%5d ", locate->k_location[i]);
        printf("\n");
    #endif

    sel = locate->k_selected;
    loc = locate->k_location;

    t = (N/2); /* 大约中间的那把刀 */

    for(i=0; i<N; i++){
        if(sel[i])
            continue;
        loc[i] = slc->kl_standard.k_location[i]; /* 设为标准位置 */
        if(loc[i] < slc->k_lmt_left[i])
            loc[i] = slc->k_lmt_left[i];
        if(loc[i] > slc->k_lmt_right[i])
            loc[i] = slc->k_lmt_right[i];
    }
    if(N&1){ /* 刀数为奇数时 */
        if(!sel[t]){ /* 先看中刀 */
            loc[t] = 0;    
            if(sel[t-1] && (loc[t]-loc[t-1]) < slc->k_distance[t-1])
                loc[t] = loc[t-1] + slc->k_distance[t-1];
            if(sel[t+1] && (loc[t+1]-loc[t]) < slc->k_distance[t])
                loc[t] = loc[t+1] - slc->k_distance[t];
        }
        for(i=t-1, j=t+1; i>=0; i--, j++){ /* 从中间到两边 */
            if(!sel[i]){ /* 左边 */
                if((loc[i]+loc[(N-1)-i])>0){ /* 尽量使两边平衡(趋向分散) */
                    loc[i] = -loc[(N-1)-i];
                }
                if((loc[i+1]-loc[i]) < slc->k_distance[i])
                    loc[i] = loc[i+1] - slc->k_distance[i];
                if(i>0 && sel[i-1] && (loc[i]-loc[i-1]) < slc->k_distance[i-1])
                    loc[i] = loc[i-1] + slc->k_distance[i-1];
            }
            if(!sel[j]){ /* 右边 */
                if((loc[j]+loc[(N-1)-j])<0){ /* 尽量使两边平衡(趋向分散) */
                    loc[j] = -loc[(N-1)-j];
                }
                if((loc[j]-loc[j-1]) < slc->k_distance[j-1])
                    loc[j] = loc[j-1] + slc->k_distance[j-1];
                if(j<N-1 && sel[j+1] && (loc[j+1]-loc[j]) < slc->k_distance[j])
                    loc[j] = loc[j+1] - slc->k_distance[j];
            }
        }
    } else { /* 刀数为偶数时 */
        for(i=t-1, j=t; i>=0; i--, j++){ /* 从中间到两边 */
            if(!sel[i]){ /* 左边 */
                if((loc[i]+loc[(N-1)-i])>0){ /* 尽量使两边平衡(趋向分散) */
                    loc[i] = -loc[(N-1)-i];
                }
                if((loc[i+1]-loc[i]) < slc->k_distance[i])
                    loc[i] = loc[i+1] - slc->k_distance[i];
                if(i>0 && sel[i-1] && (loc[i]-loc[i-1]) < slc->k_distance[i-1])
                    loc[i] = loc[i-1] + slc->k_distance[i-1];
            }
            if(!sel[j]){ /* 右边 */
                if((loc[j]+loc[(N-1)-j])<0){ /* 尽量使两边平衡(趋向分散) */
                    loc[j] = -loc[(N-1)-j];
                }
                if((loc[j]-loc[j-1]) < slc->k_distance[j-1])
                    loc[j] = loc[j-1] + slc->k_distance[j-1];
                if(j<N-1 && sel[j+1] && (loc[j+1]-loc[j]) < slc->k_distance[j])
                    loc[j] = loc[j+1] - slc->k_distance[j];
            }
        }
    }
    for(i=0; i<N; i++){ /* 检查机械限位 */
        if(sel[i])
            continue;
        if(loc[i] < slc->k_lmt_left[i])
            loc[i] = slc->k_lmt_left[i];
        if(loc[i] > slc->k_lmt_right[i])
            loc[i] = slc->k_lmt_right[i];
    }

    #ifdef SLC_DEBUG    
        printf("===== slc_adjust_k() ended =====\n");
        printf("N=%d\n", N);
        for(i=0; i<N; i++){
            if(locate->k_selected[i])
                printf("[ %d  ]", i);
            else 
                printf("------");
        }
        printf("\n");
        for(i=0; i<N; i++)
            printf("%5d ", locate->k_location[i]);
        printf("\n");
    #endif

    error = SLC_ERR_K_LIMIT;    /* 是否有机械限位 */
    for(i=0; i<N; i++){
        if(loc[i] < slc->k_lmt_left[i] ||
           loc[i] > slc->k_lmt_right[i]){
            error |= (INT32U)(unsigned)i;
            return error;
        }
    }
    error = SLC_ERR_KK_LMT;     /* 是否有间距限位 */
    for(i=1; i<N; i++){
        if((loc[i]-loc[i-1])<slc->k_distance[i-1]){
            error |= (INT32U)(unsigned)i;
            return error;
        }
    }
    error = SLC_ERR_K_DISABLED; /* 是否选中了不可用刀线 */
    for(i=0; i<N; i++){
        if(sel[i] && slc->k_disable[i]){
            error |= (INT32U)(unsigned)i;
            return error;
        }
    }

    return SLC_ERR_NONE;
}

INT32U slc_adjust_l(slc_descriptor_t * slc, slc_locate_t * locate, INT16U flag)
{
    INT32U error;
    int i, j, N, t;
    int *sel, *loc;

    N = slc->l_number;
    
    #ifdef SLC_DEBUG    
        printf("\n===== slc_adjust_l() start =====\n");
        printf("N=%d\n", N);
        for(i=0; i<N; i++){
            if(locate->l_selected[i])
                printf("[ %d  ]", i);
            else 
                printf("------");
        }
        printf("\n");
        for(i=0; i<N; i++)
            printf("%5d ", locate->l_location[i]);
        printf("\n");
    #endif
    
    sel = locate->l_selected;
    loc = locate->l_location;

    t = (N/2); /* 大约中间的那把线(偏右) */

single: /* ======== 单排线时 ======== */   
    if(slc->slc_type == SLC_TYPE_SINGLE){
        for(i=0; i<N; i++){
            if(sel[i])
                continue;
            loc[i] = slc->kl_standard.l_location[i]; /* 设为标准位置 */
            if(loc[i] < slc->l_lmt_left[i])
                loc[i] = slc->l_lmt_left[i];
            if(loc[i] > slc->l_lmt_right[i])
                loc[i] = slc->l_lmt_right[i];
        }
        for(i=t-1, j=t; i>=0; i--, j++){ /* 从中间到两边 */
            if(!sel[i]){ /* 左边 */
                if((loc[i]+loc[(N-1)-i])>0){ /* 尽量使两边平衡(趋向分散) */
                    loc[i] = -loc[(N-1)-i];
                }
                if((loc[i+1]-loc[i]) < slc->l_distance[i])
                    loc[i] = loc[i+1] - slc->l_distance[i];
                if(i>0 && sel[i-1] && (loc[i]-loc[i-1]) < slc->l_distance[i-1])
                    loc[i] = loc[i-1] + slc->l_distance[i-1];
            }
            if(!sel[j]){ /* 右边 */
                if((loc[j]+loc[(N-1)-j])<0){ /* 尽量使两边平衡(趋向分散) */
                    loc[j] = -loc[(N-1)-j];
                }
                if((loc[j]-loc[j-1]) < slc->l_distance[j-1])
                    loc[j] = loc[j-1] + slc->l_distance[j-1];
                if(j<N-1 && sel[j+1] && (loc[j+1]-loc[j]) < slc->l_distance[j])
                    loc[j] = loc[j+1] - slc->l_distance[j];
            }
        }
        for(i=0; i<N; i++){ /* 检查机械限位 */
            if(sel[i])
                continue;
            if(loc[i] < slc->l_lmt_left[i])
                loc[i] = slc->l_lmt_left[i];
            if(loc[i] > slc->l_lmt_right[i])
                loc[i] = slc->l_lmt_right[i];
        }        
        #ifdef SLC_DEBUG
            printf("===== slc_adjust_l() ended, single =====\n");
            printf("N=%d\n", N);
            for(i=0; i<N; i++){
                if(locate->l_selected[i])
                    printf("[ %d  ]", i);
                else 
                    printf("------");
            }
            printf("\n");
            for(i=0; i<N; i++)
                printf("%5d ", locate->l_location[i]);
            printf("\n");
        #endif
        error = SLC_ERR_L_LIMIT;    /* 是否有机械限位 */
        for(i=0; i<N; i++){
            if(loc[i] < slc->l_lmt_left[i] ||
               loc[i] > slc->l_lmt_right[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_LL_LMT;     /* 是否有间距限位 */
        for(i=1; i<N; i++){
            if((loc[i]-loc[i-1])<slc->l_distance[i-1]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_L_DISABLED; /* 是否选中了不可用刀线 */
        for(i=0; i<N; i++){
            if(sel[i] && slc->l_disable[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
    }
    
double_normal: /* ======== 双排线, 无预压时 ======== */   
    if((slc->slc_type == SLC_TYPE_DOUBLE) && (!__slc_is_prepress(flag))){
        for(i=0; i<N; i++){
            if(sel[i])
                continue;
            loc[i] = slc->kl_standard.l_location[i]; /* 设为标准位置 */
            if(loc[i] < slc->l_lmt_left[i])
                loc[i] = slc->l_lmt_left[i];
            if(loc[i] > slc->l_lmt_right[i])
                loc[i] = slc->l_lmt_right[i];
        }
        #if 1  /* 两排线整体平衡, 这有可能导致本排线不平衡 */
        for(i=t-1, j=t; i>=0; i--, j++){ /* 从中间到两边 */
            if(!sel[i]){ /* 左边 */
                if((loc[i]+loc[(N-1)-i])>0){ /* 尽量使两边平衡(趋向分散) */
                    loc[i] = -loc[(N-1)-i];
                }
                if((loc[i+1]-loc[i]) <= 0) /* 尽量按线序排列, 不要交叉 */
                    loc[i] = loc[i+1] - (slc->l_distance[i])/2;
                if(i>0 && sel[i-1] && (loc[i]-loc[i-1]) <= 0)
                    loc[i] = loc[i-1] + (slc->l_distance[i-1]/2);
                if((loc[i+2]-loc[i]) < slc->l_distance[i]) /* 检查线线间隙 */
                    loc[i] = loc[i+2] - slc->l_distance[i];
                if(i>1 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                    loc[i] = loc[i-2] + slc->l_distance[i-2];
            }
            if(!sel[j]){ /* 右边 */
                if((loc[j]+loc[(N-1)-j])<0){ /* 尽量使两边平衡(趋向分散) */
                    loc[j] = -loc[(N-1)-j];
                }
                if((loc[j]-loc[j-1]) <= 0) /* 尽量按线序排列, 不要交叉 */
                    loc[j] = loc[j-1] + (slc->l_distance[j-1])/2;
                if(j<N-1 && sel[j+1] && (loc[j+1]-loc[j]) <= 0)
                    loc[j] = loc[j+1] - (slc->l_distance[j])/2;
                if((loc[j]-loc[j-2]) < slc->l_distance[j-2]) /* 检查线线间隙 */
                    loc[j] = loc[j-2] + slc->l_distance[j-2];
                if(j<N-2 && sel[j+2] && (loc[j+2]-loc[j]) < slc->l_distance[j])
                    loc[j] = loc[j+2] - slc->l_distance[j];
            }
        }
        #else /* 一排线只与本排线平衡, 与另一排线不相联系 */
        /**
         * t=N/2, 这样的话, (t)就是右边部分的第一线, 同时也是左边部分的线数, 那么:
         * 
         * 如果t是奇数, 那么左边部分前排线比后排线多1, 
         *                  右边部分前排线比后排线少1, 这种情况下, 有:
         *
         *       (t-1)是前排线中线,
         *       (t-3)是前排线左一,
         *       (t+1)是前排线右一,
         *       (t)  是后排线中线,
         *       (t-2)是后排线左一,
         *       (t+2)是后排线右一;
         *
         * 如果t是偶数, 那么左边部分前排线与后排线相同, 
         *                  右边部分前排线与后排线也相同, 这种情况下, 有:
         *       
         *       (t-2)是前排线左一,
         *       (t)  是前排线右一,
         *       (t-1)是后排线左一,
         *       (t+1)是后排线右一, 前后排均没有中线;
         *
         * 其中的N是总线数(本函数内部变量N=slc->l_number)
         *
         * 以上结论的前提: 1) 总线数总是偶数, 2) 总线数不小于4
         */
        if(t&1){ /* 如果t是奇数... */
            if(!sel[t-1]){ /* 前排中线 */
                i = t-1;
                loc[i] = 0;    
                if(sel[i+2] && (loc[i+2]-loc[i]) < slc->l_distance[i])
                    loc[i] = loc[i+2] - slc->l_distance[i];
                if(sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                    loc[i] = loc[i-2] + slc->l_distance[i-2];
            }
            for(i=t-3, j=t+1; i>=0; i-=2, j+=2){ /* 前排左右线 */
                if(!sel[i]){ /* 左边 */
                    if((loc[i]+loc[(N-2)-i])>0){ /* 尽量使两边平衡(趋向分散) */
                        loc[i] = -loc[(N-2)-i];
                    }
                    if((loc[i+2]-loc[i]) < slc->l_distance[i])
                        loc[i] = loc[i+2] - slc->l_distance[i];
                    if(i>1 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                        loc[i] = loc[i-2] + slc->l_distance[i-2];
                }
                if(!sel[j]){ /* 右边 */
                    if((loc[j]+loc[(N-2)-j])<0){ /* 尽量使两边平衡(趋向分散) */
                        loc[j] = -loc[(N-2)-j];
                    }
                    if((loc[j]-loc[j-2]) < slc->l_distance[j-2])
                        loc[j] = loc[j-2] + slc->l_distance[j-2];
                    if(j<N-2 && sel[j+2] && (loc[j+2]-loc[j]) < slc->l_distance[j])
                        loc[j] = loc[j+2] - slc->l_distance[j];
                }
            }
            if(!sel[t]){ /* 后排中线 */
                i = t;
                loc[i] = 0;    
                if(sel[i+2] && (loc[i+2]-loc[i]) < slc->l_distance[i])
                    loc[i] = loc[i+2] - slc->l_distance[i];
                if(sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                    loc[i] = loc[i-2] + slc->l_distance[i-2];
            }
            for(i=t-2, j=t+2; i>=1; i-=2, j+=2){ /* 后排左右线 */
                if(!sel[i]){ /* 左边 */
                    if((loc[i]+loc[(N-2)-i])>0){ /* 尽量使两边平衡(趋向分散) */
                        loc[i] = -loc[(N-2)-i];
                    }
                    if((loc[i+2]-loc[i]) < slc->l_distance[i])
                        loc[i] = loc[i+2] - slc->l_distance[i];
                    if(i>2 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                        loc[i] = loc[i-2] + slc->l_distance[i-2];
                }
                if(!sel[j]){ /* 右边 */
                    if((loc[j]+loc[(N-2)-j])<0){ /* 尽量使两边平衡(趋向分散) */
                        loc[j] = -loc[(N-2)-j];
                    }
                    if((loc[j]-loc[j-2]) < slc->l_distance[j-2])
                        loc[j] = loc[j-2] + slc->l_distance[j-2];
                    if(j<N-1 && sel[j+2] && (loc[j+2]-loc[j]) < slc->l_distance[j])
                        loc[j] = loc[j+2] - slc->l_distance[j];
                }
            }
        } else { /* 如果t是偶数...前后排均没有中线... */
            for(i=t-2, j=t; i>=0; i-=2, j+=2){ /* 前排左右线 */
                if(!sel[i]){ /* 左边 */
                    if((loc[i]+loc[(N-2)-i])>0){ /* 尽量使两边平衡(趋向分散) */
                        loc[i] = -loc[(N-2)-i];
                    }
                    if((loc[i+2]-loc[i]) < slc->l_distance[i])
                        loc[i] = loc[i+2] - slc->l_distance[i];
                    if(i>1 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                        loc[i] = loc[i-2] + slc->l_distance[i-2];
                }
                if(!sel[j]){ /* 右边 */
                    if((loc[j]+loc[(N-2)-j])<0){ /* 尽量使两边平衡(趋向分散) */
                        loc[j] = -loc[(N-2)-j];
                    }
                    if((loc[j]-loc[j-2]) < slc->l_distance[j-2])
                        loc[j] = loc[j-2] + slc->l_distance[j-2];
                    if(j<N-2 && sel[j+2] && (loc[j+2]-loc[j]) < slc->l_distance[j])
                        loc[j] = loc[j+2] - slc->l_distance[j];
                }
            }
            for(i=t-1, j=t+1; i>=1; i-=2, j+=2){ /* 后排左右线 */
                if(!sel[i]){ /* 左边 */
                    if((loc[i]+loc[(N-2)-i])>0){ /* 尽量使两边平衡(趋向分散) */
                        loc[i] = -loc[(N-2)-i];
                    }
                    if((loc[i+2]-loc[i]) < slc->l_distance[i])
                        loc[i] = loc[i+2] - slc->l_distance[i];
                    if(i>2 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                        loc[i] = loc[i-2] + slc->l_distance[i-2];
                }
                if(!sel[j]){ /* 右边 */
                    if((loc[j]+loc[(N-2)-j])<0){ /* 尽量使两边平衡(趋向分散) */
                        loc[j] = -loc[(N-2)-j];
                    }
                    if((loc[j]-loc[j-2]) < slc->l_distance[j-2])
                        loc[j] = loc[j-2] + slc->l_distance[j-2];
                    if(j<N-1 && sel[j+2] && (loc[j+2]-loc[j]) < slc->l_distance[j])
                        loc[j] = loc[j+2] - slc->l_distance[j];
                }
            }
        }
        #endif
        for(i=0; i<N; i++){ /* 检查机械限位 */
            if(sel[i])
                continue;
            if(loc[i] < slc->l_lmt_left[i])
                loc[i] = slc->l_lmt_left[i];
            if(loc[i] > slc->l_lmt_right[i])
                loc[i] = slc->l_lmt_right[i];
        }
        #ifdef SLC_DEBUG
            printf("===== slc_adjust_l() ended, double-normal =====\n");
            printf("N=%d\n", N);
            for(i=0; i<N; i++){
                if(locate->l_selected[i])
                    printf("[ %d  ]", i);
                else 
                    printf("------");
            }
            printf("\n");
            for(i=0; i<N; i++)
                printf("%5d ", locate->l_location[i]);
            printf("\n");
        #endif
        error = SLC_ERR_L_LIMIT;    /* 是否有机械限位 */
        for(i=0; i<N; i++){
            if(loc[i] < slc->l_lmt_left[i] ||
               loc[i] > slc->l_lmt_right[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_LL_LMT;     /* 是否有间距限位 */
        for(i=2; i<N; i++){
            if((loc[i]-loc[i-2])<slc->l_distance[i-2]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_L_DISABLED; /* 是否选中了不可用刀线 */
        for(i=0; i<N; i++){
            if(sel[i] && slc->l_disable[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
    }

double_prepress: /* ======== 双排线, 有预压时 ======== */   
    if((slc->slc_type == SLC_TYPE_DOUBLE) && (__slc_is_prepress(flag))){
        for(i=0; i<N; i+=2){
            if(sel[i])
                continue;
            loc[i] = slc->kl_standard.l_location[i]; /* 设为标准位置 */
            if(loc[i] < slc->l_lmt_left[i])
                loc[i] = slc->l_lmt_left[i];
            if(loc[i] > slc->l_lmt_right[i])
                loc[i] = slc->l_lmt_right[i];
        }
        if(t&1){ /* 如果t是奇数... */
            if(!sel[t-1]){ /* 前排中线 */
                i = t-1;
                loc[i] = 0;    
                if(sel[i+2] && (loc[i+2]-loc[i]) < slc->l_distance[i])
                    loc[i] = loc[i+2] - slc->l_distance[i];
                if(sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                    loc[i] = loc[i-2] + slc->l_distance[i-2];
            }
            for(i=t-3, j=t+1; i>=0; i-=2, j+=2){ /* 前排左右线 */
                if(!sel[i]){ /* 左边 */
                    if((loc[i]+loc[(N-2)-i])>0){ /* 尽量使两边平衡(趋向分散) */
                        loc[i] = -loc[(N-2)-i];
                    }
                    if((loc[i+2]-loc[i]) < slc->l_distance[i])
                        loc[i] = loc[i+2] - slc->l_distance[i];
                    if(i>1 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                        loc[i] = loc[i-2] + slc->l_distance[i-2];
                }
                if(!sel[j]){ /* 右边 */
                    if((loc[j]+loc[(N-2)-j])<0){ /* 尽量使两边平衡(趋向分散) */
                        loc[j] = -loc[(N-2)-j];
                    }
                    if((loc[j]-loc[j-2]) < slc->l_distance[j-2])
                        loc[j] = loc[j-2] + slc->l_distance[j-2];
                    if(j<N-2 && sel[j+2] && (loc[j+2]-loc[j]) < slc->l_distance[j])
                        loc[j] = loc[j+2] - slc->l_distance[j];
                }
            }
        } else { /* 如果t是偶数...前后排均没有中线... */
            for(i=t-2, j=t; i>=0; i-=2, j+=2){ /* 前排左右线 */
                if(!sel[i]){ /* 左边 */
                    if((loc[i]+loc[(N-2)-i])>0){ /* 尽量使两边平衡(趋向分散) */
                        loc[i] = -loc[(N-2)-i];
                    }
                    if((loc[i+2]-loc[i]) < slc->l_distance[i])
                        loc[i] = loc[i+2] - slc->l_distance[i];
                    if(i>1 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                        loc[i] = loc[i-2] + slc->l_distance[i-2];
                }
                if(!sel[j]){ /* 右边 */
                    if((loc[j]+loc[(N-2)-j])<0){ /* 尽量使两边平衡(趋向分散) */
                        loc[j] = -loc[(N-2)-j];
                    }
                    if((loc[j]-loc[j-2]) < slc->l_distance[j-2])
                        loc[j] = loc[j-2] + slc->l_distance[j-2];
                    if(j<N-2 && sel[j+2] && (loc[j+2]-loc[j]) < slc->l_distance[j])
                        loc[j] = loc[j+2] - slc->l_distance[j];
                }
            }
        }
        for(i=0; i<N; i+=2){ /* 检查机械限位 */
            if(sel[i])
                continue;
            if(loc[i] < slc->l_lmt_left[i])
                loc[i] = slc->l_lmt_left[i];
            if(loc[i] > slc->l_lmt_right[i])
                loc[i] = slc->l_lmt_right[i];
        }
        for(i=1; i<N; i+=2){ /* !!!后排线映射为前排线!!!, 这就是预压! */
            loc[i] = loc[i-1];
            sel[i] = sel[i-1];
        }            
        #ifdef SLC_DEBUG
            printf("===== slc_adjust_l() ended, double-prepress =====\n");
            printf("N=%d\n", N);
            for(i=0; i<N; i++){
                if(locate->l_selected[i])
                    printf("[ %d  ]", i);
                else 
                    printf("------");
            }
            printf("\n");
            for(i=0; i<N; i++)
                printf("%5d ", locate->l_location[i]);
            printf("\n");
        #endif        
        error = SLC_ERR_L_LIMIT;    /* 是否有机械限位 */
        for(i=0; i<N; i+=2){
            if(loc[i] < slc->l_lmt_left[i] ||
               loc[i] > slc->l_lmt_right[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_LL_LMT;     /* 是否有间距限位 */
        for(i=2; i<N; i+=2){
            if((loc[i]-loc[i-2])<slc->l_distance[i-2]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_L_DISABLED; /* 是否选中了不可用刀线 */
        for(i=0; i<N; i+=2){
            if(sel[i] && slc->l_disable[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
    }
    
    return SLC_ERR_NONE;
}


/*------------------------------------------------------------------------------------
 *
 *                  刀线分配树
 *
**----------------------------------------------------------------------------------*/
struct __kl_tree {
    int kl_no;
    struct __kl_tree * next;
};
struct __kl_tree_head {
    struct __kl_tree * saved;
    struct __kl_tree ** curr;
};

#define NR_KL_TREE_NODE()  ((SLC_K_MAX_NR*SLC_K_MAX_NR) + \
                            (SLC_L_MAX_NR*SLC_L_MAX_NR))

static struct __kl_tree * kl_tree_pool = NULL; /* 必须初始化为NULL */
static int                kl_tree_curr = 0;

static struct __kl_tree_head kl_tree_k[SLC_K_MAX_NR];
static struct __kl_tree_head kl_tree_l[SLC_L_MAX_NR];

#define INIT_kl_tree_head(kltreeh) \
            do { \
                MEMSET((INT08S *)kltreeh, 0, sizeof(kltreeh)); \
            } while(0)

#define INIT_kl_tree(kltree) \
            do { \
                (kltree).kl_no = -1; \
                (kltree).next  = NULL; \
            } while(0)

#define INIT_kl_tree_pool(klpool) \
            do { \
                int __i; \
                for(__i=0; __i<NR_KL_TREE_NODE(); __i++){ \
                    INIT_kl_tree(klpool[__i]); \
                } \
            } while(0)
            
/*------------------------------------------------------------------------------------
 *
 *                  平衡度
 * 
 * __cmp_balance()函数的比较方式相当关键, 其基本上就是排单方案的决策者
 *
 * 返回值:  <0   score1比score2差
 *          =0   相同
 *          >0   score1比score2好
 *
 * 目前的比较原则为: 平衡度优先
**----------------------------------------------------------------------------------*/
struct __kl_balance {
    long value;   /* 平衡度   */
    int  close;   /* 就近度   */
};

int __cmp_balance(struct __kl_balance * score1, struct __kl_balance * score2)
{
    if(score1->value < score2->value)
        return 1;
    if(score1->value > score2->value)
        return -1;
    if(score1->close < score2->close)
        return 1;
    if(score1->close > score2->close)
        return -1;
    return 0;
}

void slc_make_score_k(slc_descriptor_t * slc, slc_locate_t * locate, 
                    struct __kl_balance * score, INT16U flag)
{
    int i, j, k, close_v;
    long v, t;

    k = slc->k_number;

    flag = flag; /* 目前不用 */

    v = 0L;
    if(k&1) {/* K为奇数时 */
        j = k/2;
        for(i=0; i<j; i++){
            t = (long)locate->k_location[(k-1)-i]+locate->k_location[i];
            if(t<0L)
                t=-t;
            t *= (((long)k-i)-i);  /* 边上刀线的平衡度更重要??? 个人意见, 呵~ */
            v += t;
        }
        t = (long)locate->k_location[j];
        if(t<0L)
            t=-t;
        v += t; /* 加上中刀的偏差 */
    } else {/* K为偶数时 */
        j = k/2;
        for(i=0; i<j; i++){
            t = (long)locate->k_location[(k-1)-i]+locate->k_location[i];
            if(t<0)
                t=-t;
            t *= (((long)k-i)-i); 
            v += t;
        }
    }
    score->value = v; /* 刀平衡度! */

    close_v = 0;
    for(i=0; i<k; i++){
        j = locate->k_location[i] - slc->kl_act.k_location[i];
        if(j<0)
            j=-j;
        if(close_v < j)
            close_v = j;
    }
    score->close = close_v; /* 刀就近度 */
    
    #ifdef SLC_DEBUG
    printf("slc_make_score_k(): value=%ld, close=%d\n", score->value, score->close);
    #endif

    return;
}

void slc_make_score_l(slc_descriptor_t * slc, slc_locate_t * locate, 
                    struct __kl_balance * score, INT16U flag)
{
    int i, j, l, close_v;
    long v, t;

    l = slc->l_number;

    v = 0L;
    j = l/2;

    if(slc->slc_type == SLC_TYPE_SINGLE){
        for(i=0; i<j; i++){
            t = (long)locate->l_location[(l-1)-i]+locate->l_location[i];
            if(t<0)
                t=-t;
            t *= (((long)l-i)-i);
            v += t;
        }
    }
    #if 1 /* 两排线总体平衡 */
    if(slc->slc_type == SLC_TYPE_DOUBLE){
        if(!__slc_is_prepress(flag)){
            for(i=0; i<j; i++){
                t = (long)locate->l_location[(l-1)-i]+locate->l_location[i];
                if(t<0)
                    t=-t;
                t *= (((long)l-i)-i);
                v += t;
            }
            for(i=1; i<l; i++){ /* 检查线序反转情况 */
                if(locate->l_location[i] <= locate->l_location[i-1])
                    v += 0x01000000L;
            }
        } else { /* 预压时 */
            for(i=0; i<j; i+=2){
                if(i==((l-2)-i)){ /* 中线 */
                    t = (long)locate->l_location[i];
                } else {
                    t = (long)locate->l_location[(l-2)-i]+locate->l_location[i];
                }
                if(t<0)
                    t=-t;
                t *= ((((long)l-i)-i)+1);
                v += t;
            }
        }
    }
    #else /* 一排线只与本排线平衡, 与另一排线不相联系 */
    if(slc->slc_type == SLC_TYPE_DOUBLE){
        /**
         * j是右边部分的第一线, 也是左边部分的线数
         *
         * 对前排线来说, 不管j是奇数, 还是偶数, 在j的左边(即i<j时),
         * 总是包含了"至少"一半线(j是奇数时, 前排线中线(j-1)包含在内), 所以:
         * 这里用: for(i=0; i<j; i+=2){}来表达左边的前排线(包括中线)
         */
        for(i=0; i<j; i+=2){
            if(i==((l-2)-i)){ /* 中线 */
                t = (long)locate->l_location[i];
            } else {
                t = (long)locate->l_location[(l-2)-i]+locate->l_location[i];
            }
            if(t<0)
                t=-t;
            t *= ((((long)l-i)-i)+1);
            v += t;
        }
        if(__slc_is_prepress(flag)) /* 预压时, 不考虑后排线 */
            goto end_this;
        /**
         * j是右边部分的第一线, 也是左边部分的线数
         *
         * 对后排线来说, 不管j是奇数, 还是偶数, 在j的左边(即i<j时),
         * 总是包含了"至多"一半线(j是奇数时, 后排线中线(j)没有包含), 所以:
         * 这里用: for(i=1; i<=j; i+=2){}来表达左边的后排线(i<=j包含了中线)
         */
        for(i=1; i<=j; i+=2){
            if(i==(l-i)){ /* 中线 */
                t = (long)locate->l_location[i];
            } else {
                t = (long)locate->l_location[l-i]+locate->l_location[i];
            }
            if(t<0)
                t=-t;
            t *= ((((long)l-i)-i)+1);
            v += t;
        }
    }
    end_this:
    #endif
    score->value = v; /* 线平衡度! */

    close_v = 0;
    for(i=0; i<l; i++){
        if(__slc_is_prepress(flag) && (i&1)){ /* 双排预压时, 后排线不参与计算 */
            continue;
        }
        j = locate->l_location[i] - slc->kl_act.l_location[i];
        if(j<0)
            j=-j;
        if(close_v < j)
            close_v = j;
    }
    score->close = close_v; /* 线就近度 */
    
    #ifdef SLC_DEBUG
    printf("slc_make_score_l(): value=%ld, close=%d\n", score->value, score->close);
    #endif
    return;
}


/*------------------------------------------------------------------------------------
 * 函数:    slc_try_locate()
 *
 * 描述:    查找排单方案
 *
 * 输入:    1) slc         分压机描述符
 *          2) k_data      刀位置数据
 *          3) k_data_nr   刀位置数据个数
 *          4) l_data      线位置数据
 *          5) l_data_nr   线位置数据个数
 *          6) flag        附加选项
 *
 * 返回:    -1: 失败
 *          ok: 成功
 *
 * 说明:    这是个不可重入函数, 调用时须加内核锁
**----------------------------------------------------------------------------------*/
static slc_locate_t __slc_locate;
int slc_try_locate(slc_descriptor_t * slc, order_kl_info_t * orderkl, INT16U flag)
{
    int    *k_selected;
    int    *l_selected;
    int    *k_location;
    int    *l_location;
    int     k, l;
    int     ordk, ordl;
    int     i, j;
    INT32U  err;
    int     last_sel;
    slc_locate_t best_k;
    slc_locate_t best_l;
    int          best_k_ok;
    int          best_l_ok;
    struct __kl_balance score_k, old_k;
    struct __kl_balance score_l, old_l;

    FamesAssert(kl_tree_pool);
    FamesAssert(slc);
    FamesAssert(orderkl);

    if(!kl_tree_pool || !slc || !orderkl)
        return 0;

    old_k.value = (long)((~0uL)>>1);
    old_l.value = (long)((~0uL)>>1);
    best_k.k_location[0]=0; /* dummy init */
    best_l.k_location[0]=0; /* dummy init */
    
    k_selected = &__slc_locate.k_selected[0];
    l_selected = &__slc_locate.l_selected[0];
    k_location = &__slc_locate.k_location[0];
    l_location = &__slc_locate.l_location[0];
    k = slc->k_number;
    l = slc->l_number;
    ordk = orderkl->k_pos_nr;
    ordl = orderkl->l_pos_nr;
    best_k_ok = 0;
    best_l_ok = 0;

    if(k<2 || l<2)
        return -1;

    for(i=0; i<k; i++){ /* 清空缓冲区 */
        k_selected[i]=0;
        k_location[i]=0;
    }
    for(i=0; i<l; i++){
        l_selected[i]=0;
        l_location[i]=0;
    }

    #ifdef SLC_DEBUG    
        printf("\n===== slc_try_locate() =====\n");
        printf("ordk=%d, ordl=%d\n", ordk, ordl);
        for(i=0; i<ordk; i++)
            printf("%-5d ", orderkl->k_pos[i]);
        printf("\n");
        for(i=0; i<ordl; i++)
            printf("%-5d ", orderkl->l_pos[i]);
        printf("\n");
    #endif
again:

build_k_tree_start:
    INIT_kl_tree_head(kl_tree_k);
    INIT_kl_tree_head(kl_tree_l);
    kl_tree_curr = 0;
    for(i=0; i<ordk; i++){
        for(j=k-1; j>=0; j--){
            if(slc->k_disable[j])
                continue;
            if(orderkl->k_pos[i] >= slc->k_lmt_left[j] &&
               orderkl->k_pos[i] <= slc->k_lmt_right[j]){
               kl_tree_pool[kl_tree_curr].kl_no = j;
               kl_tree_pool[kl_tree_curr].next = kl_tree_k[i].saved;
               kl_tree_k[i].saved = &kl_tree_pool[kl_tree_curr];
               kl_tree_curr++;
            }
        }
        if(kl_tree_k[i].saved == NULL)
            goto scan_kl_tree_ended;
        kl_tree_k[i].curr = &kl_tree_k[i].saved;
    }
build_l_tree_start:    
    for(i=0; i<ordl; i++){
        for(j=l-1; j>=0; j--){
            if(__slc_is_prepress(flag)&&(j&1))
                continue; /* 预压时, 后排线不参与排单 */
            if(slc->l_disable[j])
                continue;
            if(orderkl->l_pos[i] >= slc->l_lmt_left[j] &&
               orderkl->l_pos[i] <= slc->l_lmt_right[j]){
               kl_tree_pool[kl_tree_curr].kl_no = j;
               kl_tree_pool[kl_tree_curr].next = kl_tree_l[i].saved;
               kl_tree_l[i].saved = &kl_tree_pool[kl_tree_curr];
               kl_tree_curr++;
            }
        }
        if(kl_tree_l[i].saved == NULL)
            goto scan_kl_tree_ended;
        kl_tree_l[i].curr = &kl_tree_l[i].saved;
    }
    
/* 刀线分配树建立完成 */
    #ifdef SLC_DEBUG    
        printf("\n===== kl_tree =====");
        for(i=0; i<ordk; i++){
            struct __kl_tree * t;
            t = kl_tree_k[i].saved;
            printf("\nK%d(%5d): ", i, orderkl->k_pos[i]);
            while(t){
                printf("%d,", t->kl_no);
                t = t->next;
            }
        }
        for(i=0; i<ordl; i++){
            struct __kl_tree * t;
            t = kl_tree_l[i].saved;
            printf("\nL%d(%5d): ", i, orderkl->l_pos[i]);
            while(t){
                printf("%d,", t->kl_no);
                t = t->next;
            }
        }
        printf("\n");
    #endif

scan_kl_tree_K:  /* 扫描刀分配树 */
    err = ~SLC_ERR_NONE;
    if(ordk<=0){
        for(i=0; i<k; i++){ /* 清空选刀标记 */
            k_selected[i]=0;
            k_location[i]=0;
        }
        err = slc_adjust_k(slc, &__slc_locate, flag);
        best_k = __slc_locate;
        if(err == SLC_ERR_NONE)
            best_k_ok = 1;
        goto scan_kl_tree_L; 
    }
    while(*kl_tree_k[0].curr){ /* fetch_k_scheme */
        err = ~SLC_ERR_NONE;
        for(i=0; i<k; i++){ /* 清空选刀标记 */
            k_selected[i]=0;
            k_location[i]=0;
        }
        last_sel = -1;
        for(i=0; i<ordk; i++){
            if(kl_tree_k[i].saved==NULL)
                goto scan_kl_tree_ended; 
            while((*kl_tree_k[i].curr)->kl_no <= last_sel){
                kl_tree_k[i].curr = &(*kl_tree_k[i].curr)->next;
                if(*kl_tree_k[i].curr==NULL){
                    for(j=i; j>0; j--){
                        if((*kl_tree_k[j].curr)==NULL){
                            kl_tree_k[j].curr = &kl_tree_k[j].saved;
                            kl_tree_k[j-1].curr = &(*kl_tree_k[j-1].curr)->next;
                        }
                    }
                    goto scan_kl_tree_K; /* 下一次扫描 */
                }
            }
            last_sel = (*kl_tree_k[i].curr)->kl_no;
            k_selected[last_sel]=1;
            k_location[last_sel]=orderkl->k_pos[i];
            xxx_debug16(last_sel);
        }
        err = slc_adjust_k(slc, &__slc_locate, flag);
        xxx_debug32("slc_adjust_k return: ", err);
        if(err==SLC_ERR_NONE)
            break;
    skip_to_next_scheme_k:
    #ifdef SLC_DEBUG    
        printf("\n===== k_tree status 1=====");
        for(__i__=0; __i__<ordk; __i__++){
            struct __kl_tree * t;
            t = *kl_tree_k[__i__].curr;
            printf("\nK%d(%5d): ", __i__, orderkl->k_pos[__i__]);
            while(t){
                printf("%d,", t->kl_no);
                t = t->next;
            }
        }
        printf("\nordk=%d\n", ordk);
    #endif
        if(ordk>0){
            kl_tree_k[ordk-1].curr = &(*kl_tree_k[ordk-1].curr)->next;
        }
        for(i=ordk-1; i>0; i--){
            if((*kl_tree_k[i].curr)==NULL){
                kl_tree_k[i].curr = &kl_tree_k[i].saved;
                kl_tree_k[i-1].curr = &(*kl_tree_k[i-1].curr)->next;
            }
        }
    #ifdef SLC_DEBUG    
        printf("===== k_tree status 2=====");
        for(__i__=0; __i__<ordk; __i__++){
            struct __kl_tree * t;
            t = *kl_tree_k[__i__].curr;
            printf("\nK%d(%5d): ", __i__, orderkl->k_pos[__i__]);
            while(t){
                printf("%d,", t->kl_no);
                t = t->next;
            }
        }
        printf("\n");
        if(SLC_PAUSE)getch();
    #endif
    }
    if(err == SLC_ERR_NONE){
        slc_make_score_k(slc, &__slc_locate, &score_k, flag);
        if(__cmp_balance(&score_k, &old_k) > 0){ /* 找到了一个更好的方案 */
            old_k = score_k;
            best_k_ok = 1;
            best_k = __slc_locate;
        }
    }
    if(*kl_tree_k[0].curr){
        if(ordk>0){ /* 继续扫描下一方案(排线) */
            kl_tree_k[ordk-1].curr = &(*kl_tree_k[ordk-1].curr)->next;
        }
        for(i=ordk-1; i>0; i--){
            if((*kl_tree_k[i].curr)==NULL){
                kl_tree_k[i].curr = &kl_tree_k[i].saved;
                kl_tree_k[i-1].curr = &(*kl_tree_k[i-1].curr)->next;
            }
        }
        goto scan_kl_tree_K;
    }
xxx_debug32("k_tree scan ended", 0L);
    if(!best_k_ok){
        goto scan_kl_tree_ended; /* 排刀失败, 就不必继续排线了 */
    }
    
scan_kl_tree_L: /* 扫描线分配树 */
    err = ~SLC_ERR_NONE;
    if(ordl<=0){
        for(i=0; i<l; i++){ /* 清空选线标记 */
            l_selected[i]=0;
            l_location[i]=0;
        }
        err = slc_adjust_l(slc, &__slc_locate, flag);
        best_l = __slc_locate;
        if(err == SLC_ERR_NONE)
            best_l_ok = 1;
        goto scan_kl_tree_ended; 
    }
    while(*kl_tree_l[0].curr){ /* fetch_l_scheme */
        err = ~SLC_ERR_NONE;
        for(i=0; i<l; i++){ /* 清空选线标记 */
            l_selected[i]=0;
            l_location[i]=0;
        }
        last_sel = -1;
        for(i=0; i<ordl; i++){
            if(kl_tree_l[i].saved==NULL)
                goto scan_kl_tree_ended;
            while((*kl_tree_l[i].curr)->kl_no <= last_sel){
                kl_tree_l[i].curr = &(*kl_tree_l[i].curr)->next;
                if(*kl_tree_l[i].curr==NULL){
                    for(j=i; j>0; j--){
                        if((*kl_tree_l[j].curr)==NULL){
                            kl_tree_l[j].curr = &kl_tree_l[j].saved;
                            kl_tree_l[j-1].curr = &(*kl_tree_l[j-1].curr)->next;
                        }
                    }
                    goto scan_kl_tree_L; /* 下一次扫描 */
                }
            }
            last_sel = (*kl_tree_l[i].curr)->kl_no;
            l_selected[last_sel]=1;
            l_location[last_sel]=orderkl->l_pos[i];
            xxx_debug16(last_sel);
        }
        err = slc_adjust_l(slc, &__slc_locate, flag);
        xxx_debug32("slc_adjust_l return: ", err);
        if(err==SLC_ERR_NONE)
            break;
      skip_to_next_scheme_l:
    #ifdef SLC_DEBUG    
        printf("\n===== l_tree status 1=====");
        for(__i__=0; __i__<ordl; __i__++){
            struct __kl_tree * t;
            t = *kl_tree_l[__i__].curr;
            printf("\nL%d(%5d): ", __i__, orderkl->l_pos[__i__]);
            while(t){
                printf("%d,", t->kl_no);
                t = t->next;
            }
        }
        printf("\nordl=%d\n", ordl);
    #endif
        if(ordl>0){
            kl_tree_l[ordl-1].curr = &(*kl_tree_l[ordl-1].curr)->next;
        }
        for(i=ordl-1; i>0; i--){
            if((*kl_tree_l[i].curr)==NULL){
                kl_tree_l[i].curr = &kl_tree_l[i].saved;
                kl_tree_l[i-1].curr = &(*kl_tree_l[i-1].curr)->next;
            }
        }
    #ifdef SLC_DEBUG    
        printf("===== l_tree status 2=====");
        for(__i__=0; __i__<ordl; __i__++){
            struct __kl_tree * t;
            t = *kl_tree_l[__i__].curr;
            printf("\nL%d(%5d): ", __i__, orderkl->l_pos[__i__]);
            while(t){
                printf("%d,", t->kl_no);
                t = t->next;
            }
        }
        printf("\n");
        if(SLC_PAUSE)getch();
    #endif
    }
xxx_debug32("l_tree scan ended", 0L);
    if(err == SLC_ERR_NONE){
        slc_make_score_l(slc, &__slc_locate, &score_l, flag);
        if(__cmp_balance(&score_l, &old_l) > 0){ /* 找到了一个更好的方案 */
            old_l = score_l;
            best_l_ok = 1;
            best_l = __slc_locate;
        }
    }
    if(*kl_tree_l[0].curr){
        if(ordl>0){ /* 继续扫描下一方案(排线) */
            kl_tree_l[ordl-1].curr = &(*kl_tree_l[ordl-1].curr)->next;
        }
        for(i=ordl-1; i>0; i--){
            if((*kl_tree_l[i].curr)==NULL){
                kl_tree_l[i].curr = &kl_tree_l[i].saved;
                kl_tree_l[i-1].curr = &(*kl_tree_l[i-1].curr)->next;
            }
        }
        goto scan_kl_tree_L;
    }

scan_kl_tree_ended:  /* 刀线分配树扫描完毕, 结束 */

    if(best_k_ok) {
        for(i=0; i<k; i++){
            k_location[i]=best_k.k_location[i];
            k_selected[i]=best_k.k_selected[i];
        }
    }
    if(best_l_ok){
        for(i=0; i<l; i++){
            l_location[i]=best_l.l_location[i];
            l_selected[i]=best_l.l_selected[i];
        }
    }
    #ifdef SLC_DEBUG    
        printf("\n===== slc_try_locate() result =====\n");
        printf("k=%d\n", k);
        for(i=0; i<k; i++){
            if(k_selected[i])
                printf("[ K%d ]", i);
            else 
                printf("------");
        }
        printf("\n");
        for(i=0; i<k; i++)
            printf("%5d ", k_location[i]);
        printf("\n");
        printf("l=%d\n", l);
        for(i=0; i<l; i++){
            if(l_selected[i])
                printf("[ L%-2d]", i);
            else 
                printf("------");
        }
        printf("\n");
        for(i=0; i<l; i++)
            printf("%5d ", l_location[i]);
        printf("\n");
    #endif
    if(best_k_ok && best_l_ok)
        return ok;

    return -1;
}

/*------------------------------------------------------------------------------------
 * 函数:    slc_locate()
 *
 * 描述:    刀线选择与定位(排单算法主函数)
 *
 * 输入:    1) slc         分压机描述符
 *          2) data        分压机压线资料
 *          3) flag        附加选项
 *
 * 返回:    error  排单错误码
**----------------------------------------------------------------------------------*/
INT32U __do_slc_locate(slc_descriptor_t * slc, char data[], INT16U flag)
{
    INT32U error;
    int score;
    int v, i;
    int *k_data, k_data_nr;
    int *l_data, l_data_nr; 
    order_kl_info_t orderkl;

    FamesAssert(slc);
    FamesAssert(data);

    if(!slc || !data)
        return SLC_ERR_INVALID;

    flag |= slc->slc_flag; /* 加上全局选项 */

    switch(slc->slc_type){ /* 检查机器类型 */
        case SLC_TYPE_SINGLE:
            flag &= ~SLC_FLAG_PREP; /* 单排线不能预压 */
            break;
        case SLC_TYPE_DOUBLE:
            break;
        default:
            return SLC_ERR_INVALID_TYPE;
    }
xxx_debug16(slc->k_number);
xxx_debug16(slc->l_number);

    error = slc_check_setup(slc, flag);
    if(error != SLC_ERR_NONE){
        return error;
    }

    if(flag & SLC_FLAG_ASTD){
        slc_make_standard_location(slc, flag);
    }
    
    v = init_orderkl(slc, data, &k_data, &k_data_nr, &l_data, &l_data_nr, flag);
xxx_debug16(v);
    if(v < 0)
        return (SLC_ERR_DATA|(INT32U)(unsigned)(-v));
   
    if(k_data_nr < 0 || k_data_nr >= SLC_K_MAX_NR)
        return SLC_ERR_INVALID;

    if(l_data_nr < 0 || l_data_nr >= SLC_L_MAX_NR)
        return SLC_ERR_INVALID;        

    v = calc_k_number(slc);
    if(k_data_nr > v)
        return SLC_ERR_NO_KL;
xxx_debug16(k_data_nr); 
xxx_debug16(v); 

    v = calc_l_number(slc);
    if(l_data_nr > v)
        return SLC_ERR_NO_KL;
    if(__slc_is_prepress(flag)){ /* PrePress */
        if((l_data_nr*2) > v)
            return SLC_ERR_NO_KL;
    }
xxx_debug16(l_data_nr); 
xxx_debug16(v); 

    if(slc->order_kl.width > slc->hw_width)
        return SLC_ERR_HW_WIDTH;
xxx_debug16(slc->order_kl.width); 

    error = SLC_ERR_NONE;

    make_slc_departure(slc, flag);
    
xxx_debug32("after make_slc_departure", error);

xxx_debug16(slc_departure_nr);
    if(slc_departure_nr == 0){
        slc_orderkl_relocate(&orderkl, slc, 0);
        score = slc_try_locate(slc, &orderkl, flag);
xxx_debug16(score);
    }
    for(i=0; i<slc_departure_nr; i++){
        if(slc_departure[i]<0 && slc_departure[i] < slc->max_left_offs)
            continue;
        if(slc_departure[i]>0 && slc_departure[i] > slc->max_right_offs)
            continue;
        slc_orderkl_relocate(&orderkl, slc, slc_departure[i]);
        score = slc_try_locate(slc, &orderkl, flag);
        if(score < 0L)
            continue;
    };
    error = slc_locate_error(slc, &__slc_locate, flag);
    
    if(error == SLC_ERR_NONE) {
        slc->kl_set = __slc_locate;
    }
xxx_debug16(slc->k_number);
xxx_debug16(slc->l_number);

    return error;
}

INT32U slc_locate(slc_descriptor_t * slc, char data[], INT16U flag)
{
    INT32U err;

    FamesAssert(kl_tree_pool);
    FamesAssert(slc);
    FamesAssert(data);
    
    if(!kl_tree_pool || !slc || !data)
        return 0;

    lock_kernel();
    err = __do_slc_locate(slc, data, flag);
    unlock_kernel();

    return err;
}

/*------------------------------------------------------------------------------------
 * 函数:    slc_initialize()
 *
 * 描述:    分压机排单算法初始化
 *
 * 说明:    主要是为了分配内存空间
**----------------------------------------------------------------------------------*/
BOOL slc_initialize(void)
{
    allocate_buffer(kl_tree_pool, struct __kl_tree *, 
                     (INT32U)NR_KL_TREE_NODE()*sizeof(struct __kl_tree), return fail);
    INIT_kl_tree_head(kl_tree_k);
    INIT_kl_tree_head(kl_tree_l);
    INIT_kl_tree_pool(kl_tree_pool);
    kl_tree_curr = 0;
    
    return ok;    
}

/*------------------------------------------------------------------------------------
 * 函数:    slc_error_message()
 *
 * 描述:    由错误码返回错误信息
**----------------------------------------------------------------------------------*/
char * slc_error_message(INT32U err_code)
{
    INT16U  t;
    char   *s;

    #define __(x) ((INT16U)((x)>>16))

    t = __(err_code);
    s = "";
    
    switch(t){
        case __(SLC_ERR_NONE):
            s = pick_string("无错误", "No Error!");
            break;
        case __(SLC_ERR_INVALID):
            s = pick_string("非法参数", "Invalid Parameter");
            break;
        case __(SLC_ERR_INVALID_TYPE):
            s = pick_string("无效的分压机类型", "Invalid SLC Type");
            break;
        case __(SLC_ERR_INVALID_SETUP):
            s = pick_string("参数设置有误", "Parameter Error");
            break;
        case __(SLC_ERR_HW_WIDTH):
            s = pick_string("幅宽超出范围", "Out of Width");
            break;
        case __(SLC_ERR_NO_KL):
            s = pick_string("刀线不够用", "Out of K/L Number");
            break;
        case __(SLC_ERR_K_LIMIT):
            s = pick_string("刀机械限位", "K Out Of Range");
            break;
        case __(SLC_ERR_L_LIMIT):
            s = pick_string("线机械限位", "L Out Of Range");
            break;
        case __(SLC_ERR_KK_LMT):
            s = pick_string("刀间距离限位", "K/K Out Of Limit");
            break;
        case __(SLC_ERR_LL_LMT):
            s = pick_string("线间距离限位", "L/L Out Of Limit");
            break;
        case __(SLC_ERR_K_DISABLED):
            s = pick_string("选中了不可用的刀", "K-Disabled Selected");
            break;
        case __(SLC_ERR_L_DISABLED):
            s = pick_string("选中了不可用的线", "L-Disabled Selected");
            break;
        case __(SLC_ERR_DATA):
            t = (INT16U)(err_code & 0xFFFFL);
            switch(t){
                case 1:
                    s = pick_string("压线资料太长", "Data is too Long");
                    break;
                case 2:
                    s = pick_string("压线资料刀线重合", "Value is too Small");
                    break;
                case 3:
                    s = pick_string("压线资料数值太大", "Value is too Large");
                    break;
                case 4:
                    s = pick_string("宽度不大于0", "Width is too Small");
                    break;
                case 5:
                    s = pick_string("压线资料刀数不对", "K Nr Incorrect in Data");
                    break;
                case 6:
                    s = pick_string("压线资料有非法字符", "Illegal Character");
                    break;
                case 7:
                    s = pick_string("压线资料中刀数太多", "Too Many '*' in Data");
                    break;
                case 8:
                    s = pick_string("压线资料中线数太多", "Too Many '+' in Data");
                    break;
                case 9:
                    s = pick_string("压线资料太短", "Data is too Short");
                    break;
                default:
                    s = pick_string("压线资料错误", "Error in Data");
                    break;
            }
            break;
        default:
            s = pick_string("未知错误", "Unknown");
            break;
    }

    return s;
}

/*=====================================================================================
 * 
 * 本文件结束: slc/core.c
 * 
**===================================================================================*/


