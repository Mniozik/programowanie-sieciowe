/*
 * Compilation:  gcc -Wall ./zadanie_03_ipping.c -o ./ipping
 * Usage:        ./ipping DST_IP_ADDR
 * NOTE:         This program requires root privileges.
 * 
 */

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

char* argv1;
int tx = 0, rx = 0;

void cleanup() { // jesli wcisniemy ctrl+c
  printf("\n--- %s statistics ---\n", argv1);
  printf("%d packets transmitted, %d packets received, %d%% packet loss\n",
      tx, rx, ((tx-rx)*100)/tx);
}

void stop(int signo) {
  exit(EXIT_SUCCESS);
}

void tdiff(struct timeval* t1, struct timeval* t2) {
  t1->tv_sec = t2->tv_sec - t1->tv_sec;
  if ((t1->tv_usec = t2->tv_usec - t1->tv_usec) < 0) {
    t1->tv_sec--;
    t1->tv_usec += 1000000;
  }
}

uint16_t chksum(uint16_t *addr, int len) { // obliczenie sumy kontrolnej naglowka ICMP
  int nleft = len, sum = 0;
  uint16_t *w = addr, u = 0, result;

  while(nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }
  if (nleft == 1) {
    *(u_char*) &u = *(u_char*) w;
    sum += u;
  }
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  result = ~sum;
  return result;
}

int main(int argc, char **argv) {
  int sfd, rc;
  long rtt;
  socklen_t sl;
  char buf[2048];
  struct timeval out, in;
  struct sockaddr_in snd, rcv;
  struct icmphdr req;
  struct icmphdr *rep;
  struct iphdr *ip;

  atexit(cleanup);
  signal(SIGINT, stop);
  argv1 = argv[1];
  sfd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
  memset(&snd, 0, sizeof(snd));
  snd.sin_family = AF_INET;
  snd.sin_port = 0;
  snd.sin_addr.s_addr = inet_addr(argv[1]);
  memset(&req, 0, sizeof(req));
  req.type = ICMP_ECHO; //naglowek protokolu ICMP, tu: jego typ
  req.code = 0;
  req.un.echo.id = htons(1234); //identyfikator zapytania
  req.un.echo.sequence = tx; //kolejny numer zadania ECHO 
  printf("IPPING %s\n", argv[1]);
  while(1) {
    tx++; //nr pakietu ktory wysylam
    req.un.echo.sequence = htons(tx); //wstawienie do naglowka ICMP 
    req.checksum = 0; 
    req.checksum = chksum((uint16_t*) &req, sizeof(req)); //obliczenie sumy kontrolnej naglowka ICMP
    gettimeofday(&out, NULL);
    sendto(sfd, &req, sizeof(req), 0, (struct sockaddr*) &snd, sizeof(snd));
    sl = sizeof(rcv);
    rc = recvfrom(sfd, &buf, sizeof(buf), 0, (struct sockaddr*) &rcv, &sl);
    gettimeofday(&in, NULL); // pozyskanie czasu
    if (rcv.sin_addr.s_addr == snd.sin_addr.s_addr) {
      tdiff(&out, &in); // obliczenie roznicy pomiedzy wyslaniem a odebrania
      rtt = out.tv_sec * 1000000 + out.tv_usec;
      rx++;
      ip = (struct iphdr*) &buf;
      rep = (struct icmphdr*) ((char*) buf + (ip->ihl * 4)); //rozmiar naglowka IP jest w slowach 32 bitowych, dlatego mnozenie przez 4
      // ---- Zadanie 03 ----
      if (rep->type == ICMP_ECHOREPLY) 
      {
        printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
             rc - (ip->ihl * 4), argv[1], ntohs(rep->un.echo.sequence),
             ip->ttl, rtt/1000.0);
      }
      // --------------------
    }
    sleep(1);
  }
  close(sfd);
  return EXIT_SUCCESS;
}
