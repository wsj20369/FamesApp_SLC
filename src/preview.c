/******************************************************************************************
 * 文件:    slc/preview.c
 *
 * 描述:    订单预览控件
 *
 * 作者:    Jun
******************************************************************************************/
#define  SLC_PREVIEW_C
#include <FamesOS.h>
#include "common.h"

/*-----------------------------------------------------------------------------------------
 * 
 *      订单预览控件
 * 
**---------------------------------------------------------------------------------------*/
static int __widget_id_preview = -1; /* 控件ID */

struct gui_preview_private_s {
    int   redraw;
    int   have_order;
    order_struct order;
    RECT  order_info_area;
    int   ruler_font;
    COLOR ruler_color;
    int   ruler_unit_font;
    COLOR ruler_unit_color;
    int   order_info_font;
    COLOR order_info_color;
    COLOR preview_paper_color;
    int   preview_marker_font;
    COLOR preview_marker_color;
    slc_descriptor_t temp_slc;
};
typedef struct gui_preview_private_s gui_preview_private;


/*-----------------------------------------------------------------------------------------
 * 函数:    preview_init_private()
 *
 * 描述:    实例初始化, 主要是给私有结构申请内存
**---------------------------------------------------------------------------------------*/
BOOL guical preview_init_private(gui_widget * preview)
{
    INT08S * buf;
    gui_preview_private * t, * t2;
    int  bytes;
    
    FamesAssert(preview);

    if(!preview)
        return fail;

    FamesAssert(preview->type == __widget_id_preview);

    if(preview->type != __widget_id_preview)
        return fail;

    bytes = (int)sizeof(gui_preview_private);

    buf = (INT08S *)mem_alloc((INT32U)(INT32S)bytes);

    if(buf){
        MEMSET(buf, 0, bytes);
        t = (gui_preview_private *)buf;/*lint !e826*/
        t->redraw = 0;
        t->have_order = 0;
        t->ruler_font = preview->font;
        t->ruler_color = preview->color;
        t->ruler_unit_font = preview->font;
        t->ruler_unit_color = preview->color;
        t->order_info_font = preview->font;
        t->order_info_color = preview->color;
        t->preview_paper_color = PREVIEW_PAPER_COLOR;
        t->preview_marker_font = preview->font;
        t->preview_marker_color = preview->color;
        if(preview->private_data){
            lock_kernel();
            t2 = preview->private_data;
            preview->private_data = NULL;
            unlock_kernel();
            mem_free(t2);
        }
        lock_kernel();
        preview->private_data = (void *)t;
        unlock_kernel();
        gui_refresh_widget(preview);
        return ok;
    } else {
        return fail;
    }
}

/*-----------------------------------------------------------------------------------------
 * 函数:    preview_set_order()
 *
 * 描述:    给个订单, 让他预览预览...
**---------------------------------------------------------------------------------------*/
BOOL guical preview_set_order(gui_widget * preview, order_struct * order)
{
    gui_preview_private * t;
    BOOL retval;

    FamesAssert(preview);

    if(!preview)
        return fail;

    retval = fail;
    
    lock_kernel();
    t = (gui_preview_private *)preview->private_data;
    if(t){
        if(order){
            if(t->have_order){
                if(MEMCMP((INT08S *)(&t->order), (INT08S *)order, sizeof(*order))){
                    t->redraw = 1;
                    t->order = *order;
                    t->have_order = 1;
                } else {
                    ; /* 两个单是相同的, 没必要重画         */
                }
            } else {  /* 之前没有订单, 现在有了, 所有要重画 */
                t->redraw = 1;
                t->order = *order;
                t->have_order = 1;
            }
        } else {
            if(t->have_order){
                t->redraw = 1;
                t->have_order = 0;
            } else {
                ;     /* 之前也没有订单, 所有不需要重画     */
            }
        }
        retval = ok;
    }
    unlock_kernel();
    
    return retval;
}

/*-----------------------------------------------------------------------------------------
 * 函数:    preview_set_param()
 *
 * 描述:    设置预览参数
**---------------------------------------------------------------------------------------*/
BOOL guical preview_set_param(gui_widget * preview,
                              int   ruler_font, COLOR ruler_color,
                              int   ruler_unit_font, COLOR ruler_unit_color,
                              int   order_info_font, COLOR order_info_color,
                              COLOR preview_paper_color,
                              int   preview_marker_font, COLOR preview_marker_color
                             )
{
    gui_preview_private * t;
    BOOL retval;

    FamesAssert(preview);

    if(!preview)
        return fail;

    retval = fail;
    
    lock_kernel();
    t = (gui_preview_private *)preview->private_data;
    if(t){
        t->redraw = 1;
        retval = ok;
        if(ruler_font >= 0)
            t->ruler_font = ruler_font;
        t->ruler_color = ruler_color;
        if(ruler_unit_font >= 0)
            t->ruler_unit_font = ruler_unit_font;
        t->ruler_unit_color = ruler_unit_color;
        if(order_info_font >= 0)
            t->order_info_font = order_info_font;
        t->order_info_color = order_info_color;
        t->preview_paper_color = preview_paper_color;
        if(preview_marker_font >= 0)
            t->preview_marker_font = preview_marker_font;
        t->preview_marker_color = preview_marker_color;
        gui_refresh_widget(preview);
    }
    unlock_kernel();
    
    return retval;
}

/*-----------------------------------------------------------------------------------------
 * 函数:    ___draw_width_mark()
 *
 * 描述:    绘图, 类似于: <---- 123.4 ---->
 *
 * 说明:    只用于函数__widget_preview_draw()
**---------------------------------------------------------------------------------------*/
void ___draw_width_mark(int x, int x1, int y, int height, int value, 
                        int font, COLOR color, COLOR bkcolor)
{
    char __s[16];
    int  __len, v;

    #if 0
    printf("x=%d, x1=%d, y=%d, value=%d\n", x, x1, y, value);
    #endif
    value /= 10;             /* value的单位是0.1毫米, 这里需要转化为毫米 */
    v = ((height-14)/2)+y;   /* 画"<---->"的纵坐标 */
    if((x1-x) >= 60){        /* 60 = (3+4)*8+4 */
        INT16toSTR(&__s[1], value, CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_SIG);
        __s[0]=' ';
        __len = STRLEN(__s);
        __s[__len]=' ';
        __s[__len+1]=0;
        draw_font(x+1,  v, "<", color, 0, 0);
        draw_font(x1-7, v, ">", color, 0, 0);
        x+=5; x1-=5;
        gdi_draw_h_line(x, v+7, x1, color);
    } else if((x1-x) >= 44){ /* 44 = (3+2)*8+4 */
        INT16toSTR(&__s[0], value, CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_SIG);
        draw_font(x+1,  v, "<", color, 0, 0);
        draw_font(x1-7, v, ">", color, 0, 0);
        x+=9; x1-=7;
    } else if((x1-x) > 28){  /* 28 = (3)*8+4 */
        INT16toSTR(&__s[0], value, CHG_OPT_END|CHG_OPT_DEC|CHG_OPT_SIG);
        x+=1;
    } else {
        __len = ((x1-x)/get_font_width(font))-2;/* 一般情况下, 一个字符占8个象素 */
        if(__len < 0)
            __len = 0;
        __s[__len] = 0;
        while((--__len) >= 0)
            __s[__len] = '.';
    }
    if(value > 0){
        if(font == 0){ /* FamesOS系统字体: 7x14 */
            y += 1;
        } else {
            y += 2;
        }
        draw_font_ex(x, y, x1-x, height, __s, color, bkcolor, font, DRAW_OPT_ALIGN_CENTER|DRAW_OPT_FIL_BG);
    }
}


/*-----------------------------------------------------------------------------------------
 * 函数:    __widget_preview_draw()
 *
 * 描述:    绘图
 *
 * 说明:    这里的绘图部分要求了这类控件的宽度不能小于910
**---------------------------------------------------------------------------------------*/
#if CONFIG_PREVIEW_WIDER == 1

void __widget_preview_draw(gui_widget * preview)
{
/*lint --e{534}*/
    int x, y, x1, y1, w, h, move;
    COLOR bkcolor, color;
    gui_preview_private * t;
    order_struct * order;
    int i, _x, _y, _x1, _y1;
    int __x, __y, __x1;
    INT16U locate_flag;
    RECT * inner_rect, * oi_area;
    char order_info_buf[256], x_buf[2048];

    #define PREVIEW_MAX_WIDTH 3000  /* 预览支持的最大纸宽是3米 */

    FamesAssert(preview);

    if(!preview)
        return;

    t = (gui_preview_private *)preview->private_data;
    if(!t)
        return;
    
    inner_rect = &preview->inner_rect;
    oi_area    = &t->order_info_area;
    color = preview->color;
    bkcolor = preview->bkcolor;
    if(bkcolor==0)
        bkcolor = WIDGET_BKCOLOR;
    
    if(preview->flag & GUI_WIDGET_FLAG_REFRESH){
        x  = preview->real_rect.x;
        y  = preview->real_rect.y;
        x1 = preview->real_rect.width + x;
        y1 = preview->real_rect.height + y;
        if(preview->style & PREVIEW_STYLE_MODAL_FRAME){
            move = gui_widget_draw_modal_frame(x, y, x1, y1);
            ___gui_widget_xy_move(move);
        } 
        if(preview->style & PREVIEW_STYLE_CLIENT_BDR){
            move = gui_widget_draw_client_bdr(x, y, x1, y1);
            ___gui_widget_xy_move(move);
        } 
        if(preview->style & PREVIEW_STYLE_BORDER){
            move = gui_widget_draw_static_bdr(x, y, x1, y1);
            ___gui_widget_xy_move(move);
        } 
        if(preview->style & PREVIEW_STYLE_SUBSIDE){
            move = gui_widget_draw_subside_bdr(x, y, x1, y1);
            ___gui_widget_xy_move(move);
        } 
        if(!gdi_draw_box(x, y, x1, y1, bkcolor)){
            ;
        }
        __x1 = x1;         /* 右端边界位置 */
        w  = (x1-x)+1;
        h  = (y1-y)+1;
        if(w < 840)
            w = 840;
        x += (w-840)/2;
        x1 = x+839;
        __x = x;           /* 记住起始位置 */
        __y = y;

        y += 6;
        if(1){             /* 显示标尺开始 */
            char *__s[] = { "-1500", "-1000", "-500", "0", "500", "1000", "1500" };
            x -= 22;
            for(i=0; i<7; i++){
                draw_font_ex(x, y, 50, 8, __s[i], color, bkcolor, preview->font, DRAW_OPT_ALIGN_CENTER);
                x += 140;
            }
        }
        x  = __x;
        y += 16;           /* 画标尺的小竖线 */  
        _y1 = y+8;
        for(i=0; i<=30; i++){
            if(i%5)
                _y = y+3;
            else 
                _y = y;
            gdi_draw_v_line(x-1, _y+1, _y1, color);
            gdi_draw_v_line(x, _y, _y1, color);
            gdi_draw_v_line(x+1, _y+1, _y1, color);
            x += 28;
        }
        x = __x;
        y = _y1+1;
        gdi_draw_h_line(x-1, y, x1+1+1, color);
        
        _y = y-14;         /* 显示标尺单位 */
        _x = x1+20;
        draw_font(_x, _y, pick_string("(毫米)", "(mm)"), color, preview->font, 0);
        
        _y = y1-22;        /* 订单信息的区域 */
        _x = __x;
        gui_init_rect(oi_area, _x, _y, (__x1-x)-3, 19);
        
        x  = __x;  x1 = x1;
        y += 2;    y1 = _y-5;
        gui_init_rect(inner_rect, x, y, (x1-x)+1, (y1-y)+1);
        t->redraw = 1;
    } else {
        if(t->redraw){
            int  cuts, k_nr, l_nr, order_width;
            int *k_data, *l_data;
            int  half_width;
            t->redraw = 0;
            x  = inner_rect->x;
            y  = inner_rect->y;
            x1 = inner_rect->width + x;
            y1 = inner_rect->height + y;
            _x = t->order_info_area.x;
            _y = t->order_info_area.y;
            gdi_draw_box(x, y, x1, y1, bkcolor);
            gdi_draw_box(_x, _y, _x + t->order_info_area.width, _y + t->order_info_area.height, bkcolor);
            if(t->have_order){
                order = &t->order;
                /*********   显示订单信息   *********/
                make_order_info(order_info_buf, order);
                draw_font_ex(x, _y, (x1-x), 16, order_info_buf, t->order_info_color, bkcolor, t->order_info_font, DRAW_OPT_ALIGN_CENTER);
                /*********   计算完整的压线资料   *********/
                MEMSET(x_buf, 0, sizeof(x_buf));
                cuts = order->CUTS;
                if(cuts<0)
                    cuts = 1;
                strcat(x_buf, order->SPECCUT);
                for(i=1; i<cuts; i++){
                    #if 0
                    printf("i=%d, cuts=%d, strlen(x_buf)=%d\n", i, cuts, strlen(x_buf));
                    #endif
                    x_buf[strlen(x_buf)]=SLC_K_TOKEN;
                    strcat(x_buf, order->SPECCUT);
                    if(strlen(x_buf)+strlen(order->SPECCUT) >= 500){
                        draw_font_ex(x, y, (x1-x), (y1-y)-8, 
                                     pick_string("=== 压 线 资 料 太 长 ===", "=== Data is too long ==="), 
                                     color, bkcolor, font24, DRAW_OPT_ALIGN_CENTER);                        
                        return;
                    }
                }
                #if 0
                printf("x_buf=%s\n", x_buf);
                printf("x_buf-length = %d\n", strlen(x_buf));
                getch();
                #endif
                /*********   计算预计的刀线位置   *********/
                locate_flag = (config.slc_reverse_mode ? SLC_FLAG_RVSE : 0) | SLC_FLAG_TRIM;
                order_width = init_orderkl(&t->temp_slc, x_buf, &k_data, &k_nr, &l_data, &l_nr, locate_flag);
                if(k_nr < 2 || k_nr > 12){ /* 一般来说, 不会用到12把刀吧, -- Jun */
                    draw_font_ex(x, y, (x1-x), (y1-y)-8, 
                                 pick_string("=== 压 线 资 料 有 误 ===", "=== Data has some mistakes ==="), 
                                 color, bkcolor, font24, DRAW_OPT_ALIGN_CENTER);
                } else if(order_width < 0){
                    draw_font_ex(x, y, (x1-x), (y1-y)-8, 
                                 pick_string("=== 压 线 资 料 有 误 ===", "=== Data has some mistakes ==="), 
                                 color, bkcolor, font24, DRAW_OPT_ALIGN_CENTER);
                } else if(order_width > 30000){
                    draw_font_ex(x, y, (x1-x), (y1-y)-8, 
                                 pick_string("=== 订 单 宽 度 太 大 ===", "=== Width is too large ==="), 
                                 color, bkcolor, font24, DRAW_OPT_ALIGN_CENTER);
                } else if(order_width <= 1500){
                    draw_font_ex(x, y, (x1-x), (y1-y)-8, 
                                 pick_string("=== 订 单 宽 度 太 小 ===", "=== Width is too small ==="), 
                                 color, bkcolor, font24, DRAW_OPT_ALIGN_CENTER);
                }
                /*********   显示预览图形   *********/
                else {
                    int  __last = k_data[0];
                    #define __mm_to_x(mm) (int)(((long)half_width*(long)(mm))/(5L*PREVIEW_MAX_WIDTH))
                    half_width = inner_rect->width/2;
                    y += 5;
                    x += half_width;       /* x指向中线 */
                    _y = y+21;             /* 纸板图象的顶坐标  */
                    _y1 = y1-29;           /* 纸板图象的底坐标  */
                    for(i=0; i<k_nr; i++){ /* k_nr意为本笔单所用刀数 */
                        _x = x + __mm_to_x(k_data[i]);
                        gdi_draw_v_line(_x-1, y+1, y+18, t->preview_marker_color);
                        gdi_draw_v_line(_x, y, y+19, t->preview_marker_color);
                        gdi_draw_v_line(_x+1, y+1, y+18, t->preview_marker_color);
                        if(i>0){
                            ___draw_width_mark(__x, _x, y, 20, (k_data[i]-__last), 
                                               1, t->preview_marker_color, bkcolor);
                            gdi_draw_box(__x+2, _y, _x-2, _y1, t->preview_paper_color);
                        }
                        __x = _x; /* __x记录了_x上一次的值 */
                        __last = k_data[i];
                    }
                    for(i=0; i<l_nr; i++){ /* l_nr意为本笔单所用线数 */
                        _x = x + __mm_to_x(l_data[i]);
                        draw_vertical_dashed_line(_x, _y, _y1, color, 0xF0);
                        draw_vertical_dashed_line(_x+1, _y, _y1, color, 0xF0);
                    }
                    _y1 += 2;
                    _x = x + __mm_to_x(k_data[0]);
                    gdi_draw_v_line(_x-1, _y1+1, _y1+18, t->preview_marker_color);
                    gdi_draw_v_line(_x, _y1, _y1+19, t->preview_marker_color);
                    gdi_draw_v_line(_x+1, _y1+1, _y1+18, t->preview_marker_color);
                    __last = _x;
                    _x = x + __mm_to_x(k_data[k_nr-1]);
                    gdi_draw_v_line(_x-1, _y1+1, _y1+18, t->preview_marker_color);
                    gdi_draw_v_line(_x, _y1, _y1+19, t->preview_marker_color);
                    gdi_draw_v_line(_x+1, _y1+1, _y1+18, t->preview_marker_color);
                    ___draw_width_mark(__last, _x, _y1, 20, order_width, 
                                       t->preview_marker_font, t->preview_marker_color, bkcolor);
                }
            } else { /* if(t->have_order) */
                draw_font_ex(x, y, (x1-x), (y1-y)-8, 
                             pick_string("=== 无 订 单 预 览 ===", "=== No Order to Preview ==="), 
                             color, bkcolor, font24, DRAW_OPT_ALIGN_CENTER);
            }
        }
    }

    __y = __y;      /* 减少编译器警告 */
    h = h;
    _x1 = 0;
    _x1 = _x1;

    return;
}

#else /* CONFIG_PREVIEW_WIDER */

void __widget_preview_draw(gui_widget * preview)
{
/*lint --e{534}*/
    int x, y, x1, y1, w, h, move;
    COLOR bkcolor, color;
    gui_preview_private * t;
    order_struct * order;
    int i, _x, _y, _x1, _y1;
    int __x, __y, __x1;
    INT16U locate_flag;
    RECT * inner_rect, * oi_area;
    char order_info_buf[256], x_buf[2048];

    #define PREVIEW_MAX_WIDTH 3000  /* 预览支持的最大纸宽是3米 */

    FamesAssert(preview);

    if(!preview)
        return;

    t = (gui_preview_private *)preview->private_data;
    if(!t)
        return;
    
    inner_rect = &preview->inner_rect;
    oi_area    = &t->order_info_area;
    color = preview->color;
    bkcolor = preview->bkcolor;
    if(bkcolor==0)
        bkcolor = WIDGET_BKCOLOR;
    
    if(preview->flag & GUI_WIDGET_FLAG_REFRESH){
        x  = preview->real_rect.x;
        y  = preview->real_rect.y;
        x1 = preview->real_rect.width + x;
        y1 = preview->real_rect.height + y;
        if(preview->style & PREVIEW_STYLE_MODAL_FRAME){
            move = gui_widget_draw_modal_frame(x, y, x1, y1);
            ___gui_widget_xy_move(move);
        } 
        if(preview->style & PREVIEW_STYLE_CLIENT_BDR){
            move = gui_widget_draw_client_bdr(x, y, x1, y1);
            ___gui_widget_xy_move(move);
        } 
        if(preview->style & PREVIEW_STYLE_BORDER){
            move = gui_widget_draw_static_bdr(x, y, x1, y1);
            ___gui_widget_xy_move(move);
        } 
        if(preview->style & PREVIEW_STYLE_SUBSIDE){
            move = gui_widget_draw_subside_bdr(x, y, x1, y1);
            ___gui_widget_xy_move(move);
        } 
        if(!gdi_draw_box(x, y, x1, y1, bkcolor)){
            ;
        }
        __x1 = x1;         /* 右端边界位置 */
        w  = (x1-x)+1;
        h  = (y1-y)+1;
        w -= 150;          /* 靠左点 */
        if(w < 750)
            w = 750;
        x += (w-750)/2;
        x1 = x+749;
        __x = x;           /* 记住起始位置 */
        __y = y;

        y += 6;
        if(1){             /* 显示标尺开始 */
            char *__s[] = { "-1500", "-1000", "-500", "0", "500", "1000", "1500" };
            x -= 22;
            for(i=0; i<7; i++){
                draw_font_ex(x, y, 50, 8, __s[i], color, bkcolor, preview->font, DRAW_OPT_ALIGN_CENTER);
                x += 125;
            }
        }
        x  = __x;
        y += 16;           /* 画标尺的小竖线 */  
        _y1 = y+8;
        for(i=0; i<=30; i++){
            if(i%5)
                _y = y+3;
            else 
                _y = y;
            gdi_draw_v_line(x, _y, _y1, color);
            x += 25;
        }
        x = __x;
        y = _y1+1;
        gdi_draw_h_line(x, y, x1+1, color);
        
        _y = y-14;         /* 显示标尺单位 */
        _x = x1+40;
        draw_font(_x, _y, pick_string("标尺单位: 毫米", "Ruler Unit: mm"), color, preview->font, 0);
        
        _y = y1-22;        /* 订单信息的区域 */
        _x = __x;
        gui_init_rect(oi_area, _x, _y, (__x1-x)-3, 19);
        
        x  = __x;  x1 = x1;
        y += 2;    y1 = _y-5;
        gui_init_rect(inner_rect, x, y, (x1-x)+1, (y1-y)+1);
        t->redraw = 1;
    } else {
        if(t->redraw){
            int  cuts, k_nr, l_nr, order_width;
            int *k_data, *l_data;
            int  half_width;
            t->redraw = 0;
            x  = inner_rect->x;
            y  = inner_rect->y;
            x1 = inner_rect->width + x;
            y1 = inner_rect->height + y;
            _x = t->order_info_area.x;
            _y = t->order_info_area.y;
            gdi_draw_box(x, y, x1, y1, bkcolor);
            gdi_draw_box(_x, _y, _x + t->order_info_area.width, _y + t->order_info_area.height, bkcolor);
            if(t->have_order){
                order = &t->order;
                /*********   显示订单信息   *********/
                make_order_info(order_info_buf, order);
                draw_font_ex(x, _y, (x1-x), 16, order_info_buf, t->order_info_color, bkcolor, t->order_info_font, DRAW_OPT_ALIGN_CENTER);
                /*********   计算完整的压线资料   *********/
                MEMSET(x_buf, 0, sizeof(x_buf));
                cuts = order->CUTS;
                if(cuts<0)
                    cuts = 1;
                strcat(x_buf, order->SPECCUT);
                for(i=1; i<cuts; i++){
                    #if 0
                    printf("i=%d, cuts=%d, strlen(x_buf)=%d\n", i, cuts, strlen(x_buf));
                    #endif
                    x_buf[strlen(x_buf)]=SLC_K_TOKEN;
                    strcat(x_buf, order->SPECCUT);
                    if(strlen(x_buf)+strlen(order->SPECCUT) >= 500){
                        draw_font_ex(x, y, (x1-x), (y1-y)-8, 
                                     pick_string("=== 压 线 资 料 太 长 ===", "=== Data is too long ==="), 
                                     color, bkcolor, font24, DRAW_OPT_ALIGN_CENTER);                        
                        return;
                    }
                }
                #if 0
                printf("x_buf=%s\n", x_buf);
                printf("x_buf-length = %d\n", strlen(x_buf));
                getch();
                #endif
                /*********   计算预计的刀线位置   *********/
                locate_flag = (config.slc_reverse_mode ? SLC_FLAG_RVSE : 0) | SLC_FLAG_TRIM;
                order_width = init_orderkl(&t->temp_slc, x_buf, &k_data, &k_nr, &l_data, &l_nr, locate_flag);
                if(k_nr < 2 || k_nr > 12){ /* 一般来说, 不会用到12把刀吧, -- Jun */
                    draw_font_ex(x, y, (x1-x), (y1-y)-8, 
                                 pick_string("=== 压 线 资 料 有 误 ===", "=== Data has some mistakes ==="), 
                                 color, bkcolor, font24, DRAW_OPT_ALIGN_CENTER);
                } else if(order_width < 0){
                    draw_font_ex(x, y, (x1-x), (y1-y)-8, 
                                 pick_string("=== 压 线 资 料 有 误 ===", "=== Data has some mistakes ==="), 
                                 color, bkcolor, font24, DRAW_OPT_ALIGN_CENTER);
                } else if(order_width > 30000){
                    draw_font_ex(x, y, (x1-x), (y1-y)-8, 
                                 pick_string("=== 订 单 宽 度 太 大 ===", "=== Width is too large ==="), 
                                 color, bkcolor, font24, DRAW_OPT_ALIGN_CENTER);
                } else if(order_width <= 1500){
                    draw_font_ex(x, y, (x1-x), (y1-y)-8, 
                                 pick_string("=== 订 单 宽 度 太 小 ===", "=== Width is too small ==="), 
                                 color, bkcolor, font24, DRAW_OPT_ALIGN_CENTER);
                }
                /*********   显示预览图形   *********/
                else {
                    int  __last = k_data[0];
                    #define __mm_to_x(mm) (int)(((long)half_width*(long)(mm))/(5L*PREVIEW_MAX_WIDTH))
                    half_width = inner_rect->width/2;
                    y += 5;
                    x += half_width;       /* x指向中线 */
                    _y = y+21;             /* 纸板图象的顶坐标  */
                    _y1 = y1-29;           /* 纸板图象的底坐标  */
                    for(i=0; i<k_nr; i++){ /* k_nr意为本笔单所用刀数 */
                        _x = x + __mm_to_x(k_data[i]);
                        gdi_draw_v_line(_x, y, y+19, t->preview_marker_color);
                        if(i>0){
                            ___draw_width_mark(__x, _x, y, 20, (k_data[i]-__last), 
                                               1, t->preview_marker_color, bkcolor);
                            gdi_draw_box(__x+1, _y, _x-1, _y1, t->preview_paper_color);
                        }
                        __x = _x; /* __x记录了_x上一次的值 */
                        __last = k_data[i];
                    }
                    for(i=0; i<l_nr; i++){ /* l_nr意为本笔单所用刀数 */
                        _x = x + __mm_to_x(l_data[i]);
                        draw_vertical_dashed_line(_x, _y, _y1, color, 0xF0);
                    }
                    _y1 += 2;
                    _x = x + __mm_to_x(k_data[0]);
                    gdi_draw_v_line(_x, _y1, _y1+19, t->preview_marker_color);
                    __last = _x;
                    _x = x + __mm_to_x(k_data[k_nr-1]);
                    gdi_draw_v_line(_x, _y1, _y1+19, t->preview_marker_color);
                    ___draw_width_mark(__last, _x, _y1, 20, order_width, 
                                       t->preview_marker_font, t->preview_marker_color, bkcolor);
                }
            } else { /* if(t->have_order) */
                draw_font_ex(x, y, (x1-x), (y1-y)-8, 
                             pick_string("=== 无 订 单 预 览 ===", "=== No Order to Preview ==="), 
                             color, bkcolor, font24, DRAW_OPT_ALIGN_CENTER);
            }
        }
    }

    __y = __y;      /* 减少编译器警告 */
    h = h;
    _x1 = 0;
    _x1 = _x1;

    return;
}

#endif /* CONFIG_PREVIEW_WIDER */

/*-----------------------------------------------------------------------------------------
 * 函数:    preview_initialize()
 *
 * 描述:    订单预览控件的注册
**---------------------------------------------------------------------------------------*/
int preview_initialize(void)
{
    int id;
    gui_widget_type __widget_type_preview;
    
    __widget_type_preview.draw = __widget_preview_draw;

    id = gui_register_usr_widget(&__widget_type_preview);

    lock_kernel();
    __widget_id_preview = id;
    unlock_kernel();

    return id;
}


/*=========================================================================================
 * 
 * 本文件结束: slc/preview.c
 * 
**=======================================================================================*/


