/*
 * clientPost.c: A very, very primitive HTTP client for sensor
 *
 * To run, prepare config-cp.txt and try:
 *      ./clientPost
 *
 * Sends one HTTP request to the specified HTTP server.
 * Get the HTTP response.
 */

#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
#include "stems.h"

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
extern int cmd_value(int arfc, char *argv[]);
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
  if (argc == 1)
  {
    printf("Current sensor is '%s'\n", Getenv("SENSOR_NAME"));
  }
  else if (argc == 2)
  {
    Setenv("SENSOR_NAME", argv[1], 1);
    printf("Sensor name is changed '%s\n'", Getenv("SENSOR_NAME"));
    return 2;
  }
  else
  {
    fputs("Error : cmd_name\n", stdout);
  }

  return 0;
}
int cmd_value(int argc, char *argv[])
{
  if (argc == 1)
  {
    printf("Current value is '%s'\n", Getenv("RECORDED_VALUE"));
  }
  else if (argc == 2)
  {
    Setenv("RECORDED_VALUE", argv[1], 1);
    printf("Sensor value is changed '%s'\n", Getenv("RECORDED_VALUE"));
    return 3;
  }
  else
  {
    fputs("Error : cmd_value\n", stdout);
  }
  return 0;
}
int cmd_send(int argc, char *argv[])
{
  if (argc == 1)
  {
    return 4;
  }
  else
  {
    fputs("Error : this commad don't have argv.\n", stdout);
  }
  return 0;
}
int cmd_random(int argc, char *argv[])
{
  if (argc == 2)
  {
    Setenv("RANDOM_NUM", argv[1], 1);
    return 5;
  }
  else
  {
    fputs("Error : please use 1 argv\n", stdout);
    return 0;
  }
}
int cmd_quit(int argc, char *argv[])
{
  fputs("Exit. :) \n", stdout);
  return 1;
}

void clientSend(int fd, char *filename, char *body)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "POST %s HTTP/1.1\n", filename);
  sprintf(buf, "%sHost: %s\n", buf, hostname);
  sprintf(buf, "%sContent-Type: text/plain; charset=utf-8\n", buf);
  sprintf(buf, "%sContent-Length: %lu\n\r\n", buf, strlen(body));
  sprintf(buf, "%s%s\n", buf, body);
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
    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1)
      printf("Length = %d\n", length);
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0)
  {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

/* currently, there is no loop. I will add loop later */
void userTask(char *myname, char *hostname, int port, char *filename, float time, float value)
{
  int clientfd;
  char msg[MAXLINE];

  //sprintf(msg, "name=%s&time=%f&value=%f", Getenv("SENSOR_NAME"), Getenv("RECORDED_TIME"), Getenv("RECORDED_VALUE"));
  sprintf(msg, "name=%s&time=%f&value=%f", myname, time, value);
  clientfd = Open_clientfd(hostname, port);
  clientSend(clientfd, filename, msg);
  clientPrint(clientfd);
  Close(clientfd);
}

void getargs_cp(char *myname, char *hostname, int *port, char *filename, float *time, float *value)
{
  FILE *fp;

  char temp_time[MAXLINE];
  char temp_value[MAXLINE];

  fp = fopen("config-cp.txt", "r");
  if (fp == NULL)
    unix_error("config-cp.txt file does not open.");

  fscanf(fp, "%s", myname);
  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", filename);
  fscanf(fp, "%f", time);
  fscanf(fp, "%f", value);

  sprintf(temp_time, "%f", *time);
  sprintf(temp_value, "%f", *value);

  Setenv("SENSOR_NAME", myname, 1);
  Setenv("RECORDED_TIME", temp_time, 1);
  Setenv("RECORDED_VALUE", temp_value, 1);

  fclose(fp);
}

int main(void)
{
  srand(time(NULL));

  char myname[MAXLINE], hostname[MAXLINE], filename[MAXLINE];
  char temp[MAXLINE];
  int port;
  float time, value;
  int sel;

  getargs_cp(myname, hostname, &port, filename, &time, &value);
  while (1)
  {
    //call command processing rootin
    sel = cmdProcessing();
    if (sel == 1)
    { //Exit
      break;
    }
    else if (sel == 2)
    { //Change myname
      sprintf(myname, "%s", Getenv("SENSOR_NAME"));
    }
    else if (sel == 3)
    { //Change value
      sprintf(temp, "%s", Getenv("RECORDED_VALUE"));
      value = atof(temp);
    }
    else if (sel == 4)
    { //Send
      printf("hi\n");
      userTask(myname, hostname, port, filename, time, value);
    }
    else if (sel == 5)
    { //Random
      printf("hi\n");
      sprintf(temp, "%s", Getenv("RANDOM_NUM"));
      for (int i = 0; i < atoi(temp); i++)
      {
        userTask(myname, hostname, port, filename, time, value + (rand() % 21 - 10));
        sleep(1);
      }
    }
  }

  return (0);
}