/*************************************************************************************
 * �ļ�:    slc/start.c
 *
 * ˵��:    ��ѹ����ʼ����
 *
 * ����:    Jun
 *
 * ʱ��:    2011-2-19
*************************************************************************************/
#define  SLC_START_C
#include <FamesOS.h>
#include "common.h"

/*------------------------------------------------------------------------------------
 * ����:    startup_init()
 *
 * ����:    ��ʼ��ʼ��, ���ڴ�ϵͳ��ص�һЩ��Դ
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
 * ����:    sk_refresh_screen()
 *
 * ����:    ˢ����Ļ
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
 * ����:    start()
 *
 * ����:    SLC��ʼ����, ��ʼ�����ṩ�û���SLC�Ŀ���
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

    RegisterSpecialKey(INSERT, sk_refresh_screen); /* INSERT: ˢ�µ�ǰ��Ļ */

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

    active_main_screen(); /* ��ʾ������ */

    start_main_loop();    /* ������ѭ��, �˺����������� */
}

/*------------------------------------------------------------------------------------
 * ����: TaskSwitchHook()
 *
 * ˵��: �����л�����
 *
 * �ر�: �˺����������жϷ������֮��,Ӧ�ر�ע����ִ��Ч��
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
 *  ȡϵͳ��ʱ������
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
 * ���ļ�����: slc/start.c
 * 
**==================================================================================*/

