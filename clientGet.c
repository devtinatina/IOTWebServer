/*
 * clientGet.c: A very, very primitive HTTP client for console.
 * 
 * To run, prepare config-cg.txt and try: 
 *      ./clientGet
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * For testing your server, you will want to modify this client.  
 *
 * When we test your server, we will be using modifications to this client.
 *
 */

#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include "stems.h"

#define STR_LEN 1024
#define MAX_TOKENS 10

struct CMD
{
  char *name;
  char *desc;
  int (*cmd)(int argc, char *agrv[], char webaddr[]);
};

int cmdProcessing(char webaddr[]);
extern int cmd_help(int arfc, char *argv[], char webaddr[]);
extern int cmd_list(int argc, char *argv[], char webaddr[]);
extern int cmd_info(int arfc, char *argv[], char webaddr[]);
extern int cmd_get(int arfc, char *argv[], char webaddr[]);
extern int cmd_quit(int argc, char *argv[], char webaddr[]);
const int builtins = 5;
struct CMD builtin[] = {
    {"help", "list available commands", cmd_help},
    {"list", "print sensor list in DB", cmd_list},
    {"info", "<sname>: print sensor's information", cmd_info},
    {"get", "<sname> : print current sensor's data (time, data)\nget <sname> <n> : print current sensor's <n> data (time, data)", cmd_get},
    {"quit", "exit from this program", cmd_quit}};

int init_myShell(char *);

int cmdProcessing(char webaddr[])
{
  char cmdLine[STR_LEN] = "";
  char *cmdTokens[MAX_TOKENS];
  char delim[] = " \t\n\r";
  char *token;
  int tokenNum;
  int exitCode = 0;
  int i;

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
  if (strcmp(cmdTokens[0], "quit") == 0)
    return 1;
  for (i = 0; i < builtins; ++i)
    if (strcmp(cmdTokens[0], builtin[i].name) == 0)
    {
      return builtin[i].cmd(tokenNum, cmdTokens, webaddr);
    }

  return exitCode;
}

int cmd_help(int argc, char *argv[], char webaddr[])
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

int cmd_list(int argc, char *argv[], char webaddr[])
{
  if (argc == 1)
  {
    sprintf(webaddr, "/dataGet.cgi?command=LIST");
    return 2;
  }
  else
  {
    fputs("Error : cmd_name\n", stdout);
    return 0;
  }
}
int cmd_info(int argc, char *argv[], char webaddr[])
{
  if (argc == 2)
  {
    sprintf(webaddr, "/dataGet.cgi?command=INFO&sname=%s", argv[1]);
    return 2;
  }
  else
  {
    fputs("Error : cmd_value\n", stdout);
    return 0;
  }
}

int cmd_get(int argc, char *argv[], char webaddr[])
{
  if (argc == 2) // argument 1
  {
    sprintf(webaddr, "/dataGet.cgi?command=GET&sname=%s&n=1", argv[1]);
    return 2;
  }
  if (argc == 3) // argument 2
  {
    sprintf(webaddr, "/dataGet.cgi?command=GET&sname=%s&n=%s", argv[1], argv[2]);
    return 2;
  }
  else
  {
    fputs("Error : cmd_get\n", stdout);
    return 0;
  }
}
int cmd_quit(int argc, char *argv[], char webaddr[])
{
  fputs("Exit. :) \n", stdout);
  return 1;
}

/*
 * Send an HTTP request for the specified file 
 */
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  Rio_writen(fd, buf, strlen(buf));
}

/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];
  int length = 0;
  int n;

  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0))
  {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1)
    {
      printf("Length = %d\n", length);
    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0)
  {
    printf("%s", buf); // delete header:
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

/* currently, there is no loop. I will add loop later */
void userTask(char hostname[], int port, char webaddr[])
{
  int clientfd;

  clientfd = Open_clientfd(hostname, port);
  clientSend(clientfd, webaddr);
  clientPrint(clientfd);
  Close(clientfd);
}

void getargs_cg(char hostname[], int *port, char webaddr[])
{
  FILE *fp;

  fp = fopen("config-cg.txt", "r");
  if (fp == NULL)
    unix_error("config-cg.txt file does not open.");

  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", webaddr);
  fclose(fp);
}

int main(void)
{
  char hostname[MAXLINE], webaddr[MAXLINE];
  int port, sel;

  getargs_cg(hostname, &port, webaddr);
  while (1)
  {
    sel = cmdProcessing(webaddr);
    if (sel == 1)
    {
      break;
    }
    else if (sel == 0)
    {
      // exception when return 0 ;
    }
    else if (sel == 2)
    {
      userTask(hostname, port, webaddr);
    }
  }

  return (0);
}