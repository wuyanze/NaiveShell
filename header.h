//
// Created by wyz on 16-6-15.
//

#ifndef SHELL_HEADER_H
#define SHELL_HEADER_H

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>


#define MAX_COMMAND 1024
#define MAX_LEN 100

struct command
{
    int argnum; //参数数目
    char *arg[MAX_LEN]; //参数
    char in_redir[MAX_LEN]; //输入重定向
    char out_redir[MAX_LEN]; //输出重定向
    int id; //命令标志符
    int in_fd;
    int out_fd;
};

int is_empty(int i); //判断字符串i是否为空格或者tab
void getStr(int *x);
void parse(void ); //指令解析函数
void pr_command(int ); //打印命令 用于debug

extern struct command com[MAX_COMMAND]; //指令数组
extern char str[1024];  //输入指令字符串
extern int len_of_command; //输入指令字符串长度
extern int num_of_command; //指令个数
extern int background; //指令序列是否需要后台执行
extern int current_fd;
extern char current_path[1024];
void ReadLineInit();
void ReadLineDeInit();
char *ReadCmdLine();


#endif