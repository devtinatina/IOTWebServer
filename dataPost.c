#include "/usr/local/Cellar/mysql/8.0.22/include/mysql/mysql.h"
#include "stems.h"
//#include "/usr/include/mysql/mysql.h"
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

/* Database */
#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PASS "7624"
#define DB_NAME "test"

bool SelectName(MYSQL *connection, char query[], char *name, char *time, char *value);
bool AddSensor(MYSQL *connection, char query[], char *name, char *time, char *value);
bool AddSensorValue(MYSQL *connection, char query[], char *sensorID, char *time, char *value);
bool AddSensorTable(MYSQL *connection, char query[], char *sensorID, char *time, char *value);
bool SelectCntAve(MYSQL *connection, char query[], char *sensorID);
bool UpdateCntAve(MYSQL *connection, char query[], char *sensorID, char *cnt, char *ave);

bool UpdateCntAve(MYSQL *connection, char query[], char *sensorID, char *cnt, char *ave)
{
  sprintf(query, "UPDATE sensorList SET cnt = %s, ave = %s WHERE id = %s;", cnt, ave, sensorID);
  if (mysql_query(connection, query))
  {
    //printf("query fail\n");
    return false;
  }
  else
  {
    //printf("query suc\n");
    return true;
  }
}

bool SelectCntAve(MYSQL *connection, char query[], char *sensorID)
{
  MYSQL_RES *res;
  MYSQL_ROW row;

  sprintf(query, "SELECT COUNT(*), AVG(value) FROM sensor%s;", sensorID);
  if (mysql_query(connection, query))
  {
    //printf("query fail\n");
    return false;
  }
  else
  {
    //printf("query suc\n");
    res = mysql_store_result(connection);
    while ((row = mysql_fetch_row(res)) != NULL)
    {
      UpdateCntAve(connection, query, sensorID, row[0], row[1]);
    }
    mysql_free_result(res);
    return true;
  }
}

bool AddSensorTable(MYSQL *connection, char query[], char *sensorID, char *time, char *value)
{
  sprintf(query, "create table sensor%s(time double not null, value double not null, idx int (11) not null AUTO_INCREMENT, primary key(idx));", sensorID);
  if (mysql_query(connection, query))
  {
    //printf("query fail\n");
    return false;
  }
  else
  {
    //printf("query suc\n");
    AddSensorValue(connection, query, sensorID, time, value);
    return true;
  }
}

bool AddSensorValue(MYSQL *connection, char query[], char *sensorID, char *time, char *value)
{
  sprintf(query, "INSERT INTO sensor%s(time, value) VALUES (%s, %s);", sensorID, time, value);
  if (mysql_query(connection, query))
  {
    //printf("query fail\n");
    AddSensorTable(connection, query, sensorID, time, value);
    return false;
  }
  else
  {
    //printf("query suc\n");

    SelectCntAve(connection, query, sensorID);
    return true;
  }
}

bool AddSensor(MYSQL *connection, char query[], char *name, char *time, char *value)
{
  Write(STDOUT_FILENO, name, strlen(name));
  sprintf(query, "INSERT INTO sensorList (name, cnt, ave) VALUES ('%s', 0, 0);", name);
  Write(STDOUT_FILENO, query, strlen(query));
  if (mysql_query(connection, query))
  {
    //printf("query fail\n");
    Write(STDOUT_FILENO, "ss", strlen("ss"));
    return false;
  }
  else
  {
    //printf("query suc\n");
    return true;
  }
}

bool SelectName(MYSQL *connection, char query[], char *name, char *time, char *value)
{
  MYSQL_RES *res;
  MYSQL_ROW row;

  sprintf(query, "select id from sensorList where name='%s'", name);
  if (mysql_query(connection, query))
  {
    //not exist
    return false;
  }
  else
  {
    //exist
    res = mysql_store_result(connection);
    while ((row = mysql_fetch_row(res)) != NULL)
    {
      AddSensorValue(connection, query, row[0], time, value);
    }

    mysql_free_result(res);
    return true;
  }
}

//
// This program is intended to help you test your web server.
//

int main(int argc, char *argv[])
{
  int bodylength = atoi(Getenv("CONTENT_LENGTH")) + 1;
  int i = 0;
  char buf[MAXBUF];
  char response[MAXLINE];
  char temp1[MAXBUF], query[MAXBUF];
  char *token[3], *context, *parse, *temp2;

  MYSQL *connection = NULL, conn;
  MYSQL_RES *res;
  MYSQL_ROW row;
  //int query_stat;

  Read(0, buf, bodylength);

  strcpy(temp1, buf);
  temp2 = strtok_r(temp1, "&", &parse);

  while (temp2 != NULL)
  {
    strtok_r(temp2, "=", &context);
    token[i++] = strtok_r(NULL, "=", &context);

    temp2 = strtok_r(NULL, "&", &parse);
  }

  token[1] = strtok(token[1], ".");

  mysql_init(&conn);
  connection = mysql_real_connect(&conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);

  if (connection == NULL)
  {
    fprintf(stderr, "Mysql connection error : %s", mysql_error(&conn));
  }

  sprintf(query, "select COUNT(*) from sensorList where name='%s';", token[0]);
  if (mysql_query(connection, query))
  {
    //not exist
  }
  else
  {
    //exist
    res = mysql_store_result(connection);
    while ((row = mysql_fetch_row(res)) != NULL)
    {
      if (!strcmp(row[0], "0"))
        AddSensor(connection, query, token[0], token[1], token[2]);
    }
    mysql_free_result(res);
  }

  if (SelectName(connection, query, token[0], token[1], token[2]))
  {
    sprintf(response, "%sHTTP/1.0 200 OK\r\n", response);
    sprintf(response, "%sServer: My Web Server\r\n", response);
    sprintf(response, "%sContent-Length: %d\r\n", response, bodylength);
    sprintf(response, "%sContent-Type: text/plain\r\n\r\n", response);
    sprintf(response, "%s%s\r\n", response, buf);
    Write(STDOUT_FILENO, response, strlen(response));
    fflush(stdout);
  }

  mysql_close(connection);

  return (0);
}