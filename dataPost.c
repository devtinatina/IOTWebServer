#include "stems.h"
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include "/usr/local/opt/mysql@5.7/include/mysql/mysql.h"

/* Database */

//
// This program is intended to help you test your web server.
//

int main(int argc, char *argv[])
{
  int bodylength = atoi(Getenv("CONTENT_LENGTH")) + 1;
  int i = 0;
  char buf[MAXBUF];
  char response[MAXLINE];
  char temp1[MAXBUF];
  char *temp2;
  char *token[3], *context, *parse;

  printf("version : %s\n", mysql_get_client_info());

  Read(0, buf, bodylength);

  strcpy(temp1, buf);
  temp2 = strtok_r(temp1, "&", &parse);

  while (temp2 != NULL)
  {
    strtok_r(temp2, "=", &context);
    token[i++] = strtok_r(NULL, "=", &context);

    temp2 = strtok_r(NULL, "&", &parse);
  }

  sprintf(response, "HTTP/1.0 200 OK\r\n", response);
  sprintf(response, "%sServer: My Web Server\r\n", response);
  sprintf(response, "%sContent-Length: %d\r\n", response, bodylength);
  sprintf(response, "%sContent-Type: text/plain\r\n\r\n", response);
  sprintf(response, "%s%s\r\n", response, buf);
  Write(STDOUT_FILENO, response, strlen(response));
  fflush(stdout);
  return (0);
}