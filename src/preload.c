/**************************************************************************************
 * 文件:    slc/preload.c
 *
 * 说明:    预先加载一些相关文件
 *
 * 作者:    Jun
 *
 * 时间:    2011-3-3
**************************************************************************************/
#define  SLC_PRELOAD_C
#include <includes.h>
#include "common.h"


static FONTINFO font_asc_16 = {NO, FONT_TYPE_ASCII,   0,  8, 16, 1,  0,  "ASC16",     NULL};
static FONTINFO font_hz_16  = {NO, FONT_TYPE_GB2312,  0, 16, 16, 2,  0,  "HZK16",     NULL};
static FONTINFO font_asc_48 = {NO, FONT_TYPE_ASCII,   0, 24, 48, 3, 32,  "ASC48",     NULL};
static FONTINFO font_asc_24 = {NO, FONT_TYPE_ASCII,   0, 12, 24, 2,  0,  "ASC24",     NULL};
static FONTINFO font_hz_24  = {NO, FONT_TYPE_GB2312,  0, 24, 24, 3, 15*94 , "HZK24",  NULL};
static FONTINFO font_hz_20s = {NO, FONT_TYPE_GB2312,  0, 20, 20, 3,  0 , "HZK20S",    NULL};
static FONTINFO font_monitr = {NO, FONT_TYPE_ASCII,   0,  8, 19, 1,  0 , "ASC-MNTR",  NULL};

int font16, font48, font24, font20, font_mntr;

void load_fonts(void) 
{
    load_font(&font_asc_16);
    load_font(&font_hz_16);
    load_font(&font_asc_48);
    load_font(&font_hz_24);
    load_font(&font_asc_24);
    load_font(&font_hz_20s);
    load_font(&font_monitr);

    font16    = register_font(&font_hz_16, &font_asc_16);
    font48    = register_font(NULL, &font_asc_48);
    font24    = register_font(&font_hz_24, &font_asc_24);
    font20    = register_font(&font_hz_20s, NULL);
    font_mntr = register_font(NULL, &font_monitr);
}

BMPINFO welcome_picture, icon, company_bmp;
static char welcome_filename[64]  = "welcome.bmp";
static char icon_filename[64]     = "icon.bmp";
static char company_bmp_fname[64] = "company.bmp";

BMPINFO machine_name[2];
static char * m1_bmp_filename   = "m1.bmp";
static char * m2_bmp_filename   = "m2.bmp";

void load_bmps(void)
{
    load_string(welcome_filename,  sizeof(welcome_filename),  "welcome_picture");
    load_string(icon_filename,     sizeof(icon_filename),     "icon_picture");
    load_string(company_bmp_fname, sizeof(company_bmp_fname), "company_picture");

    InitBMPINFO(&icon);
    LoadBmp(&icon, icon_filename);
    
    InitBMPINFO(&company_bmp);
    LoadBmp(&company_bmp, company_bmp_fname);

    #if 0
    /* 为了节省内存, 不要加载这个图片了 */
    InitBMPINFO(&welcome_picture);
    LoadBmp(&welcome_picture, welcome_filename);
    #endif

    InitBMPINFO(&machine_name[0]);
    LoadBmp(&machine_name[0], m1_bmp_filename);

    InitBMPINFO(&machine_name[1]);
    LoadBmp(&machine_name[1], m2_bmp_filename);
}

void early_loads(void)
{
    load_fonts();
    load_bmps();
}

/*=====================================================================================
 * 
 * 本文件结束: slc/preload.c
 * 
**===================================================================================*/

