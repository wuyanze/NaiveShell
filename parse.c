//
// Created by wyz on 16-6-15.
//

#include "header.h"
#include <string.h>
#include <stdio.h>


int background = 0;

int is_empty(int i)
{
    if(str[i] == ' ' || str[i] == '\t')
        return 1;
    return 0;
}


char ret[1024];


void getStr(int *x)
{
    while((*x)<len_of_command && is_empty(*x))
        (*x)++;
    int index=0;
    while((*x)<len_of_command&& !is_empty(*x))
    {
        ret[index++]=str[*x];
        (*x)++;
    }
    ret[index++]=0;
}


void parse(void )
{
    int index = 0;
    int flag = 0;

    int i;
    char s[100];

    background=0;

    for(i = 0; i < len_of_command; i ++)
    {
        if(str[i] == ' ' || str[i] == '\t')
            continue;
        if(str[i] == '<')
        {
            i++;
            getStr(&i);
            strcpy(com[index].in_redir,ret);
        }
        else if(str[i] == '>')
        {
            i++;
            getStr(&i);
            strcpy(com[index].out_redir,ret);
        }
        else if(str[i]=='&')
        {
            background = 1;
        }
        else if(str[i] == '|')
        {
            index++;
        }
        else{
            com[index].arg[com[index].argnum] = strtok(str+i," \t");
            i+=strlen(com[index].arg[com[index].argnum]);
            com[index].argnum++;
            com[index].arg[com[index].argnum] = NULL;

        }
    }
    num_of_command = index+1;
}