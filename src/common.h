/*************************************************************************************
 * 文件:    slc/common.h
 *
 * 作者:    Jun
 *
 * 时间:    2010-12-17
*************************************************************************************/
#ifndef FAMES_SLC_COMMON_H
#define FAMES_SLC_COMMON_H

#include <string.h>

#include "cfg_defs.h"
#include "reg_core.h"
#include "slc_reg.h"
#include "passwd.h"
#include "yes_no.h"
#include "order.h"
#include "slc_core.h"
#include "yx_dat.h"
#include "config.h"
#include "tmp_cfg.h"
#include "keycmd.h"
#include "preview.h"
#include "ord_edit.h"
#include "slc_mntr.h"
#include "slc_para.h"
#include "slc_whet.h"
#include "slc_ctrl.h"
#include "slc_lb.h"
#include "cim_link.h"
#include "language.h"

/*lint -e806*/

void quit(void);

long get_free_mem(void);

BOOL m1_start_plc(void);
BOOL m1_stop_plc(void);
void m1_plc_initialize(void);
BOOL m2_start_plc(void);
BOOL m2_stop_plc(void);
void m2_plc_initialize(void);

void slc_open_plc(void);
void slc_close_plc(void);
void slc_init_gui(void);
void example_init_xms(void);
void early_loads(void);

void init_welcome_screen(void);
void welcome_start(void);
void welcome_ended(void);
void startup_message(INT08S * s);

void show_palette_init(void);

void start_main_loop(void);

void init_main_screen(void);
void active_main_screen(void);
void message(INT08S * s);
void set_buttons_caption(INT08S *captions[]);

void init_send_screen(void);

extern int font16, font48, font24, font20, font_mntr;

extern struct slc_config_s config;

INT16U get_version(void);
STRING get_version_string(void);

gui_widget * init_malfunction_screen(void);
void         slc_malfunction_monitor(void);
void         slc_malfunction_get_comment(char * buf, int buf_len, int malf_code);

gui_widget * init_plc_monitor_screen(void);
void         plc_monitor(void);
gui_widget * init_plc_io_monitor_screen(void);
void         plc_io_monitor(void);

struct bits16{
    int b0:1;
    int b1:1;
    int b2:1;
    int b3:1;
    int b4:1;
    int b5:1;
    int b6:1;
    int b7:1;
    int b8:1;
    int b9:1;
    int b10:1;
    int b11:1;
    int b12:1;
    int b13:1;
    int b14:1;
    int b15:1;
};

struct bits32{
    int b0:1;
    int b1:1;
    int b2:1;
    int b3:1;
    int b4:1;
    int b5:1;
    int b6:1;
    int b7:1;
    int b8:1;
    int b9:1;
    int b10:1;
    int b11:1;
    int b12:1;
    int b13:1;
    int b14:1;
    int b15:1;
    int b16:1;
    int b17:1;
    int b18:1;
    int b19:1;
    int b20:1;
    int b22:1;
    int b23:1;
    int b24:1;
    int b25:1;
    int b26:1;
    int b27:1;
    int b28:1;
    int b29:1;
    int b30:1;
    int b31:1;
};

#include "plc_def.h"


#define COLOR_SLC_RUNNING  198 /* 标志分压机正在运行的颜色 */

#define COLOR_CIM_LINK_RX  199 /* 又一次接到了生管的订单   */



#endif /* #ifndef FAMES_SLC_COMMON_H */

/*====================================================================================
 * 
 * 本文件结束: slc/common.h
 * 
**==================================================================================*/

