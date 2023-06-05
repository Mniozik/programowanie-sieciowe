/*
 * Compilation:  gcc -Wall ./zadanie_05_sock-buff.c -o ./sock-buff
 * Usage:        ./sock-buff tcp_rmem tcp_wmem
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void changebuffsize(int sfd, int srb, int ssb)
{
  setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, &srb, sizeof(srb));
  setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, &ssb, sizeof(ssb));
}

int getbuffsize(int sfd, int buffname)
{
  int s;
  socklen_t slt = (socklen_t)sizeof(s);
  getsockopt(sfd, SOL_SOCKET, buffname, (void *)&s, &slt);
  return s;
}

void buffsizes(int sfd, int *srb, int *ssb)
{
  *srb = getbuffsize(sfd, SO_RCVBUF);
  *ssb = getbuffsize(sfd, SO_SNDBUF);
}

int main(int argc, char **argv)
{
  int sfd, srb, ssb;
  int tcp_rmem = atoi(argv[1]);
  int tcp_wmem = atoi(argv[2]);

  sfd = socket(PF_INET, SOCK_STREAM, 0);
  buffsizes(sfd, &srb, &ssb);
  printf("--BEFORE--\n  TCP:  RCVBUF = %6d [B]  SNDBUF = %6d [B]\n\n", srb, ssb);

  changebuffsize(sfd, tcp_rmem, tcp_wmem);

  buffsizes(sfd, &srb, &ssb);
  printf("--AFTER--\n TCP:  RCVBUF = %6d [B]  SNDBUF = %6d [B]\n", srb, ssb);
  close(sfd);

  return EXIT_SUCCESS;
}
