/*
 * Compilation:  gcc -Wall ./zadanie_06_TCP.c -o ./TCP
 * Usage (Terminals):
 *  1) ./TCP localhost 1235
 *  2) nc -l 1235
 *  3) nc localhost 1234
 */

#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  char buf[256];
  
  // Receive
  socklen_t sl;
  int sfd, cfd;
  int on = 1;
  int rc;
  struct sockaddr_in saddr, caddr; // server/client add-ress

  // Recipient
  int ssd;
  struct sockaddr_in caddr2;
  struct hostent *addrent;

  
  // --- RECEIVE SERVER --- 
  memset(&saddr, 0, sizeof(saddr));

  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port = htons(1234);

  sfd = socket(PF_INET, SOCK_STREAM, 0);
  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

  bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr));
  listen(sfd, 5);


  // --- RECIPIENT ---
  ssd = socket(PF_INET, SOCK_STREAM, 0);
  memset(&caddr2, 0, sizeof(caddr2));
  
  addrent = gethostbyname(argv[1]); // IP address
  caddr2.sin_family = AF_INET;
  caddr2.sin_port = htons(atoi(argv[2])); // port number
  memcpy(&caddr2.sin_addr.s_addr, addrent->h_addr, addrent->h_length);


  while (1)
  {
    // Waiting for connection
    memset(&caddr, 0, sizeof(caddr)); // druga wolna struktura dla tego co przychodzi
    sl = sizeof(caddr);
    cfd = accept(sfd, (struct sockaddr *)&caddr, &sl); 

    // Receive message (data)
    rc = read(cfd, buf, 256);
    close(cfd);

    // Send
    printf("Message: %s \n", buf);

    connect(ssd, (struct sockaddr *)&caddr2, sizeof(caddr2));
    write(ssd, buf, rc);
    close(ssd);
  }
  return EXIT_SUCCESS;
}
