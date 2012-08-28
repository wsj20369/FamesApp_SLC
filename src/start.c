/*************************************************************************************
 * 文件:    slc/start.c
 *
 * 说明:    分压机起始程序
 *
 * 作者:    Jun
 *
 * 时间:    2011-2-19
*************************************************************************************/
#define  SLC_START_C
#include <FamesOS.h>
#include "common.h"

/*------------------------------------------------------------------------------------
 * 函数:    startup_init()
 *
 * 描述:    启始初始化, 用于打开系统相关的一些资源
**----------------------------------------------------------------------------------*/
void startup_init(void)
{
    InitPLCService();
    OpenConsole();
    if (!slc_initialize()) {
        sys_print("Fatal Error: SLC-CORE Init Failed!!!\n");
        ExitApplication();
    }
    if (register_service_init() == fail) {
        sys_print("Fatal Error: Register Service Init Failed!!!\n");
        ExitApplication();
    }
}

/*------------------------------------------------------------------------------------
 * 函数:    sk_refresh_screen()
 *
 * 描述:    刷新屏幕
**----------------------------------------------------------------------------------*/
void sk_refresh_screen(void)
{
    gui_widget * root;
    
    root = gui_get_root_widget();
    if(root)
        gui_refresh_widget(root);
    gui_put_root_widget();
}

/*------------------------------------------------------------------------------------
 * 函数:    start()
 *
 * 描述:    SLC启始任务, 初始化并提供用户对SLC的控制
**----------------------------------------------------------------------------------*/
void __task start(void * data)
{
    char  ___s[64];
    extern struct slc_config_s config;

    data = data;

    #if 0
    lock_kernel();
    {
        struct date today;
        getdate(&today);
        if(today.da_year != 2011 || (today.da_year == 2011 && today.da_mon >= 9))
            Register();
    }
    unlock_kernel();
    #endif

    startup_init();

    RegisterSpecialKey(INSERT, sk_refresh_screen); /* INSERT: 刷新当前屏幕 */

    read_config();
    read_tmp_config();
    language_initialize(config.language);

    read_yx_dat();
    read_lb_dat();
    InitOrderEnv();
    init_cim_link();
    slc_control_service_initialize();
    m1_plc_initialize();
    m2_plc_initialize();
    slc_open_plc();

    slc_init_gui();

    #if 0
    welcome_start();
    ___s[0] = 0;
    load_string(___s, sizeof(___s), "welcome_started");
    startup_message(___s);
    welcome_ended();
    #endif

    active_main_screen(); /* 显示主画面 */

    start_main_loop();    /* 启动主循环, 此函数永不返回 */
}

/*------------------------------------------------------------------------------------
 * 函数: TaskSwitchHook()
 *
 * 说明: 任务切换钩子
 *
 * 特别: 此函数运行在中断服务程序之中,应特别注意其执行效率
**----------------------------------------------------------------------------------*/
void apical TaskSwitchHook(void)
{
}

void quit(void)
{
    ExitApplication();
}

long get_free_mem(void)
{
    long mem;

    lock_kernel();
    mem = (long)coreleft();
    unlock_kernel();

    return mem;
}

/*------------------------------------------------------------------------------------
 *  取系统的时间日期
**----------------------------------------------------------------------------------*/
void GetDateTime (INT08S *s)
{
    struct time now;
    struct date today;

    lock_kernel();
    gettime(&now);
    getdate(&today);
    unlock_kernel();
    sprintf(s, "%02d-%02d-%02d  %02d:%02d:%02d",
            today.da_mon,
            today.da_day,
            today.da_year,
            now.ti_hour,
            now.ti_min,
            now.ti_sec);
}


/*====================================================================================
 * 
 * 本文件结束: slc/start.c
 * 
**==================================================================================*/

