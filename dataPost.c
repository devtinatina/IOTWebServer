#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

//
// This program is intended to help you test your web server.
//

int main(int argc, char *argv[])
{
  int bodylength = atoi(Getenv("CONTENT_LENGTH")) + 1;
  char buf[MAXBUF];
  char response[MAXLINE];
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
