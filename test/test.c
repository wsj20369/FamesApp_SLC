/**************************************************************************************
 * 文件:    slc/test.c
 *
 * 说明:    SLC算法测试
 *
 * 作者:    Jun
 *
 * 时间:    2011-1-28
**************************************************************************************/
#define  SLC_TEST
#include "type.h"
#include "core.h"
#include <string.h>

slc_descriptor_t slc1;
#if USE_FAMESOS_TYPE == 0
INT32U SecondsFromStart = 0L;
#endif

char buf[512]={0};
char bak[512]={0};
char xxx[512]={0};
INT16U flag=SLC_FLAG_TRIM|SLC_FLAG_ASTD;
INT32U ms=0L;
INT32U err = 0L;
int    cuts = 1;

struct __batch_test_s {
    int  max_cuts;     /* 最大剖数 */
    char base_str[64]; /* 基本压线 */
};

struct __batch_test_s batch_test_list[] = {
        {7,"100+200+100"},
        {2,"1100"},
        {1,"160*1800"},
        {1,"1800*160*160"},
        {4,"200+200+300"},
        {0,""}/**END**/
};

void batch_test(void)
{
    int i, j, v, mc;
    char * str;
    FILE *out;
    prepare_atomic()

    i=j=mc=v=0;

    out = fopen("123.txt", "a");

    if(!out)
        out = stdout;

    printf("\n=========== batch-test start ===========\n");
    fprintf(out, "\n=========== batch-test start ===========\n");
    while(batch_test_list[v].max_cuts!=0){
        mc  = batch_test_list[v].max_cuts;
        str = batch_test_list[v].base_str;
        for(j=1; j<=mc; j++){
            fprintf(out, "data = %d(%s) => ", j, str);
            printf("data = %d(%s) => ", j, str);
          start_locate:
            memset(xxx, 0, sizeof(xxx));
            strcat(xxx, str);
            for(i=1; i<j; i++){
                xxx[strlen(xxx)]=SLC_K_TOKEN;
                strcat(xxx, str);
                if(strlen(xxx)+strlen(str) >= 500)
                    break;
            }
            fprintf(out, "locate: %s\n", xxx);
            printf("locate: %s\n", xxx);
            in_atomic();
            ms = SecondsFromStart;
            out_atomic();
            err=slc_locate(&slc1, xxx, flag);
            in_atomic();
            ms = SecondsFromStart - ms;
            out_atomic();
            fprintf(out, "OK, ms=%ld, err=%08lX, ", ms, err);
            fprintf(out, "K=%d, L=%d, width=%d\n", slc1.k_number, slc1.l_number, 
                                               slc1.order_kl.width/10);
            if(err != SLC_ERR_NONE){
                fprintf(out, " --- Error Occured: %08lX --- ", err);
                goto ended_locate;
            }
            fprintf(out, "RESULT ->\n");
            fprintf(out, " ");
            for(i=0; i<slc1.k_number; i++){
                if(slc1.kl_set.k_selected[i])
                    fprintf(out, "[   K%d   ]", i);
                else 
                    fprintf(out, "----------");
            }
            fprintf(out, "\n");
            fprintf(out, " ");
            for(i=0; i<slc1.k_number; i++){
                if(slc1.kl_set.k_selected[i])
                    textcolor(4);
                else 
                    textcolor(2);
                fprintf(out, "  %+-5d   ", slc1.kl_set.k_location[i]);
            }
            fprintf(out, "\n");
            for(i=0; i<slc1.l_number; i++){
                if(slc1.kl_set.l_selected[i])
                    fprintf(out, "[ L%-2d]", i);
                else 
                    fprintf(out, "------");
            }
            fprintf(out, "\n");
            for(i=0; i<slc1.l_number; i++){
                if(slc1.kl_set.l_selected[i])
                    textcolor(4);
                else 
                    textcolor(2);
                fprintf(out, " %+-5d", slc1.kl_set.l_location[i]);
            }
            textcolor(7);
          ended_locate: 
            fprintf(out, "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
            fprintf(out, "\n");
            fprintf(out, "\n");
        }
        v++;
    }
    printf("=========== batch-test ended ===========\n");
    fprintf(out, "=========== batch-test ended ===========\n");
    if(out!=stdout)
        fclose(out);
}

void help_msg(int msg)
{
    if(msg == 0){
        printf("\n"
               "    Command: \n\n"
               "    Q:   Quit               X:   DisableK\n"
               "    ?:   Help               Z:   DisableL\n"
               "    S:   Single Type        [:   K-LeftLimit\n"
               "    D:   Double Type        ]:   K-RightLimit\n"
               "    T:   TrimKnife          {:   L-LeftLimit\n"
               "    P:   PrePress           }:   L-RightLimit\n"
               "    R:   Redo               !:   K-Distance\n"
               "    C:   Set Cuts           @:   L-Distance\n"
               "    K:   Set Knife          #:   Automake K/L limit\n"
               "    L:   Set Line           $:   Display K/L limit\n"
               "    W:   Set Hw-Width       A:   Auto Batch test\n"
               "    Tab: Show status\n");
    }
    if(msg == 1){
        printf("\n    Status: \n\n");
        printf("    trim   : %s\n", (flag&SLC_FLAG_TRIM)?"YES":"NO");
        printf("    prep   : %s\n", (flag&SLC_FLAG_PREP)?"YES":"NO");
        printf("    slc    : %s\n", (slc1.slc_type==SLC_TYPE_DOUBLE)?"double":"single");
        printf("    k/l    : %d/%d\n", slc1.k_number, slc1.l_number);
        printf("    lasterr: %08lX\n", err);
        printf("    last ms: %ld\n", ms);
        printf("    lastbuf: %s(cuts=%d)\n", bak, cuts);
    }
}

void make_kl_limit(void)
{
    int k,l,width,i;
    slc_descriptor_t * slc;

    slc = &slc1;
    k=slc->k_number;
    l=slc->l_number;
    width=slc->hw_width;

    printf("Generate K/L Limit(Left/Right):\n");
    
    printf("\nwidth=%d, k=%d, l=%d\n", width, k, l);
    for(i=0; i<k; i++){
        slc->k_lmt_left[i] = -(width/2);
        slc->k_lmt_right[i] = (width/2);
    }
    for(i=0; i<l; i++){
        slc->l_lmt_left[i] = -(width/2);
        slc->l_lmt_right[i] = (width/2);
    }
}

void disp_kl_limit(void)
{
    int k,l,width,i;
    slc_descriptor_t * slc;

    slc = &slc1;
    k=slc->k_number;
    l=slc->l_number;
    width=slc->hw_width;

    printf("K/L Limit(Left/Right):\n");
    
    printf("\nwidth=%d, k=%d, l=%d\n", width, k, l);
    for(i=0; i<k; i++){
            printf("[   K%d   ]", i);
    }
    printf("\n");
    printf(" ");
    for(i=0; i<k; i++){
        printf(" %+-7d  ", slc->k_lmt_left[i]);
    }
    printf("\n");
    printf(" ");
    for(i=0; i<k; i++){
        printf(" %+-7d  ", slc->k_lmt_right[i]);
    }
    printf("\n\n");
    for(i=0; i<l; i++){
            printf("[ L%-2d]", i);
    }
    printf("\n");
    for(i=0; i<l; i++){
        printf("%+-6d", slc->l_lmt_left[i]);
    }
    printf("\n");
    for(i=0; i<l; i++){
        printf("%+-6d", slc->l_lmt_right[i]);
    }
    printf("\n");
}

void slc_test_main(void)
{    
    char c;
    int  i,v,v2;
    char ts[64];
    prepare_atomic()
    
    slc_setup_to_default(&slc1);
   
    for(;;){
        textcolor(7);
        printf("\n> ");
        c = getch();
        switch(c){
            case '\x1b':
            case 'Q':
            case 'q':
                printf("Quit?(y/n) ");
                c = getch();
                if(c == 'Y' || c=='y')
                    return;
                if(c == 'Q' || c=='q')
                    return;
                break;
            case 'T':
            case 't':
                if(flag & SLC_FLAG_TRIM){ 
                    printf("Cancel K-TRIM");
                    flag &= ~SLC_FLAG_TRIM;
                } else {
                    printf("Enable K-TRIM");
                    flag |=  SLC_FLAG_TRIM;
                }
                break;
            case 's':
            case 'S':
                printf("Set slc-type to Single-Line!");
                slc1.slc_type = SLC_TYPE_SINGLE;
                break;
            case 'd':
            case 'D':
                printf("Set slc-type to Double-Line!");
                slc1.slc_type = SLC_TYPE_DOUBLE;
                break;
            case 'p':
            case 'P':                
                if(flag & SLC_FLAG_PREP){ 
                    printf("Cancel PrePress");
                    flag &= ~SLC_FLAG_PREP;
                } else {
                    printf("Enable PrePress");
                    flag |=  SLC_FLAG_PREP;
                }
                break;
            case '\xa':
            case '\xd':
                break;
            case '?':
            case 'h':
            case 'H':
                help_msg(0);
                break;
            case '\t':
                help_msg(1);
                break;
            case ' ':
                break;
            case 'r':
            case 'R':
                if(bak[0]>' '){
                    printf("Redo: %s, cuts=%d\n", buf, cuts);
                    strcpy(buf, bak);
                    goto start_locate;
                } else {
                    printf("bak is empty!");
                }
                break;
            case 'c':
            case 'C':
                printf("Now = %d, Input Cuts Number(1,%d): ", cuts, slc1.k_number);
                gets(ts);
                cuts = atoi(ts);
                if(cuts < 1)
                    cuts = 1;
                if(cuts > slc1.k_number)
                    cuts = slc1.k_number;
                printf("\n  New Cuts Number: %d\n", cuts);
                break;
            case 'k':
            case 'K':
                printf("Now = %d, Input K Number(3,%d): ", slc1.k_number, 9);
                gets(ts);
                v = atoi(ts);
                if(v < 3)
                    v = 3;
                if(v > 9)
                    v = 9;
                slc1.k_number = v;
                printf("\n  New K Number: %d\n", v);
                break;
            case 'l':
            case 'L':
                printf("Now = %d, Input L Number(4,%d): ", slc1.l_number, 16);
                gets(ts);
                v = atoi(ts);
                if(v < 4)
                    v = 4;
                if(v > 16)
                    v = 16;
                slc1.l_number = v;
                printf("\n  New L Number: %d\n", v);
                break;
            case 'w':
            case 'W':
                printf("Now = %d, Input HW-Width(1200,%d): ", slc1.hw_width/10, 3200);
                gets(ts);
                v = atoi(ts);
                if(v < 1200)
                    v = 1200;
                if(v > 3200)
                    v = 3200;
                slc1.hw_width= v*10;
                printf("\n  New HW-Width: %d\n", v);
                break;
            case 'x':
            case 'X':
                printf("Disable/Enable which K(0,%d): ", slc1.k_number-1);
                gets(ts);
                v = atoi(ts);
                if(v < 0)
                    v = 0;
                if(v > slc1.k_number-1)
                    v = slc1.k_number-1;
                if(slc1.k_disable[v]){
                    printf("  Enable K%d", v);
                    slc1.k_disable[v]=0;
                } else {
                    printf("  Disable K%d", v);
                    slc1.k_disable[v]=1;
                }
                break;
            case 'z':
            case 'Z':
                printf("Disable/Enable which L(0,%d): ", slc1.l_number-1);
                gets(ts);
                v = atoi(ts);
                if(v < 0)
                    v = 0;
                if(v > slc1.l_number-1)
                    v = slc1.l_number-1;
                if(slc1.l_disable[v]){
                    printf("  Enable L%d", v);
                    slc1.l_disable[v]=0;
                } else {
                    printf("  Disable L%d", v);
                    slc1.l_disable[v]=1;
                }
                break;
            case '[':
                printf("Set Left-Limit for which K(0,%d): ", slc1.k_number-1);
                gets(ts);
                v = atoi(ts);
                if(v < 0)
                    v = 0;
                if(v > slc1.k_number-1)
                    v = slc1.k_number-1;
                printf("\n  Now = %d, Set to: ", slc1.k_lmt_left[v]);
                gets(ts);
                v2 = atoi(ts);
                slc1.k_lmt_left[v] = v2;
                break;
            case ']':
                printf("Set Right-Limit for which K(0,%d): ", slc1.k_number-1);
                gets(ts);
                v = atoi(ts);
                if(v < 0)
                    v = 0;
                if(v > slc1.k_number-1)
                    v = slc1.k_number-1;
                printf("\n  Now = %d, Set to: ", slc1.k_lmt_right[v]);
                gets(ts);
                v2 = atoi(ts);
                slc1.k_lmt_right[v] = v2;
                break;
            case '{':
                printf("Set Left-Limit for which L(0,%d): ", slc1.l_number-1);
                gets(ts);
                v = atoi(ts);
                if(v < 0)
                    v = 0;
                if(v > slc1.l_number-1)
                    v = slc1.l_number-1;
                printf("\n  Now = %d, Set to: ", slc1.l_lmt_left[v]);
                gets(ts);
                v2 = atoi(ts);
                slc1.l_lmt_left[v] = v2;
                break;
            case '}':
                printf("Set Right-Limit for which L(0,%d): ", slc1.l_number-1);
                gets(ts);
                v = atoi(ts);
                if(v < 0)
                    v = 0;
                if(v > slc1.l_number-1)
                    v = slc1.l_number-1;
                printf("\n  Now = %d, Set to: ", slc1.l_lmt_right[v]);
                gets(ts);
                v2 = atoi(ts);
                slc1.l_lmt_right[v] = v2;
                break;
            case '!':
                printf("Set K-Distance for which K(0,%d): ", slc1.k_number-2);
                gets(ts);
                v = atoi(ts);
                if(v < 0)
                    v = 0;
                if(v > slc1.k_number-2)
                    v = slc1.k_number-2;
                printf("\n  Now = %d, Set to: ", slc1.k_distance[v]);
                gets(ts);
                v2 = atoi(ts);
                slc1.k_distance[v] = v2;
                printf("\n  Distance Between K%d - K%d is set to %d\n", v, v+1, v2);
                break;
            case '@':
                printf("Set L-Distance for which L(0,%d): ", slc1.k_number-2);
                gets(ts);
                v = atoi(ts);
                if(v < 0)
                    v = 0;
                if(v > slc1.l_number-2)
                    v = slc1.l_number-2;
                printf("\n  Now = %d, Set to: ", slc1.l_distance[v]);
                gets(ts);
                v2 = atoi(ts);
                slc1.l_distance[v] = v2;
                printf("\n  Distance Between L%d - L%d is set to %d\n", v, v+1, v2);
                break;
            case '#':
                make_kl_limit();
                break;
            case '$':
                disp_kl_limit();
                break;
            case 'a':
            case 'A':
                lock_kernel();
                batch_test();
                unlock_kernel();
                break;
            default:
                printf("\n  Unknown command: \'%c\', type ? to get help!", c);
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
                textcolor(7);
                putch(c);
                buf[0]=c;
                gets(&buf[1]);
              start_locate:
                memset(xxx, 0, sizeof(xxx));
                strcat(xxx, buf);
                for(i=1; i<cuts; i++){
                    xxx[strlen(xxx)]=SLC_K_TOKEN;
                    strcat(xxx, buf);
                    if(strlen(xxx)+strlen(buf) >= 500)
                        break;
                }
                printf("\nlocate: %s\n", xxx);
                in_atomic();
                ms = SecondsFromStart;
                out_atomic();
                err=slc_locate(&slc1, xxx, flag);
                in_atomic();
                ms = SecondsFromStart - ms;
                out_atomic();
                if(err==SLC_ERR_NONE){
                    strcpy(bak, buf);
                }   
                printf("\nOK, ms=%ld, err=%08lX, ", ms, err);
                printf("K=%d, L=%d, width=%d\n\n", slc1.k_number, slc1.l_number, 
                                                   slc1.order_kl.width/10);
                if(err != SLC_ERR_NONE){
                    printf(" --- Error Occured: %08lX --- ", err);
                    goto ended_locate;
                }
                textcolor(7);
                printf("RESULT ->\n\n");
                printf(" ");
                for(i=0; i<slc1.k_number; i++){
                    if(slc1.kl_set.k_selected[i])
                        printf("[   K%d   ]", i);
                    else 
                        printf("----------");
                }
                printf("\n");
                printf(" ");
                for(i=0; i<slc1.k_number; i++){
                    if(slc1.kl_set.k_selected[i])
                        textcolor(4);
                    else 
                        textcolor(2);
                    cprintf("  %+-5d   ", slc1.kl_set.k_location[i]);
                }
                printf("\n\n");
                for(i=0; i<slc1.l_number; i++){
                    if(slc1.kl_set.l_selected[i])
                        printf("[ L%-2d]", i);
                    else 
                        printf("------");
                }
                printf("\n");
                for(i=0; i<slc1.l_number; i++){
                    if(slc1.kl_set.l_selected[i])
                        textcolor(4);
                    else 
                        textcolor(2);
                    cprintf(" %+-5d", slc1.kl_set.l_location[i]);
                }
                textcolor(7);
                cprintf("\n\r");
              ended_locate: 
                cprintf("\n\r");
                break;
        }
    }
}

#if USE_FAMESOS_TYPE == 0
void interrupt (*oldint8)(void);
int  dos_timer = 0;

void interrupt myint8(void)
{
    SecondsFromStart++;
    if(dos_timer++>=54) {
       (*oldint8)();
       dos_timer=0;
    } else {
        outportb(0x20,0x20);
    }
}

void OpenIRQ8(void)
{
    int hz,hzl,hzh;
    hz=1193;
    hzl=hz%256;
    hzh=hz/256;
    disable();
    oldint8=getvect(0x08);
    outportb(0x43,0x36);
    outportb(0x40,hzl);
    outportb(0x40,hzh);
    setvect(0x08,myint8);
    enable();
}

void CloseIRQ8(void)
{
    disable();
    setvect(0x08,oldint8);
    outportb(0x43,0x36);
    outportb(0x40,0x00);
    outportb(0x40,0x00);
    enable();
}
#endif

#if USE_FAMESOS_TYPE == 1
void start(void __far * data)
{
    lock_kernel();
    clrscr();
    printf("\nSLC Algorithm test routine, %s %s\n", __DATE__, __TIME__);
    slc_initialize();
    slc_test_main();
    ExitApplication();
}
void TaskSwitchHook(void)
{
}
#else
int main(void)
{
    clrscr();

    printf("\nSLC Algorithm test routine, %s %s\n", __DATE__, __TIME__);

    slc_initialize();

    OpenIRQ8();
    slc_test_main();
    CloseIRQ8();
        
    return 0;
}
#endif


