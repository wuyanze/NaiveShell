//
// Created by wyz on 16-6-21.
//

#include "header.h"


static char *lineRead = NULL;  //终端输入字符串
static char *stripLine = NULL; //剔除前端空格的输入字符串


static char *cmd[10000];
static int cmdNum;
static char shell_prompt[1024];




static char *StripWhite(char *pszOrig)
{
    if(NULL == pszOrig)
        return "NUL";

    char *pszStripHead = pszOrig;
    while(isspace(*pszStripHead))
        pszStripHead++;

    if('\0' == *pszStripHead)
        return pszStripHead;

    char *pszStripTail = pszStripHead + strlen(pszStripHead) - 1;
    while(pszStripTail > pszStripHead && isspace(*pszStripTail))
        pszStripTail--;
    *(++pszStripTail) = '\0';

    return pszStripHead;
}

char *ReadCmdLine()
{
    //若已分配命令行缓冲区，则将其释放
    if(lineRead)
    {
        free(lineRead);
        lineRead = NULL;
    }
    //读取用户输入的命令行
    snprintf(shell_prompt, sizeof(shell_prompt), "%s:%s $ ", getenv("USER"), getcwd(NULL, 1024));
    lineRead = readline(shell_prompt);

    if(lineRead == NULL)
        return NULL;

    //剔除命令行首尾的空白字符。若剔除后的命令不为空，则存入历史列表
    stripLine = StripWhite(lineRead);
    if(stripLine && *stripLine)
        add_history(stripLine),write_history("/home/wyz/sync/shell/HISTORY");

    return stripLine;
}



static char *CmdGenerator(const char *pszText, int dwState)
{
    static int dwListIdx = 0, dwTextLen = 0;
    if(!dwState)
    {
        dwListIdx = 0;
        dwTextLen = strlen(pszText);
    }

    //当输入字符串与命令列表中某命令部分匹配时，返回该命令字符串
    const char *pszName = NULL;
    while((pszName = cmd[dwListIdx]))
    {
        dwListIdx++;

        if(!strncmp (pszName, pszText, dwTextLen))
            return strdup(pszName);
    }

    return NULL;
}

static char **CmdCompletion (const char *pszText, int dwStart, int dwEnd)
{
    //rl_attempted_completion_over = 1;
    char **pMatches = NULL;
    if(0 == dwStart)
        pMatches = rl_completion_matches(pszText, CmdGenerator);

    return pMatches;
}

//初始化Tab键能补齐的Command函数
static void InitReadLine(void)
{
    rl_attempted_completion_function = CmdCompletion;
}


static void LoadCmd(DIR *dp, struct dirent *dir, char *path)
{
    dp = opendir(path);
    int i,len;
    while(dir = readdir(dp))
    {
        len = strlen(dir->d_name);
        cmd[cmdNum] = malloc(len+1);
        for(i=0;i<len;i++)
            cmd[cmdNum][i] = dir->d_name[i];
        cmd[cmdNum][len] = 0;
        cmdNum++;
    }
    closedir(dp);
}


static void AddSpecialCom(char *cc)
{
    int i,len;
    len = strlen(cc);
    cmd[cmdNum] = malloc(len+1);
    for(i=0;i<len;i++)
        cmd[cmdNum][i] = cc[i];
    cmd[cmdNum][len] = 0;
    cmdNum++;
}

void ReadLineInit()
{
    struct dirent *dir;
    DIR *dp;
    cmdNum=0;
    LoadCmd(dp, dir, "/usr/bin");
    LoadCmd(dp, dir, "/bin");
    AddSpecialCom("alias");
    AddSpecialCom("history");
    stifle_history(1000);
    read_history("/home/wyz/sync/shell/HISTORY");
    InitReadLine();
}

void ReadLineDeInit()
{
    int i;
    for(i=0;i<cmdNum;i++)
        free(cmd[i]);
}



