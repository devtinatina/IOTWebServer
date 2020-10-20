#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

#define STR_LEN 1024
#define MAX_TOKENS 10

struct CMD
{
  char *name;
  char *desc;
  int (*cmd)(int argc, char *agrv[]);
};

int cmdProcessing(void);
extern int cmd_help(int arfc, char *argv[]);
extern int cmd_name(int argc, char *argv[]);
//extern int cmd_changeName(int arfc, char* argv[]);
extern int cmd_value(int arfc, char *argv[]);
//extern int cmd_valueChange(int arfc, char* argv[]);
extern int cmd_send(int arfc, char *argv[]);
extern int cmd_random(int arfc, char *argv[]);
extern int cmd_quit(int argc, char *argv[]);
const int builtins = 6;
struct CMD builtin[] = {
    {"help", "list available commands", cmd_help},
    {"name", "print current sensor name\nname <sensor>: change sensor name to <sensor>", cmd_name},
    //{"name <sensor>", "change sensor name to <sensor>", cmd_changeName},
    {"value", "print help for builtin function\nvalue <n>: set sensor value to <n>", cmd_value},
    //{"value <n>", "set sensor value to <n>", cmd_valueChange},
    {"send", "send (current sensor anme, time, value) to server", cmd_send},
    {"random", "<n>: send (name, time, random value) to server <n> times", cmd_random},
    {"quit", "exit from this program", cmd_quit}};

int init_myShell(char *);

int cmdProcessing(void)
{
  char cmdLine[STR_LEN] = "";
  char *cmdTokens[MAX_TOKENS];
  char delim[] = " \t\n\r";
  char *token;
  int tokenNum;
  int exitCode = 0;
  int i;
  int state;

  //command line
  //fputs("If you want to see commands, type 'help'\n", stdout);

  fputs(">>", stdout);
  fgets(cmdLine, STR_LEN, stdin);

  //token split
  tokenNum = 0;
  token = strtok(cmdLine, delim);
  while (token)
  {
    cmdTokens[tokenNum++] = token;
    token = strtok(NULL, delim);
  }
  cmdTokens[tokenNum] = NULL;
  if (tokenNum == 0)
    return exitCode;

  for (i = 0; i < builtins; ++i)
    if (strcmp(cmdTokens[0], builtin[i].name) == 0)
      return builtin[i].cmd(tokenNum, cmdTokens);

  return exitCode;
}

int cmd_help(int argc, char *argv[])
{
  int i;

  if (argc == 1)
  {
    for (i = 0; i < builtins; ++i)
    {

      fputs(builtin[i].name, stdout);
      fputs(": ", stdout);
      fputs(builtin[i].desc, stdout);
      fputc('\n', stdout);
    }
  }
  else if (argc == 2)
  {
    for (i = 0; i < builtins; ++i)
    {
      if (strcmp(argv[1], builtin[i].name) == 0)
      {
        fputs(builtin[i].name, stdout);
        fputs(": ", stdout);
        fputs(builtin[i].desc, stdout);
        fputc('\n', stdout);
        break;
      }
    }
    if (i == builtins)
      fputs("can't find command\n", stdout);
  }
  else
    fputs("help: help [argument]\n", stdout);

  return 0;
}
int cmd_name(int argc, char *argv[])
{
  char curpath[STR_LEN];
  int dest = 1;

  if (argc == 1)
    chdir(getenv("HOME"));

  getcwd(curpath, STR_LEN);
  setenv("OLDPWD", curpath, 1);

  chdir(argv[dest]);
  getcwd(curpath, STR_LEN);
  setenv("PWD", curpath, 1);

  return 0;
}
//int cmd_changeName(int arfc, char* argv[]) {

//}
int cmd_value(int argc, char *argv[])
{
  char *buf;

  buf = getenv("PWD");
  fputs(buf, stdout);
  fputs("\n", stdout);

  return 0;
}
//int cmd_valueChange(int arfc, char* argv[]) {

//}
int cmd_send(int arfc, char *argv[])
{
}
int cmd_random(int arfc, char *argv[])
{
}
int cmd_quit(int argc, char *argv[])
{
  fputs("Exit my shell. bye~! :) \n", stdout);
  return 1;
}
//
// This program is intended to help you test your web server.
//

// pid_t child;

int main(void)
{
  char myname[MAXLINE], hostname[MAXLINE], filename[MAXLINE];
  int port;
  float time, value;

  cmdProcessing();

  getargs_cp(myname, hostname, &port, filename, &time, &value);

  userTask(myname, hostname, port, port, filename, time, value);

  return (0);
}