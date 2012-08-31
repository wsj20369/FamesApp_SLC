/******************************************************************************************
 * 文件:    slc/make_sn.c
 *
 * 描述:    SLC注册机(SN生成器)
 *
 * 作者:    Jun
 *
 * 创建:    2012-8-31
******************************************************************************************/
#define THIS_IS_UNREGISTER
#include "..\src\reg_core.c"

void __task start(void * data)
{
    char user_sn[32];
    char mach_id[32];
    char user_id[32], saved_id[32];

    data = data;
    OpenConsole();

    lock_kernel();

    printf("\nInput User ID: ");
    scanf("%s", user_id);
    memcpy(saved_id, user_id, sizeof(saved_id));

    printf("\n---------------------------------------\n\n");
    if (decrypt_machine_id(mach_id, user_id) == 0) {
        printf("ID is invalid: %s\n", saved_id);
    } else {
        generate_register_sn(user_sn, mach_id);
        printf("User ID: %s\n", saved_id);
        printf("Mach ID: %s\n", mach_id);
        printf("User SN: %s\n", user_sn);
    }
    printf("\n---------------------------------------\n\n");

    unlock_kernel();

    waitkey(10000);

    ExitApplication();
}

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

/*=========================================================================================
 * 
 * 本文件结束: slc/make_sn.c
 * 
**=======================================================================================*/


