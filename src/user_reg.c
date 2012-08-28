/*************************************************************************************
 * 文件:    slc/user_reg.c
 *
 * 描述:    用户注册
 *
 * 作者:    Jun
 *
 * 时间:    2011-4-8
 *
 * 说明:    硬盘注册(只能检测第1个IDE设备(IDE0))
*************************************************************************************/
#define  SLC_USER_REGISTER_C
#include <includes.h>
#include <string.h>
#include "common.h"


char *getascii (unsigned int in_data [], int off_start, int off_end)
{
    static char ret_val [255];
    int loop, loop1;

    for (loop = off_start, loop1 = 0; loop <= off_end; loop++)
    {
        ret_val [loop1++] = (char) (in_data [loop] / 256); /* Get High byte */
        ret_val [loop1++] = (char) (in_data [loop] % 256); /* Get Low byte */
    }
    ret_val [loop1] = '\0'; /* Make sure it ends in a NULL character */
    return (ret_val);
}
 
void My_pass(char *string)
{
    int i;
    for(i=0; string[i]; i++){
        string[i]= (( string[i] << ((i % 3 )+ 1)) + ( string[i+1] >> ( 2 - i%2))) % 10 + 0x30;
    }
}

void Register(void)
{
    char hdstring[40];
    char temp_hdstring[40];
    unsigned int dd [256]; /* DiskData */
    unsigned int dd_off; /* DiskData offset */
    int  length, i;
    char Register_Code[40];
    FILE * fp;
    char read_buffer[]="                                        ";
    long wait;

    fp = fopen("__reg.dat", "r");
    if( fp ){
        fscanf(fp, "%s",  read_buffer);
        fclose(fp);     
    }
    wait = 2000L;
    while (inportb(0x1F7) != 0x50){ /* Wait for controller not busy */
        wait--;
        if(wait <= 0L)
            break;
    }
    outportb(0x1F6, 0xA0); /* Get first/second drive */
    outportb(0x1F7, 0xEC); /* Get drive info data */
    wait = 2000L;
    while (inportb(0x1F7) != 0x58){ /* Wait for data ready */
        wait--;
        if(wait <= 0L)
            break;
    }
    for (dd_off = 0; dd_off != 256; dd_off++) /* Read "sector" */
        dd [dd_off] = inport(0x1F0);
    strcpy(hdstring,getascii(dd,10,19));
    length = strlen( hdstring );
    for( i=0; i < length; i++)
    hdstring[i] = hdstring[i]%10 + 0x30;
    length = strlen(hdstring);
    for( i=0; i < (length / 2 -1 ); i++)
    {
        hdstring[i] = ((hdstring [i] - 0x30 ) * ( hdstring [length - i -1] - 0x30 )) / 10 + 0x30;
    }
    hdstring[i] = 0x0;
    strcpy( temp_hdstring, hdstring );
    My_pass ( hdstring );
    if( (!strcmp( hdstring, read_buffer )))return;
    printf("\nUser ID: %s \n", temp_hdstring );
    printf("Please Input Register Code: ");
    scanf("%s", Register_Code);
    if( ! strcmp(Register_Code,  hdstring) ){
    fp = fopen ("__reg.dat", "w");
    if( fp ){
        fprintf(fp, "%s", hdstring);
        fclose(fp);
        }
        return ;
    } else {
        printf("\nBad Register Code!!! I\'m sorry!\n");
        getch();
        ExitApplication();
    }
}


/*====================================================================================
 * 
 * 本文件结束: slc/user_reg.c
 * 
**==================================================================================*/

