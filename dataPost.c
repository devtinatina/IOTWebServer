//#include "/usr/local/opt/mysql@5.7/include/mysql/mysql.h"

#include "stems.h"
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

/* Database */
#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PASS "762486"
#define DB_NAME "test"

//
// This program is intended to help you test your web server.
//

int main(int argc, char *argv[])
{
  int bodylength = atoi(Getenv("CONTENT_LENGTH")) + 1;
  int i = 0, j = 0;
  char buf[MAXBUF];
  char response[MAXLINE];
  char temp1[MAXBUF], query[MAXBUF], sensorID[MAXBUF];
  char *token[3], *context, *parse, *temp2;

  Read(0, buf, bodylength);

  sprintf(response, "HTTP/1.0 200 OK\r\n", response);
  sprintf(response, "%sServer: My Web Server\r\n", response);
  sprintf(response, "%sContent-Length: %d\r\n", response, bodylength);
  sprintf(response, "%sContent-Type: text/plain\r\n\r\n", response);
  sprintf(response, "%s%s\r\n", response, buf);
  Write(STDOUT_FILENO, response, strlen(response));
  fflush(stdout);

  return (0);
}