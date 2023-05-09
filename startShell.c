#include "io.h"
#include "myPrintk.h"
#include "uart.h"
#include "vga.h"
#include "i8253.h"
#include "i8259A.h"
#include "tick.h"
#include "wallClock.h"

int func_cmd(int argc, char(*argv)[8]);
int func_help(int argc, char(*argv)[8]);

#define nullstr "null"

typedef struct myCommand {
    char name[80];
    char help_content[200];
    int (*func)(int argc, char (*argv)[8]);
}myCommand; 

myCommand cmd={"cmd\0","List all command\n\0",func_cmd};
myCommand help={"help\0","Usage: help [command]\n\0Display info about [command]\n\0",func_help};
myCommand commands[]={
    {"cmd","List all command\n",func_cmd},
    {"help","Usage: help [command]\nDisplay info about [command]\n",func_help},
    {nullstr,nullstr,0}
};


int ud_strcmp(char *str1, char *str2){
    int i;

    while (*str1 && *str2)
    {
        if(*str1 != *str2)
        {
            return (*str1 > *str2)? 1 : -1;
        }
        str1++;
        str2++;
    }
    return (*str1 == *str2)? 0 : ((*str1 > *str2)? 1 : -1);
}

int func_cmd(int argc, char (*argv)[8]){
	int i;

    myPrintk(0x7, "list all the command:\n");
    for(i = 0; ud_strcmp(commands[i].name, nullstr)!=0; i++)
    {
        myPrintk(0x7, "%s\n",commands[i].name);
    }
} 


int func_help(int argc, char (*argv)[8]){
    if(argc == 1)//help only
    {
        myPrintk(0x7, "%s", "Usage:help [command]\nDisplay info about [command].\n");
        return 0;
    }
    
    //search for [command]
    int i;
    for(i = 0; ud_strcmp(commands[i].name, nullstr)!= 0; i++)
    {
        if(ud_strcmp(commands[i].name, argv[1]) == 0)
        {
            myPrintk(0x7, "%s", commands[i].help_content);
            return 0;
        }
    }

    //[command] not found
    myPrintk(0x7, "Command \"%s\" is not found.\n", argv[1]);
    return -1;

}

int split_str(char *str, char (*argv)[8]){
    int i,j;
    i = 0;
    int count = 0;

    while (*str++ == ' ');//skip at the beginning
    str--;
    
    while (*str && *str!='\r' && *str!='\n')//go through str
    {
        if(*str == ' ')//space
        {
            while (*str++ == ' ');//skip
            str--;
            if(*str)
            {
                argv[count][i] = '\0';
                count++;
                i = 0;
            }
        }
        else
        {
            if(i < 6)//max length 8
            {
                argv[count][i++]=*str;
            }
            str++;
        }
    }
    argv[count++][i] = '\0';
    return count;
    
}

int argv_classified(int argc, char (*argv)[8]){
    int i;

    for(i = 0; ud_strcmp(commands[i].name, nullstr)!= 0; i++)
    {
        if(ud_strcmp(argv[0],commands[i].name) == 0)//correspondent command found
        {
            commands[i].func(argc, argv);
            return 0;
        }
    }

    //not found
    myPrintk(0x07, "Command \"%s\" is not found.\n", argv[0]);
    return 0;
}

void startShell(void){
//我们通过串口来实现数据的输入
char BUF[256]; //输入缓存区
int BUF_len=0;	//输入缓存区的长度
    
	int argc;
    char argv[8][8];
    int i,j;

    do{
        for(i = 0; i < 8; i++)
        {
            for(j = 0; j < 8; j++)
            {
                argv[i][j] = '\0';
            }
        }
        BUF_len=0; 
        myPrintk(0x07,"Student>>\0");
        while((BUF[BUF_len]=uart_get_char())!='\r'){
            uart_put_char(BUF[BUF_len]);//将串口输入的数存入BUF数组中
            BUF_len++;  //BUF数组的长度加
        }
        uart_put_chars(" -pseudo_terminal\0");
        uart_put_char('\n');

        BUF[BUF_len] = '\0'; //cover '\r'
        
        
        argc = split_str(BUF, argv);//split buf
        myPrintk(0x07, "%s\n", BUF);//print buf on qemu
        argv_classified(argc, argv);

    }while(1);

}

