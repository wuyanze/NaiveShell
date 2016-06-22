//
// Created by wyz on 16-6-15.
//

#include "header.h"

void pr_command(int i)
{
    int j;
    printf("id:%d\n",com[i].id);
    printf("pathname:%s\n",com[i].arg[0]);
    printf("argnum:%d\n",com[i].argnum);
    for(j=0;j<com[i].argnum;j++)
        printf("    arg%d:%s\n",j,com[i].arg[j]);
    printf("in_dir:%s\n",com[i].in_redir);
    printf("out_dir:%s\n",com[i].out_redir);
    printf("pre_pipe:%d\n",com[i].in_fd);
    printf("next_pipe:%d\n",com[i].out_fd);
}
