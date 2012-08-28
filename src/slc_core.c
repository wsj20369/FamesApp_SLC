/**************************************************************************************
 * �ļ�:    slc/core.c
 *
 * ˵��:    SLC�ŵ��㷨
 *
 * ����:    Jun
 *
 * ʱ��:    2011-01-11
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
 * ����:    slc_setup_to_default()
 *
 * ����:    ��SLC��ΪĬ��״̬
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
 * ����:    slc_check_setup()
 *
 * ����:    ����������
 *
 * ˵��:    ������ʵӦ�ý�������ϸ�ֵ�, �����Ժ���˵��
**----------------------------------------------------------------------------------*/
INT32U slc_check_setup(slc_descriptor_t * slc, INT16U flag)
{
    INT32U err;

    flag = flag;

    err = SLC_ERR_INVALID_SETUP;

    if(slc->l_number & 1){ /* �������������� */
        return err;
    }
    if(slc->k_number < 3){ /* ��������С��3  */
        return err;
    }
    if(slc->l_number < 4){ /* ��������С��4  */
        return err;
    }
    if(slc->hw_width > 32000){ /* ��е�����ܴ���3200mm(3��2) */
        return err;
    }

    return SLC_ERR_NONE;
}

/*------------------------------------------------------------------------------------
 * 
 *                  ����ƫ�����������
 * 
 * ����:    make_slc_departure()
 *
 * ����:    ���ݵ���"���ɶ�"״̬��������ƫ������
 *
 * ����:    slc     SLC������, ���а����˵�����ص���Ϣ, ��
 *                  �����еĵ���λ����Ϣ(�Ѱ����ĶԳ�).
 *          flag    ѡ��
 * 
 * ���:    >=0     ����ƫ������Ĵ�С
 *          <0      ������
 *
 * ˵��:    ����ƫ����ָ���������������̨��������֮ƫ��,
 *          ���ŵ�ʱ����������ԭ��(�粻���򶩵�, ĳ���߲��ɶ���),
 *          �����������߲��ܷ��ڻ�̨����������, ��ʱ��, ����Ҫ
 *          ȷ��ƫ��Ĵ�С(��ֹһ��, ��������������ʾ)
**----------------------------------------------------------------------------------*/
#define SLC_DEPARTURE_SIZE  64   /* ����ƫ������Ĵ�С */
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
        if(slc->k_lmt_left[k] == slc->k_lmt_right[k]) {/* ���� */
            for(v=0; v<t.k_pos_nr; v++){
                if(k==0 && t.k_pos[v] < slc->k_lmt_left[k]){ 
                    /* ����ߵ� */
                    slc_departure[i]= slc->k_lmt_left[k] - t.k_pos[k];
                    i++;
                }
                if(k>0 && t.k_pos[v] > (slc->k_lmt_left[k]-slc->k_distance[k-1])
                   && t.k_pos[v] < slc->k_lmt_left[k]){
                    /* ���ô���ĳ���Ĳ��ɼ�����(��) */
                    slc_departure[i]=(slc->k_lmt_left[k]-slc->k_distance[k-1])-t.k_pos[k];
                    i++;
                    slc_departure[i]= slc->k_lmt_left[k] - t.k_pos[k];
                    i++;
                }
                if(k==(slc->k_number-1) && t.k_pos[v] > slc->k_lmt_left[k]){
                    /* ���ұߵ� */
                    slc_departure[i]= slc->k_lmt_left[k] - t.k_pos[k];
                    i++;
                }
                if(k<(slc->k_number-1) && t.k_pos[v] > slc->k_lmt_left[k]
                   && t.k_pos[v] < (slc->k_lmt_left[k]+slc->k_distance[k])){
                    /* ���ô���ĳ���Ĳ��ɼ�����(��) */
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
    if(slc->slc_type == SLC_TYPE_SINGLE){ /* ������ */
        for(l=0; l<slc->l_number; l++){
            if(slc->l_lmt_left[l] == slc->l_lmt_right[l]) {/* ���� */
                for(v=0; v<t.l_pos_nr; v++){
                    if(l==0 && t.l_pos[v] < slc->l_lmt_left[l]){
                        /* ������� */
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;                    
                    }
                    if(l>=1 && t.l_pos[v] > (slc->l_lmt_left[l]-slc->l_distance[l-1])
                       && t.l_pos[v] < slc->l_lmt_left[l]){
                        /* ���ô���ĳ�ߵĲ��ɼ�����(��) */
                        slc_departure[i]=(slc->l_lmt_left[l]-slc->l_distance[l-1])-t.l_pos[l];
                        i++;
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;
                    }
                    if(l==(slc->l_number-1) && t.l_pos[v] >  slc->l_lmt_left[l]){
                        /* ���ұ��� */
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;                    
                    }
                    if(l<=(slc->l_number-2) && t.l_pos[v] >  slc->l_lmt_left[l]
                       && t.l_pos[v] < (slc->l_lmt_left[l]+slc->l_distance[l])){
                        /* ���ô���ĳ�ߵĲ��ɼ�����(��) */
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
    if(slc->slc_type == SLC_TYPE_DOUBLE){ /* ˫���� */
        for(l=0; l<slc->l_number; l+=2){  /* 1,3,5,... */
            if(slc->l_lmt_left[l] == slc->l_lmt_right[l]) {/* ���� */
                for(v=0; v<t.l_pos_nr; v++){
                    if(l==0 && t.l_pos[v] < slc->l_lmt_left[l]){
                        /* ������� */
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;                    
                    }
                    if(l>=2 && t.l_pos[v] > (slc->l_lmt_left[l]-slc->l_distance[l-2])
                       && t.l_pos[v] < slc->l_lmt_left[l]){
                        /* ���ô���ĳ�ߵĲ��ɼ�����(��) */
                        slc_departure[i]=(slc->l_lmt_left[l]-slc->l_distance[l-2])-t.l_pos[l];
                        i++;
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;
                    }
                    if(l==(slc->l_number-2) && t.l_pos[v] >  slc->l_lmt_left[l]){
                        /* ���ұ��� */
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;                    
                    }
                    if(l<(slc->l_number-2) && t.l_pos[v] >  slc->l_lmt_left[l]
                       && t.l_pos[v] < (slc->l_lmt_left[l]+slc->l_distance[l])){
                        /* ���ô���ĳ�ߵĲ��ɼ�����(��) */
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
        if(__slc_is_prepress(flag)){ /* Ԥѹʱ, 2,4,6��ӦΪ1,3,5 */
            goto out;
        }
        for(l=1; l<slc->l_number; l+=2){  /* 2,4,6,... */
            if(slc->l_lmt_left[l] == slc->l_lmt_right[l]) {/* ���� */
                for(v=0; v<t.l_pos_nr; v++){
                    if(l==1 && t.l_pos[v] < slc->l_lmt_left[l]){
                        /* ������� */
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;                    
                    }
                    if(l>=3 && t.l_pos[v] > (slc->l_lmt_left[l]-slc->l_distance[l-2])
                       && t.l_pos[v] < slc->l_lmt_left[l]){
                        /* ���ô���ĳ�ߵĲ��ɼ�����(��) */
                        slc_departure[i]=(slc->l_lmt_left[l]-slc->l_distance[l-2])-t.l_pos[l];
                        i++;
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;
                    }
                    if(l==(slc->l_number-1) && t.l_pos[v] >  slc->l_lmt_left[l]){
                        /* ���ұ��� */
                        slc_departure[i]= slc->l_lmt_left[l] - t.l_pos[l];
                        i++;                    
                    }
                    if(l<(slc->l_number-1) && t.l_pos[v] >  slc->l_lmt_left[l]
                       && t.l_pos[v] < (slc->l_lmt_left[l]+slc->l_distance[l])){
                        /* ���ô���ĳ�ߵĲ��ɼ�����(��) */
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
 * ����:    calc_k_number()/calc_l_number()
 *
 * ����:    ������õĵ�/������
 *
 * ����:    1) slc  ��ѹ��������
 *
 * ����:    ����/����
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
 * ����:    slc_make_standard_location()
 *
 * ����:    �Զ����ɱ�׼λ��
 *
 * ����:    1) slc       ��ѹ��������
 *          2) flag      �ŵ�ѡ��(λͼ, ��: �Ƿ��ޱߵ�)
 *
 * ���:    1) slc->kl_standard ���ߵı�׼λ��
 *
 * ����:    ��
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
    } else { /* Ԥѹʱ */
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
 * ����:    init_orderkl()
 *
 * ����:    �����ĵ���λ�ó�ʼ��
 *
 * ����:    1) slc       ��ѹ��������, ��Ҫ��Ϊ������slc->order_kl
 *          2) data      ѹ������(�ַ���, ��: 100+200+100*100+200+100)
 *          3) flag      �ŵ�ѡ��(λͼ, ��: �Ƿ��ޱߵ�)
 *
 * ���:    1) k_data    �õ��Ķ���λ��
 *          2) k_data_nr k_data���������ݸ���
 *          3) l_data    ѹ�ߵĶ���λ��
 *          4) l_data_nr l_data���������ݸ���
 *
 * ����:    >0 �ܲÿ�
 *          -1 ѹ������̫��
 *          -2 �����غ�
 *          -3 ��ֵ̫��
 *          -4 ��Ȳ�����0
 *          -5 ��������
 *          -6 �зǷ��ַ�
 *          -7 ����̫��
 *          -8 ����̫��
 *          -9 ѹ������̫��
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
            case '\0': /* ���� */
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
            default: /* �����ַ� */
                return -6;
        };
        i++;
        if(i > SLC_DATA_LEN)
            return -1; /* ѹ������̫���� */
    }
    if(width <= 0)
        return -4;

    FamesAssert(k >= 2);
    if(k < 2)
        return -5;

    v = width/2; /* width��10�ı���, ���϶��Ǹ�ż�� */
    for(i=0; i<k; i++){ /* ��ƽ�� */
        slc->order_kl.k_pos[i]-=v;
    }
    for(i=0; i<l; i++){ /* ��ƽ�� */
        slc->order_kl.l_pos[i]-=v;    
    }

    if(!(flag & SLC_FLAG_TRIM)){ /* �����ޱ� */
        for(i=1; i<k; i++){
            slc->order_kl.k_pos[i-1] = slc->order_kl.k_pos[i];    
        }
        k -= 2; /* �ޱߵ�����2�� */
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
 * ����:    slc_orderkl_relocate()
 *
 * ����:    ��������λ�õ��ض�λ
 *
 * ����:    1) dst        �ض�λĿ��
 *          2) slc        SLC
 *          3) departure  ƫ��
 *
 * ����:    ��
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
 * ����:    slc_locate_error()
 *
 * ����:    ��鵶�߶�λ����
 *
 * ����:    1) slc        SLC
 *          2) locate     ��λ����
 *          3) flag       ѡ��
 *
 * ����:    ������
**----------------------------------------------------------------------------------*/
INT32U slc_locate_error(slc_descriptor_t * slc, slc_locate_t * locate, INT16U flag)
{
    INT32U error;
    int i, k, l;

    k = slc->k_number;
    l = slc->l_number;

knife:
    error = SLC_ERR_K_LIMIT;    /* �Ƿ��л�е��λ */
    for(i=0; i<k; i++){
        if(locate->k_location[i] < slc->k_lmt_left[i] ||
           locate->k_location[i] > slc->k_lmt_right[i]){
            error |= (INT32U)(unsigned)i;
            return error;
        }
    }
    error = SLC_ERR_K_DISABLED; /* �Ƿ�ѡ���˲����õ��� */
    for(i=0; i<k; i++){
        if(locate->k_selected[i] && slc->k_disable[i]){
            error |= (INT32U)(unsigned)i;
            return error;
        }
    }
    error = SLC_ERR_KK_LMT;   /* �Ƿ��м����λ */
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
                if(i&1) /* Ԥѹʱ, ���������� */
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
                if(i&1) /* Ԥѹʱ, ���������� */
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
                if(i&1) /* Ԥѹʱ, ���������� */
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
 * ����:    slc_adjust_k()/slc_adjust_l()
 *
 * ����:    ����λ�õ���
 *
 * ����:    1) slc        SLC
 *          2) locate     ��λ����
 *          3) flag       ѡ��
 *
 * ����:    ������
**----------------------------------------------------------------------------------*/
INT32U slc_adjust_k(slc_descriptor_t * slc, slc_locate_t * locate, INT16U flag)
{
    INT32U error;
    int i, j, N, t;
    int *sel, *loc;

    N = slc->k_number;

    flag = flag; /* Ŀǰ���� */

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

    t = (N/2); /* ��Լ�м���ǰѵ� */

    for(i=0; i<N; i++){
        if(sel[i])
            continue;
        loc[i] = slc->kl_standard.k_location[i]; /* ��Ϊ��׼λ�� */
        if(loc[i] < slc->k_lmt_left[i])
            loc[i] = slc->k_lmt_left[i];
        if(loc[i] > slc->k_lmt_right[i])
            loc[i] = slc->k_lmt_right[i];
    }
    if(N&1){ /* ����Ϊ����ʱ */
        if(!sel[t]){ /* �ȿ��е� */
            loc[t] = 0;    
            if(sel[t-1] && (loc[t]-loc[t-1]) < slc->k_distance[t-1])
                loc[t] = loc[t-1] + slc->k_distance[t-1];
            if(sel[t+1] && (loc[t+1]-loc[t]) < slc->k_distance[t])
                loc[t] = loc[t+1] - slc->k_distance[t];
        }
        for(i=t-1, j=t+1; i>=0; i--, j++){ /* ���м䵽���� */
            if(!sel[i]){ /* ��� */
                if((loc[i]+loc[(N-1)-i])>0){ /* ����ʹ����ƽ��(�����ɢ) */
                    loc[i] = -loc[(N-1)-i];
                }
                if((loc[i+1]-loc[i]) < slc->k_distance[i])
                    loc[i] = loc[i+1] - slc->k_distance[i];
                if(i>0 && sel[i-1] && (loc[i]-loc[i-1]) < slc->k_distance[i-1])
                    loc[i] = loc[i-1] + slc->k_distance[i-1];
            }
            if(!sel[j]){ /* �ұ� */
                if((loc[j]+loc[(N-1)-j])<0){ /* ����ʹ����ƽ��(�����ɢ) */
                    loc[j] = -loc[(N-1)-j];
                }
                if((loc[j]-loc[j-1]) < slc->k_distance[j-1])
                    loc[j] = loc[j-1] + slc->k_distance[j-1];
                if(j<N-1 && sel[j+1] && (loc[j+1]-loc[j]) < slc->k_distance[j])
                    loc[j] = loc[j+1] - slc->k_distance[j];
            }
        }
    } else { /* ����Ϊż��ʱ */
        for(i=t-1, j=t; i>=0; i--, j++){ /* ���м䵽���� */
            if(!sel[i]){ /* ��� */
                if((loc[i]+loc[(N-1)-i])>0){ /* ����ʹ����ƽ��(�����ɢ) */
                    loc[i] = -loc[(N-1)-i];
                }
                if((loc[i+1]-loc[i]) < slc->k_distance[i])
                    loc[i] = loc[i+1] - slc->k_distance[i];
                if(i>0 && sel[i-1] && (loc[i]-loc[i-1]) < slc->k_distance[i-1])
                    loc[i] = loc[i-1] + slc->k_distance[i-1];
            }
            if(!sel[j]){ /* �ұ� */
                if((loc[j]+loc[(N-1)-j])<0){ /* ����ʹ����ƽ��(�����ɢ) */
                    loc[j] = -loc[(N-1)-j];
                }
                if((loc[j]-loc[j-1]) < slc->k_distance[j-1])
                    loc[j] = loc[j-1] + slc->k_distance[j-1];
                if(j<N-1 && sel[j+1] && (loc[j+1]-loc[j]) < slc->k_distance[j])
                    loc[j] = loc[j+1] - slc->k_distance[j];
            }
        }
    }
    for(i=0; i<N; i++){ /* ����е��λ */
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

    error = SLC_ERR_K_LIMIT;    /* �Ƿ��л�е��λ */
    for(i=0; i<N; i++){
        if(loc[i] < slc->k_lmt_left[i] ||
           loc[i] > slc->k_lmt_right[i]){
            error |= (INT32U)(unsigned)i;
            return error;
        }
    }
    error = SLC_ERR_KK_LMT;     /* �Ƿ��м����λ */
    for(i=1; i<N; i++){
        if((loc[i]-loc[i-1])<slc->k_distance[i-1]){
            error |= (INT32U)(unsigned)i;
            return error;
        }
    }
    error = SLC_ERR_K_DISABLED; /* �Ƿ�ѡ���˲����õ��� */
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

    t = (N/2); /* ��Լ�м���ǰ���(ƫ��) */

single: /* ======== ������ʱ ======== */   
    if(slc->slc_type == SLC_TYPE_SINGLE){
        for(i=0; i<N; i++){
            if(sel[i])
                continue;
            loc[i] = slc->kl_standard.l_location[i]; /* ��Ϊ��׼λ�� */
            if(loc[i] < slc->l_lmt_left[i])
                loc[i] = slc->l_lmt_left[i];
            if(loc[i] > slc->l_lmt_right[i])
                loc[i] = slc->l_lmt_right[i];
        }
        for(i=t-1, j=t; i>=0; i--, j++){ /* ���м䵽���� */
            if(!sel[i]){ /* ��� */
                if((loc[i]+loc[(N-1)-i])>0){ /* ����ʹ����ƽ��(�����ɢ) */
                    loc[i] = -loc[(N-1)-i];
                }
                if((loc[i+1]-loc[i]) < slc->l_distance[i])
                    loc[i] = loc[i+1] - slc->l_distance[i];
                if(i>0 && sel[i-1] && (loc[i]-loc[i-1]) < slc->l_distance[i-1])
                    loc[i] = loc[i-1] + slc->l_distance[i-1];
            }
            if(!sel[j]){ /* �ұ� */
                if((loc[j]+loc[(N-1)-j])<0){ /* ����ʹ����ƽ��(�����ɢ) */
                    loc[j] = -loc[(N-1)-j];
                }
                if((loc[j]-loc[j-1]) < slc->l_distance[j-1])
                    loc[j] = loc[j-1] + slc->l_distance[j-1];
                if(j<N-1 && sel[j+1] && (loc[j+1]-loc[j]) < slc->l_distance[j])
                    loc[j] = loc[j+1] - slc->l_distance[j];
            }
        }
        for(i=0; i<N; i++){ /* ����е��λ */
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
        error = SLC_ERR_L_LIMIT;    /* �Ƿ��л�е��λ */
        for(i=0; i<N; i++){
            if(loc[i] < slc->l_lmt_left[i] ||
               loc[i] > slc->l_lmt_right[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_LL_LMT;     /* �Ƿ��м����λ */
        for(i=1; i<N; i++){
            if((loc[i]-loc[i-1])<slc->l_distance[i-1]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_L_DISABLED; /* �Ƿ�ѡ���˲����õ��� */
        for(i=0; i<N; i++){
            if(sel[i] && slc->l_disable[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
    }
    
double_normal: /* ======== ˫����, ��Ԥѹʱ ======== */   
    if((slc->slc_type == SLC_TYPE_DOUBLE) && (!__slc_is_prepress(flag))){
        for(i=0; i<N; i++){
            if(sel[i])
                continue;
            loc[i] = slc->kl_standard.l_location[i]; /* ��Ϊ��׼λ�� */
            if(loc[i] < slc->l_lmt_left[i])
                loc[i] = slc->l_lmt_left[i];
            if(loc[i] > slc->l_lmt_right[i])
                loc[i] = slc->l_lmt_right[i];
        }
        #if 1  /* ����������ƽ��, ���п��ܵ��±����߲�ƽ�� */
        for(i=t-1, j=t; i>=0; i--, j++){ /* ���м䵽���� */
            if(!sel[i]){ /* ��� */
                if((loc[i]+loc[(N-1)-i])>0){ /* ����ʹ����ƽ��(�����ɢ) */
                    loc[i] = -loc[(N-1)-i];
                }
                if((loc[i+1]-loc[i]) <= 0) /* ��������������, ��Ҫ���� */
                    loc[i] = loc[i+1] - (slc->l_distance[i])/2;
                if(i>0 && sel[i-1] && (loc[i]-loc[i-1]) <= 0)
                    loc[i] = loc[i-1] + (slc->l_distance[i-1]/2);
                if((loc[i+2]-loc[i]) < slc->l_distance[i]) /* ������߼�϶ */
                    loc[i] = loc[i+2] - slc->l_distance[i];
                if(i>1 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                    loc[i] = loc[i-2] + slc->l_distance[i-2];
            }
            if(!sel[j]){ /* �ұ� */
                if((loc[j]+loc[(N-1)-j])<0){ /* ����ʹ����ƽ��(�����ɢ) */
                    loc[j] = -loc[(N-1)-j];
                }
                if((loc[j]-loc[j-1]) <= 0) /* ��������������, ��Ҫ���� */
                    loc[j] = loc[j-1] + (slc->l_distance[j-1])/2;
                if(j<N-1 && sel[j+1] && (loc[j+1]-loc[j]) <= 0)
                    loc[j] = loc[j+1] - (slc->l_distance[j])/2;
                if((loc[j]-loc[j-2]) < slc->l_distance[j-2]) /* ������߼�϶ */
                    loc[j] = loc[j-2] + slc->l_distance[j-2];
                if(j<N-2 && sel[j+2] && (loc[j+2]-loc[j]) < slc->l_distance[j])
                    loc[j] = loc[j+2] - slc->l_distance[j];
            }
        }
        #else /* һ����ֻ�뱾����ƽ��, ����һ���߲�����ϵ */
        /**
         * t=N/2, �����Ļ�, (t)�����ұ߲��ֵĵ�һ��, ͬʱҲ����߲��ֵ�����, ��ô:
         * 
         * ���t������, ��ô��߲���ǰ���߱Ⱥ����߶�1, 
         *                  �ұ߲���ǰ���߱Ⱥ�������1, ���������, ��:
         *
         *       (t-1)��ǰ��������,
         *       (t-3)��ǰ������һ,
         *       (t+1)��ǰ������һ,
         *       (t)  �Ǻ���������,
         *       (t-2)�Ǻ�������һ,
         *       (t+2)�Ǻ�������һ;
         *
         * ���t��ż��, ��ô��߲���ǰ�������������ͬ, 
         *                  �ұ߲���ǰ�����������Ҳ��ͬ, ���������, ��:
         *       
         *       (t-2)��ǰ������һ,
         *       (t)  ��ǰ������һ,
         *       (t-1)�Ǻ�������һ,
         *       (t+1)�Ǻ�������һ, ǰ���ž�û������;
         *
         * ���е�N��������(�������ڲ�����N=slc->l_number)
         *
         * ���Ͻ��۵�ǰ��: 1) ����������ż��, 2) ��������С��4
         */
        if(t&1){ /* ���t������... */
            if(!sel[t-1]){ /* ǰ������ */
                i = t-1;
                loc[i] = 0;    
                if(sel[i+2] && (loc[i+2]-loc[i]) < slc->l_distance[i])
                    loc[i] = loc[i+2] - slc->l_distance[i];
                if(sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                    loc[i] = loc[i-2] + slc->l_distance[i-2];
            }
            for(i=t-3, j=t+1; i>=0; i-=2, j+=2){ /* ǰ�������� */
                if(!sel[i]){ /* ��� */
                    if((loc[i]+loc[(N-2)-i])>0){ /* ����ʹ����ƽ��(�����ɢ) */
                        loc[i] = -loc[(N-2)-i];
                    }
                    if((loc[i+2]-loc[i]) < slc->l_distance[i])
                        loc[i] = loc[i+2] - slc->l_distance[i];
                    if(i>1 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                        loc[i] = loc[i-2] + slc->l_distance[i-2];
                }
                if(!sel[j]){ /* �ұ� */
                    if((loc[j]+loc[(N-2)-j])<0){ /* ����ʹ����ƽ��(�����ɢ) */
                        loc[j] = -loc[(N-2)-j];
                    }
                    if((loc[j]-loc[j-2]) < slc->l_distance[j-2])
                        loc[j] = loc[j-2] + slc->l_distance[j-2];
                    if(j<N-2 && sel[j+2] && (loc[j+2]-loc[j]) < slc->l_distance[j])
                        loc[j] = loc[j+2] - slc->l_distance[j];
                }
            }
            if(!sel[t]){ /* �������� */
                i = t;
                loc[i] = 0;    
                if(sel[i+2] && (loc[i+2]-loc[i]) < slc->l_distance[i])
                    loc[i] = loc[i+2] - slc->l_distance[i];
                if(sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                    loc[i] = loc[i-2] + slc->l_distance[i-2];
            }
            for(i=t-2, j=t+2; i>=1; i-=2, j+=2){ /* ���������� */
                if(!sel[i]){ /* ��� */
                    if((loc[i]+loc[(N-2)-i])>0){ /* ����ʹ����ƽ��(�����ɢ) */
                        loc[i] = -loc[(N-2)-i];
                    }
                    if((loc[i+2]-loc[i]) < slc->l_distance[i])
                        loc[i] = loc[i+2] - slc->l_distance[i];
                    if(i>2 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                        loc[i] = loc[i-2] + slc->l_distance[i-2];
                }
                if(!sel[j]){ /* �ұ� */
                    if((loc[j]+loc[(N-2)-j])<0){ /* ����ʹ����ƽ��(�����ɢ) */
                        loc[j] = -loc[(N-2)-j];
                    }
                    if((loc[j]-loc[j-2]) < slc->l_distance[j-2])
                        loc[j] = loc[j-2] + slc->l_distance[j-2];
                    if(j<N-1 && sel[j+2] && (loc[j+2]-loc[j]) < slc->l_distance[j])
                        loc[j] = loc[j+2] - slc->l_distance[j];
                }
            }
        } else { /* ���t��ż��...ǰ���ž�û������... */
            for(i=t-2, j=t; i>=0; i-=2, j+=2){ /* ǰ�������� */
                if(!sel[i]){ /* ��� */
                    if((loc[i]+loc[(N-2)-i])>0){ /* ����ʹ����ƽ��(�����ɢ) */
                        loc[i] = -loc[(N-2)-i];
                    }
                    if((loc[i+2]-loc[i]) < slc->l_distance[i])
                        loc[i] = loc[i+2] - slc->l_distance[i];
                    if(i>1 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                        loc[i] = loc[i-2] + slc->l_distance[i-2];
                }
                if(!sel[j]){ /* �ұ� */
                    if((loc[j]+loc[(N-2)-j])<0){ /* ����ʹ����ƽ��(�����ɢ) */
                        loc[j] = -loc[(N-2)-j];
                    }
                    if((loc[j]-loc[j-2]) < slc->l_distance[j-2])
                        loc[j] = loc[j-2] + slc->l_distance[j-2];
                    if(j<N-2 && sel[j+2] && (loc[j+2]-loc[j]) < slc->l_distance[j])
                        loc[j] = loc[j+2] - slc->l_distance[j];
                }
            }
            for(i=t-1, j=t+1; i>=1; i-=2, j+=2){ /* ���������� */
                if(!sel[i]){ /* ��� */
                    if((loc[i]+loc[(N-2)-i])>0){ /* ����ʹ����ƽ��(�����ɢ) */
                        loc[i] = -loc[(N-2)-i];
                    }
                    if((loc[i+2]-loc[i]) < slc->l_distance[i])
                        loc[i] = loc[i+2] - slc->l_distance[i];
                    if(i>2 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                        loc[i] = loc[i-2] + slc->l_distance[i-2];
                }
                if(!sel[j]){ /* �ұ� */
                    if((loc[j]+loc[(N-2)-j])<0){ /* ����ʹ����ƽ��(�����ɢ) */
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
        for(i=0; i<N; i++){ /* ����е��λ */
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
        error = SLC_ERR_L_LIMIT;    /* �Ƿ��л�е��λ */
        for(i=0; i<N; i++){
            if(loc[i] < slc->l_lmt_left[i] ||
               loc[i] > slc->l_lmt_right[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_LL_LMT;     /* �Ƿ��м����λ */
        for(i=2; i<N; i++){
            if((loc[i]-loc[i-2])<slc->l_distance[i-2]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_L_DISABLED; /* �Ƿ�ѡ���˲����õ��� */
        for(i=0; i<N; i++){
            if(sel[i] && slc->l_disable[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
    }

double_prepress: /* ======== ˫����, ��Ԥѹʱ ======== */   
    if((slc->slc_type == SLC_TYPE_DOUBLE) && (__slc_is_prepress(flag))){
        for(i=0; i<N; i+=2){
            if(sel[i])
                continue;
            loc[i] = slc->kl_standard.l_location[i]; /* ��Ϊ��׼λ�� */
            if(loc[i] < slc->l_lmt_left[i])
                loc[i] = slc->l_lmt_left[i];
            if(loc[i] > slc->l_lmt_right[i])
                loc[i] = slc->l_lmt_right[i];
        }
        if(t&1){ /* ���t������... */
            if(!sel[t-1]){ /* ǰ������ */
                i = t-1;
                loc[i] = 0;    
                if(sel[i+2] && (loc[i+2]-loc[i]) < slc->l_distance[i])
                    loc[i] = loc[i+2] - slc->l_distance[i];
                if(sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                    loc[i] = loc[i-2] + slc->l_distance[i-2];
            }
            for(i=t-3, j=t+1; i>=0; i-=2, j+=2){ /* ǰ�������� */
                if(!sel[i]){ /* ��� */
                    if((loc[i]+loc[(N-2)-i])>0){ /* ����ʹ����ƽ��(�����ɢ) */
                        loc[i] = -loc[(N-2)-i];
                    }
                    if((loc[i+2]-loc[i]) < slc->l_distance[i])
                        loc[i] = loc[i+2] - slc->l_distance[i];
                    if(i>1 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                        loc[i] = loc[i-2] + slc->l_distance[i-2];
                }
                if(!sel[j]){ /* �ұ� */
                    if((loc[j]+loc[(N-2)-j])<0){ /* ����ʹ����ƽ��(�����ɢ) */
                        loc[j] = -loc[(N-2)-j];
                    }
                    if((loc[j]-loc[j-2]) < slc->l_distance[j-2])
                        loc[j] = loc[j-2] + slc->l_distance[j-2];
                    if(j<N-2 && sel[j+2] && (loc[j+2]-loc[j]) < slc->l_distance[j])
                        loc[j] = loc[j+2] - slc->l_distance[j];
                }
            }
        } else { /* ���t��ż��...ǰ���ž�û������... */
            for(i=t-2, j=t; i>=0; i-=2, j+=2){ /* ǰ�������� */
                if(!sel[i]){ /* ��� */
                    if((loc[i]+loc[(N-2)-i])>0){ /* ����ʹ����ƽ��(�����ɢ) */
                        loc[i] = -loc[(N-2)-i];
                    }
                    if((loc[i+2]-loc[i]) < slc->l_distance[i])
                        loc[i] = loc[i+2] - slc->l_distance[i];
                    if(i>1 && sel[i-2] && (loc[i]-loc[i-2]) < slc->l_distance[i-2])
                        loc[i] = loc[i-2] + slc->l_distance[i-2];
                }
                if(!sel[j]){ /* �ұ� */
                    if((loc[j]+loc[(N-2)-j])<0){ /* ����ʹ����ƽ��(�����ɢ) */
                        loc[j] = -loc[(N-2)-j];
                    }
                    if((loc[j]-loc[j-2]) < slc->l_distance[j-2])
                        loc[j] = loc[j-2] + slc->l_distance[j-2];
                    if(j<N-2 && sel[j+2] && (loc[j+2]-loc[j]) < slc->l_distance[j])
                        loc[j] = loc[j+2] - slc->l_distance[j];
                }
            }
        }
        for(i=0; i<N; i+=2){ /* ����е��λ */
            if(sel[i])
                continue;
            if(loc[i] < slc->l_lmt_left[i])
                loc[i] = slc->l_lmt_left[i];
            if(loc[i] > slc->l_lmt_right[i])
                loc[i] = slc->l_lmt_right[i];
        }
        for(i=1; i<N; i+=2){ /* !!!������ӳ��Ϊǰ����!!!, �����Ԥѹ! */
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
        error = SLC_ERR_L_LIMIT;    /* �Ƿ��л�е��λ */
        for(i=0; i<N; i+=2){
            if(loc[i] < slc->l_lmt_left[i] ||
               loc[i] > slc->l_lmt_right[i]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_LL_LMT;     /* �Ƿ��м����λ */
        for(i=2; i<N; i+=2){
            if((loc[i]-loc[i-2])<slc->l_distance[i-2]){
                error |= (INT32U)(unsigned)i;
                return error;
            }
        }
        error = SLC_ERR_L_DISABLED; /* �Ƿ�ѡ���˲����õ��� */
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
 *                  ���߷�����
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

static struct __kl_tree * kl_tree_pool = NULL; /* �����ʼ��ΪNULL */
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
 *                  ƽ���
 * 
 * __cmp_balance()�����ıȽϷ�ʽ�൱�ؼ�, ������Ͼ����ŵ������ľ�����
 *
 * ����ֵ:  <0   score1��score2��
 *          =0   ��ͬ
 *          >0   score1��score2��
 *
 * Ŀǰ�ıȽ�ԭ��Ϊ: ƽ�������
**----------------------------------------------------------------------------------*/
struct __kl_balance {
    long value;   /* ƽ���   */
    int  close;   /* �ͽ���   */
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

    flag = flag; /* Ŀǰ���� */

    v = 0L;
    if(k&1) {/* KΪ����ʱ */
        j = k/2;
        for(i=0; i<j; i++){
            t = (long)locate->k_location[(k-1)-i]+locate->k_location[i];
            if(t<0L)
                t=-t;
            t *= (((long)k-i)-i);  /* ���ϵ��ߵ�ƽ��ȸ���Ҫ??? �������, ��~ */
            v += t;
        }
        t = (long)locate->k_location[j];
        if(t<0L)
            t=-t;
        v += t; /* �����е���ƫ�� */
    } else {/* KΪż��ʱ */
        j = k/2;
        for(i=0; i<j; i++){
            t = (long)locate->k_location[(k-1)-i]+locate->k_location[i];
            if(t<0)
                t=-t;
            t *= (((long)k-i)-i); 
            v += t;
        }
    }
    score->value = v; /* ��ƽ���! */

    close_v = 0;
    for(i=0; i<k; i++){
        j = locate->k_location[i] - slc->kl_act.k_location[i];
        if(j<0)
            j=-j;
        if(close_v < j)
            close_v = j;
    }
    score->close = close_v; /* ���ͽ��� */
    
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
    #if 1 /* ����������ƽ�� */
    if(slc->slc_type == SLC_TYPE_DOUBLE){
        if(!__slc_is_prepress(flag)){
            for(i=0; i<j; i++){
                t = (long)locate->l_location[(l-1)-i]+locate->l_location[i];
                if(t<0)
                    t=-t;
                t *= (((long)l-i)-i);
                v += t;
            }
            for(i=1; i<l; i++){ /* �������ת��� */
                if(locate->l_location[i] <= locate->l_location[i-1])
                    v += 0x01000000L;
            }
        } else { /* Ԥѹʱ */
            for(i=0; i<j; i+=2){
                if(i==((l-2)-i)){ /* ���� */
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
    #else /* һ����ֻ�뱾����ƽ��, ����һ���߲�����ϵ */
    if(slc->slc_type == SLC_TYPE_DOUBLE){
        /**
         * j���ұ߲��ֵĵ�һ��, Ҳ����߲��ֵ�����
         *
         * ��ǰ������˵, ����j������, ����ż��, ��j�����(��i<jʱ),
         * ���ǰ�����"����"һ����(j������ʱ, ǰ��������(j-1)��������), ����:
         * ������: for(i=0; i<j; i+=2){}�������ߵ�ǰ����(��������)
         */
        for(i=0; i<j; i+=2){
            if(i==((l-2)-i)){ /* ���� */
                t = (long)locate->l_location[i];
            } else {
                t = (long)locate->l_location[(l-2)-i]+locate->l_location[i];
            }
            if(t<0)
                t=-t;
            t *= ((((long)l-i)-i)+1);
            v += t;
        }
        if(__slc_is_prepress(flag)) /* Ԥѹʱ, �����Ǻ����� */
            goto end_this;
        /**
         * j���ұ߲��ֵĵ�һ��, Ҳ����߲��ֵ�����
         *
         * �Ժ�������˵, ����j������, ����ż��, ��j�����(��i<jʱ),
         * ���ǰ�����"����"һ����(j������ʱ, ����������(j)û�а���), ����:
         * ������: for(i=1; i<=j; i+=2){}�������ߵĺ�����(i<=j����������)
         */
        for(i=1; i<=j; i+=2){
            if(i==(l-i)){ /* ���� */
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
    score->value = v; /* ��ƽ���! */

    close_v = 0;
    for(i=0; i<l; i++){
        if(__slc_is_prepress(flag) && (i&1)){ /* ˫��Ԥѹʱ, �����߲�������� */
            continue;
        }
        j = locate->l_location[i] - slc->kl_act.l_location[i];
        if(j<0)
            j=-j;
        if(close_v < j)
            close_v = j;
    }
    score->close = close_v; /* �߾ͽ��� */
    
    #ifdef SLC_DEBUG
    printf("slc_make_score_l(): value=%ld, close=%d\n", score->value, score->close);
    #endif
    return;
}


/*------------------------------------------------------------------------------------
 * ����:    slc_try_locate()
 *
 * ����:    �����ŵ�����
 *
 * ����:    1) slc         ��ѹ��������
 *          2) k_data      ��λ������
 *          3) k_data_nr   ��λ�����ݸ���
 *          4) l_data      ��λ������
 *          5) l_data_nr   ��λ�����ݸ���
 *          6) flag        ����ѡ��
 *
 * ����:    -1: ʧ��
 *          ok: �ɹ�
 *
 * ˵��:    ���Ǹ��������뺯��, ����ʱ����ں���
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

    for(i=0; i<k; i++){ /* ��ջ����� */
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
                continue; /* Ԥѹʱ, �����߲������ŵ� */
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
    
/* ���߷������������ */
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

scan_kl_tree_K:  /* ɨ�赶������ */
    err = ~SLC_ERR_NONE;
    if(ordk<=0){
        for(i=0; i<k; i++){ /* ���ѡ����� */
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
        for(i=0; i<k; i++){ /* ���ѡ����� */
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
                    goto scan_kl_tree_K; /* ��һ��ɨ�� */
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
        if(__cmp_balance(&score_k, &old_k) > 0){ /* �ҵ���һ�����õķ��� */
            old_k = score_k;
            best_k_ok = 1;
            best_k = __slc_locate;
        }
    }
    if(*kl_tree_k[0].curr){
        if(ordk>0){ /* ����ɨ����һ����(����) */
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
        goto scan_kl_tree_ended; /* �ŵ�ʧ��, �Ͳ��ؼ��������� */
    }
    
scan_kl_tree_L: /* ɨ���߷����� */
    err = ~SLC_ERR_NONE;
    if(ordl<=0){
        for(i=0; i<l; i++){ /* ���ѡ�߱�� */
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
        for(i=0; i<l; i++){ /* ���ѡ�߱�� */
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
                    goto scan_kl_tree_L; /* ��һ��ɨ�� */
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
        if(__cmp_balance(&score_l, &old_l) > 0){ /* �ҵ���һ�����õķ��� */
            old_l = score_l;
            best_l_ok = 1;
            best_l = __slc_locate;
        }
    }
    if(*kl_tree_l[0].curr){
        if(ordl>0){ /* ����ɨ����һ����(����) */
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

scan_kl_tree_ended:  /* ���߷�����ɨ�����, ���� */

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
 * ����:    slc_locate()
 *
 * ����:    ����ѡ���붨λ(�ŵ��㷨������)
 *
 * ����:    1) slc         ��ѹ��������
 *          2) data        ��ѹ��ѹ������
 *          3) flag        ����ѡ��
 *
 * ����:    error  �ŵ�������
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

    flag |= slc->slc_flag; /* ����ȫ��ѡ�� */

    switch(slc->slc_type){ /* ���������� */
        case SLC_TYPE_SINGLE:
            flag &= ~SLC_FLAG_PREP; /* �����߲���Ԥѹ */
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
 * ����:    slc_initialize()
 *
 * ����:    ��ѹ���ŵ��㷨��ʼ��
 *
 * ˵��:    ��Ҫ��Ϊ�˷����ڴ�ռ�
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
 * ����:    slc_error_message()
 *
 * ����:    �ɴ����뷵�ش�����Ϣ
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
            s = pick_string("�޴���", "No Error!");
            break;
        case __(SLC_ERR_INVALID):
            s = pick_string("�Ƿ�����", "Invalid Parameter");
            break;
        case __(SLC_ERR_INVALID_TYPE):
            s = pick_string("��Ч�ķ�ѹ������", "Invalid SLC Type");
            break;
        case __(SLC_ERR_INVALID_SETUP):
            s = pick_string("������������", "Parameter Error");
            break;
        case __(SLC_ERR_HW_WIDTH):
            s = pick_string("��������Χ", "Out of Width");
            break;
        case __(SLC_ERR_NO_KL):
            s = pick_string("���߲�����", "Out of K/L Number");
            break;
        case __(SLC_ERR_K_LIMIT):
            s = pick_string("����е��λ", "K Out Of Range");
            break;
        case __(SLC_ERR_L_LIMIT):
            s = pick_string("�߻�е��λ", "L Out Of Range");
            break;
        case __(SLC_ERR_KK_LMT):
            s = pick_string("���������λ", "K/K Out Of Limit");
            break;
        case __(SLC_ERR_LL_LMT):
            s = pick_string("�߼������λ", "L/L Out Of Limit");
            break;
        case __(SLC_ERR_K_DISABLED):
            s = pick_string("ѡ���˲����õĵ�", "K-Disabled Selected");
            break;
        case __(SLC_ERR_L_DISABLED):
            s = pick_string("ѡ���˲����õ���", "L-Disabled Selected");
            break;
        case __(SLC_ERR_DATA):
            t = (INT16U)(err_code & 0xFFFFL);
            switch(t){
                case 1:
                    s = pick_string("ѹ������̫��", "Data is too Long");
                    break;
                case 2:
                    s = pick_string("ѹ�����ϵ����غ�", "Value is too Small");
                    break;
                case 3:
                    s = pick_string("ѹ��������ֵ̫��", "Value is too Large");
                    break;
                case 4:
                    s = pick_string("��Ȳ�����0", "Width is too Small");
                    break;
                case 5:
                    s = pick_string("ѹ�����ϵ�������", "K Nr Incorrect in Data");
                    break;
                case 6:
                    s = pick_string("ѹ�������зǷ��ַ�", "Illegal Character");
                    break;
                case 7:
                    s = pick_string("ѹ�������е���̫��", "Too Many '*' in Data");
                    break;
                case 8:
                    s = pick_string("ѹ������������̫��", "Too Many '+' in Data");
                    break;
                case 9:
                    s = pick_string("ѹ������̫��", "Data is too Short");
                    break;
                default:
                    s = pick_string("ѹ�����ϴ���", "Error in Data");
                    break;
            }
            break;
        default:
            s = pick_string("δ֪����", "Unknown");
            break;
    }

    return s;
}

/*=====================================================================================
 * 
 * ���ļ�����: slc/core.c
 * 
**===================================================================================*/


