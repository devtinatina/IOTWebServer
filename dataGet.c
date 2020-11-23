#include "/usr/local/Cellar/mysql/8.0.22/include/mysql/mysql.h"
//#include "/usr/include/mysql/mysql.h"
#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

/* Database */
#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PASS "7624"
#define DB_NAME "test"

void listShow(MYSQL *connection, char query[]);                     // 저장된 센서의 이름을 나열해 주는 함수
void infoShow(MYSQL *connection, char query[], char *name);         // 해당 센서값의 개수와 평균값 출력
void getData(MYSQL *connection, char query[], char *name, int cnt); // 해당 이름의 센서값을 특정 개수만큼 출력해주는 함수
void alarmShow(MYSQL *connection, char query[], char sname[], int threshold);

void listShow(MYSQL *connection, char query[])
{
  MYSQL_RES *res;
  MYSQL_ROW row;

  sprintf(query, "select name from sensorList;");
  if (mysql_query(connection, query))
  {
    //not exist
    printf("list fail\n");
  }
  else
  {
    //exist
    res = mysql_store_result(connection);
    while ((row = mysql_fetch_row(res)) != NULL)
    {
      printf("%s\n", row[0]);
    }

    mysql_free_result(res);
  }
}

void infoShow(MYSQL *connection, char query[], char *name)
{
  MYSQL_RES *res;
  MYSQL_ROW row;

  int err = 0;

  sprintf(query, "select name, cnt, ave from sensorList where name='%s';", name);
  if (mysql_query(connection, query))
  {
    printf("connection fail\n");
  }
  else
  {
    res = mysql_store_result(connection);
    while ((row = mysql_fetch_row(res)) != NULL)
    {
      if (strcmp(name, row[0]) == 0)
        err = 1;
      printf("%s\n", row[1]);
      printf("%s\n", row[2]);
    }
    mysql_free_result(res);
  }
  if (err == 0)
  {
    printf("%s is Not exist\n", name);
  }
}

void getData(MYSQL *connection, char query[], char *name, int cnt)
{
  MYSQL_RES *res;
  MYSQL_ROW row;

  sprintf(query, "select id, cnt from sensorList where name='%s';", name);
  if (mysql_query(connection, query))
  {
    printf("connection fail\n");
  }
  else
  {
    res = mysql_store_result(connection);
    row = mysql_fetch_row(res);

    // when n = 1
    if (cnt == 1)
    {
      sprintf(query, "select time, value from sensor%s where idx=%s;", row[0], row[1]);
      if (mysql_query(connection, query))
      {
        printf("connection fail\n");
      }
      else
      {
        res = mysql_store_result(connection);
        row = mysql_fetch_row(res);
        time_t t = atoi(row[0]);

        printf("%s", ctime(&t));
        printf("%s\n", row[1]);
      }
    }
    else // when n > 1
    {
      sprintf(query, "select time, value from sensor%s order by idx desc;", row[0]); // desc
      if (mysql_query(connection, query))
      {
        printf("connection fail\n");
      }
      else
      {
        res = mysql_store_result(connection);
        for (int i = 0; i < cnt; i++)
        {
          row = mysql_fetch_row(res);
          time_t t = atoi(row[0]);
          printf("%s", ctime(&t));
          printf("%s\n", row[1]);
        }
      }
    }
    mysql_free_result(res);
  }
}

void alarmShow(MYSQL *connection, char query[], char sname[], int threshold)
{
  MYSQL_RES *res;
  MYSQL_ROW row;

  sprintf(query, "select * from alarmTable;");
  if (mysql_query(connection, query))
  {
    printf("connection fail\n");
  }
  else
  {
    res = mysql_store_result(connection);
    while ((row = mysql_fetch_row(res)) != NULL)
    {
      if ((strcmp(row[0], sname) == 0 && threshold < atoi(row[2])))
      {
        printf("\r\n");
        printf("-----threshold value!-----\r\n");
        printf("%s\n", row[0]);
        time_t t = atoi(row[1]);
        printf("%s", ctime(&t));
        printf("%s\n", row[2]);
        sprintf(query, "delete from alarmTable;");
        mysql_query(connection, query);
      }
      else
      {
        //printf("Not threshold\r\n");
      }
    }
    mysql_free_result(res);
  }
}

//
// This program is intended to help you test your web server.
// You can use it to test that you are correctly having multiple
// threads handling http requests.
//
// htmlReturn() is used if client program is a general web client
// program like Google Chrome. textReturn() is used for a client
// program in a embedded system.
//
// Standalone test:
// # export QUERY_STRING="name=temperature&time=3003.2&value=33.0"
// # ./dataGet.cgi

void htmlReturn(void)
{
  char header[MAXLINE];
  char content[MAXLINE];
  char *buf;
  char *ptr;

  /* Make the response body */
  sprintf(content, "%s<html>\r\n<head>\r\n", content);
  sprintf(content, "%s<title>CGI test result</title>\r\n", content);
  sprintf(content, "%s</head>\r\n", content);
  sprintf(content, "%s<body>\r\n", content);
  sprintf(content, "%s<h2>Welcome to the CGI program</h2>\r\n", content);
  buf = Getenv("QUERY_STRING");

  sprintf(content, "%s<p>Env : %s</p>\r\n", content, buf);
  ptr = strsep(&buf, "&");
  while (ptr != NULL)
  {
    sprintf(content, "%s%s\r\n", content, ptr);
    ptr = strsep(&buf, "&");
  }
  sprintf(content, "%s</body>\r\n</html>\r\n", content);

  /* Generate the HTTP response */

  sprintf(header, "%sContent-Length: %lu\r\n", header, strlen(content));
  sprintf(header, "%sContent-Type: text/html\r\n\r\n", header);

  Write(STDOUT_FILENO, header, strlen(header));
  Write(STDOUT_FILENO, content, strlen(content));
  Write(STDOUT_FILENO, "\n\n", strlen("\n\n"));
  fflush(stdout);
}

void textReturn(void)
{
  char content[MAXLINE];
  char *buf;
  char *ptr;

  buf = Getenv("QUERY_STRING");
  sprintf(content, "%sEnv : %s\n", content, buf);
  ptr = strsep(&buf, "&");
  while (ptr != NULL)
  {
    sprintf(content, "%s%s\n", content, ptr);
    ptr = strsep(&buf, "&");
  }

  /* Generate the HTTP response */
  printf("Content-Length: %lu\n", strlen(content));
  printf("Content-Type: text/plain\r\n\r\n");
  printf("%s", content);
  fflush(stdout);
}

int main(void)
{
  // char header[MAXLINE];
  // char content[MAXLINE];
  char *buf;
  // char *ptr;
  int i = 0;
  char temp1[MAXBUF], query[MAXBUF];
  char *token[3], *context, *parse, *temp2;

  buf = Getenv("QUERY_STRING");

  strcpy(temp1, buf);
  temp2 = strtok_r(temp1, "&", &parse);

  while (temp2 != NULL)
  {
    strtok_r(temp2, "=", &context);
    token[i++] = strtok_r(NULL, "=", &context);

    temp2 = strtok_r(NULL, "&", &parse);
  }

  // printf("%s\n", token[0]);
  // printf("%s\n", token[1]);
  // printf("%s\n", token[2]);

  MYSQL *connection = NULL, conn;

  mysql_init(&conn);
  connection = mysql_real_connect(&conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);

  if (connection == NULL)
  {
    fprintf(stderr, "Mysql connection error : %s", mysql_error(&conn));
  }

  if (strcmp(token[0], "LIST") == 0)
  {
    listShow(connection, query);
  }

  if (strcmp(token[0], "INFO") == 0)
  {
    infoShow(connection, query, token[1]);
  }

  if (strcmp(token[0], "GET") == 0)
  {
    getData(connection, query, token[1], atoi(token[2]));
  }

  if (strcmp(token[0], "ALARM") == 0)
  {
    alarmShow(connection, query, token[1], atoi(token[2]));
  }

  //htmlReturn();
  //textReturn();
  mysql_close(connection);
  return (0);
}
