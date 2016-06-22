//
// Created by wyz on 16-6-13.
//


#include "header.h"
#include "apue.h"
#include <signal.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <assert.h>


#define BUFSIZE 1024

char current_path[1024];
struct command com[MAX_COMMAND];
char str[1024];
int len_of_command;
int num_of_command = 0;
int pfd[MAX_COMMAND][2];
int current_fd;
int job_count;
pid_t pg;

static void sig_catch(int signo)
{
    exit(0);
}

__sighandler_t sigintSAVE,sigquitSAVE,sigtermSAVE;


void sig_init()
{
    sigintSAVE = signal(SIGINT,SIG_IGN);
    sigquitSAVE = signal(SIGQUIT,SIG_IGN);
    sigtermSAVE = signal(SIGTERM,SIG_IGN);
}


void init_command()
{
    int i;
    for(i=0;i<MAX_COMMAND;i++)
    {
        com[i].in_redir[0]=0;
        com[i].out_redir[0]=0;
        com[i].in_fd = 0;
        com[i].out_fd = 1;
        com[i].argnum = 0;
        com[i].id = i;

    }
}


void debug_process()
{
    printf("%d %d %d\n",getpid(),getppid(),getpgrp());
}



struct back_ground_process
{
    int id;
    int infd;
    pid_t pgid;
    char command[1024];
    struct back_ground_process *next;
};
struct back_ground_process *head;


void printBGP()
{
    struct back_ground_process *tra,*next,*pre;
    tra=head;
    int status;
    pid_t pgid;
    pre=NULL;
    head=NULL;
    while(tra!=NULL)
    {
        pgid = tra->pgid;
        next = tra->next;
        status = kill(pgid,0);
        if(status==-1)
        {
            if(pre!=NULL)
                pre->next=next;
            free(tra);
        }
        else
        {
            printf("[%d]\t%d\t%s\t\n",tra->id,pgid,tra->command);
            if(pre==NULL)
                head=tra;
            pre=tra;
        }
        tra=next;
    }
}




void addBGP(pid_t pgid)
{
    struct back_ground_process *p,*tra;
    p = (struct back_ground_process *)malloc(sizeof(struct back_ground_process));
    p->id = ++job_count;
    p->pgid = pgid;
    p->next=NULL;
    p->infd=com[0].in_fd;
    strcpy(p->command,str);
    if(head==NULL) {
        head = p;
    }
    else{
        tra = head;
        while(tra->next!=NULL)
            tra=tra->next;
        tra->next=p;
    }
}


void ttin_handle(int signo)
{
    printf("blocked by %s\n",strsignal(signo));
    signal(signo,SIG_DFL);
    kill(getpid(),signo);
}



void do_command(int i)
{
    int fd;
    pid_t pid;
    pid = fork();
    sig_init();
    if(pid==0)
    {
        if(pg==-1){
            setpgid(0,getpid());
        }
        else
            setpgid(0,pg);


        signal(SIGINT,SIG_DFL);
        signal(SIGQUIT,SIG_DFL);
        signal(SIGTERM,SIG_DFL);
        signal(SIGTTIN,ttin_handle);

        dup2(com[i].in_fd,0);
        dup2(com[i].out_fd,1);


        if(com[i].in_fd==0)
        {
            if(background==0)
                tcsetpgrp(0,getpgrp());
        }
        if(com[i].out_fd==1)
        {
            if(background==0)
                tcsetpgrp(1,getpgrp());
        }


        for(fd=3;fd<1024L;fd++)
            close(fd);

        execvp(com[i].arg[0],com[i].arg);
        printf("Command Error\n");
        exit(1);

    }
    else{
        if(background==0) {
            while (waitpid(pid, NULL, 0) < 0) {
                if (errno != EINTR) {
                    break;
                }
            }
        }
        else{
            if(i==0)
            {
                pg=pid;
                addBGP(pid);
            }

            signal(SIGCHLD,SIG_IGN);
        }
        tcsetpgrp(0,getpgrp());
        tcsetpgrp(1,getpgrp());

    }

}


void at_exit()
{
    ReadLineDeInit();

    struct back_ground_process *p,*next;
    p = head;
    while(p!=NULL)
    {
        next=p->next;
        kill(-p->pgid,9);
        free(p);
        p=next;
    }
}


void Cd()
{
    int fd;
    fd = openat(current_fd,com[0].arg[1],O_RDONLY);
    fchdir(fd);
    getcwd(current_path,1024);
    current_fd = open(current_path,O_RDONLY);
}





void Fg()
{
    int id = atoi(com[0].arg[1]);
    int status;
    pid_t pgid;
    struct back_ground_process *p,*next;
    p=head;
    while(p!=NULL)
    {
        next=p->next;
        pgid=p->pgid;
        status=kill(pgid,0);
        if(status==-1){
            p=next;
            continue;
        }
        if(p->id==id)
        {
            if(p->infd==0)
            {
                int ret;
                ret = tcsetpgrp(0,pgid);
                kill(pgid,SIGCONT);
            }
            waitpid(pgid,NULL,0);
            tcsetpgrp(0,getpid());
            break;
        }
        p=next;
    }
}


void Bg() {
    int id = atoi(com[0].arg[1]);
    int status;
    pid_t pgid;
    struct back_ground_process *p, *next;
    p = head;
    while (p != NULL) {
        next = p->next;
        pgid = p->pgid;
        status = kill(pgid, 0);
        if (status == -1) {
            p = next;
            continue;
        }
        if (p->id == id) {
            kill(pgid, SIGCONT);
            break;
        }
        p = next;
    }
}




char saveStr[1024];
void ALias()
{
    int fd,len;
    char buf[1024];
    if(com[0].argnum==1)
    {
        fd = open("/home/wyz/sync/shell/ALIAS",O_RDONLY);
        while((len=read(fd,buf,1024))!=0)
        {
            write(STDIN_FILENO,buf,len);
        }
        close(fd);
        return ;
    }

    char *p;
    p=strtok(saveStr," ");
    p=strtok(NULL," =");
    fd = open("/home/wyz/sync/shell/ALIAS",O_RDWR|O_CREAT|O_APPEND,0666);
    write(fd,p,strlen(p));
    write(fd,"\n",1);
    p= p + strlen(p) + 1;
    int i=0,flag=0;

    while(*p)
    {
        if(*p == '\'')
        {
            flag=1;
            p++;
            continue;
        }
        if(flag)
            buf[i++]=*p;
        p++;
    }
    buf[i++]=0;
    write(fd,buf,strlen(buf));
    write(fd,"\n",1);
    close(fd);
}

void replaceWithAlias()
{
    char str_copy[1024];
    char buf1[1024],buf2[1024];
    char tmp[1024],*p;
    int i,index=0;
    strcpy(str_copy,str);
    FILE *fp = fopen("/home/wyz/sync/shell/ALIAS","r");
    if(fp==NULL)
        return ;
    int len1,len2,j;
    while(fgets(buf1,1024,fp) != NULL )
    {
        fgets(buf2,1024,fp);
        len1=strlen(buf1),len2=strlen(buf2);
        buf1[--len1]=0,buf2[--len2]=0;
        index=0;
        while((p = strstr(str+index,buf1))!=NULL)
        {
            j=0;
            for(i=0;i<len2;i++)
                p[j++]=buf2[i];
            for(i=(p-str)+len1;i<strlen(str_copy);i++)
                p[j++]=str_copy[i];
            p[j++]=0;
            strcpy(str_copy,str);
            index=p-str+len2;
        }
    }
    fclose(fp);
}



void History()
{
    int i;
    if(com[0].argnum==1)
    {
        for(i=1;i<=history_length;i++)
            printf("%d\t%s\t\n",i,history_get(i)->line);
        return ;
    }
    int num = atoi(com[0].arg[1]);
    num=min(num,history_length);
    for(i=history_length-num+1;i<=history_length;i++)
        printf("%d\t%s\t\n",i,history_get(i)->line);
}




int main()
{
    int i;
    int len;
    char *buf;

    int fffd[2];
    int fd;

    sig_init();
    ReadLineInit();

    current_path[0]=0;

    getcwd(current_path,1024);
    current_fd = open(current_path,O_RDONLY);

    signal(SIGTTOU,SIG_IGN);


    atexit(at_exit);
    head = NULL;
    job_count=0;
    while(1)
    {
        pg=-1;
        init_command();

        buf = ReadCmdLine();
        if(buf==NULL )
            exit(0);
        if(*buf==0)
            continue;
        len_of_command=strlen(buf);
        for(i=0;i<len_of_command;i++)
            saveStr[i] = str[i]=buf[i];
        str[len_of_command]=0;
        saveStr[len_of_command]=0;
        replaceWithAlias();
        len_of_command=strlen(str);
        for(i=0;i<len_of_command;i++)
            saveStr[i] = str[i];
        saveStr[len_of_command]=0;
        if(strcmp(str,"exit")==0)
            break;
        parse();
        if(strcmp(com[0].arg[0],"cd")==0) {
            Cd();
            continue;
        }
        else if(strcmp(com[0].arg[0],"jobs")==0) {
            printBGP();
            continue;
        }
        else if(strcmp(com[0].arg[0],"fg")==0)
        {
            Fg();
            continue;
        }
        else if(strcmp(com[0].arg[0],"bg")==0)
        {
            Bg();
            continue;
        }
        else if(strcmp(com[0].arg[0],"alias")==0)
        {
            ALias();
            continue;
        }
        else if(strcmp(com[0].arg[0],"history")==0)
        {
            History();
            continue;
        }
        for(i=0;i<num_of_command;i++)
        {
            if(i+1<num_of_command){
                pipe(fffd);
                com[i].out_fd = fffd[1];
                com[i+1].in_fd = fffd[0];
            }
            if(com[i].in_redir[0]!=0)
            {
                fd = openat(current_fd,com[i].in_redir,O_RDONLY);
                com[i].in_fd = fd;
            }
            if(com[i].out_redir[0]!=0)
            {
                fd = openat(current_fd,com[i].out_redir,O_WRONLY|O_CREAT|O_TRUNC,0666);
                com[i].out_fd = fd;
            }
        }


//        if(background==1&&com[0].in_fd==0)
//            com[0].in_fd=open("/dev/null",O_RDONLY);
        for(i=0;i<num_of_command;i++)
        {
            do_command(i);
            if(com[i].in_fd!=0)
                close(com[i].in_fd);
            if(com[i].out_fd!=1)
                close(com[i].out_fd);
        }

        if(background)
            printBGP();




    }


}