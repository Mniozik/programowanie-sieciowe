/*
 *
 * Compilation:  gcc -Wall ./zadanie_04_rtdelete.c -o ./rtdelete
 * Usage:        ./rtdelete NETIP PREFIX GWIP    // adres sieci ip,  maska zapisawana jako prefix, brama domyslna
 * NOTE:         This program requires root privileges.
 *
 */

#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEV_NUMBER 6 /* !!! */ // by sprawdzic jaki numer mamy to w terminal (ip link show) 

struct reqhdr {
  struct nlmsghdr nl;
  struct rtmsg    rt;
  char buf[8192]; // bufor na strukture atrybutow
};

int main(int argc, char **argv) {
  int dev = DEV_NUMBER;
  int sfd, rtlen;
  struct sockaddr_nl snl;
  struct reqhdr req;
  struct rtattr *atp;

  sfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  memset(&snl, 0, sizeof(struct sockaddr_nl)); // adres obiorcy naszego komunikatu (jadro systemu)
  snl.nl_family = AF_NETLINK;
  snl.nl_pid = 0;

  memset(&req, 0, sizeof(struct reqhdr));
  rtlen = sizeof(struct rtmsg);
  atp = (struct rtattr*) req.buf;
  atp->rta_type = RTA_DST; // adres IP sieci docelowej (1 argument wywolania)
  atp->rta_len = sizeof(struct rtattr) + 4;
  inet_pton(AF_INET, argv[1], ((char*) atp) + sizeof(struct rtattr));
  rtlen += atp->rta_len;

  atp = (struct rtattr*) (((char*) atp) + atp->rta_len);
  atp->rta_type = RTA_GATEWAY; // adres IP bramy (3 argument)
  atp->rta_len = sizeof(struct rtattr) + 4;
  inet_pton(AF_INET, argv[3], ((char*) atp) + sizeof(struct rtattr));
  rtlen += atp->rta_len;

  atp = (struct rtattr*) (((char*) atp) + atp->rta_len);
  atp->rta_type = RTA_OIF; // nr interfejsu przez ktory wiedzie ta trasa (DEFINE 6)
  atp->rta_len = sizeof(struct rtattr) + 4;
  memcpy(((char*) atp) + sizeof(struct rtattr), &dev, 4);
  rtlen += atp->rta_len;

  req.nl.nlmsg_len = NLMSG_LENGTH(rtlen); // rozmiar struktury
  // ----- CHANGE -----
  // req.nl.nlmsg_type = RTM_NEWROUTE; // zadanie utworzenia nowego wpisu w tablicy
  req.nl.nlmsg_type = RTM_DELROUTE; // !!! ZMIANA BY USUWALO !!!
  req.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE; // NIBY MOZNA USUNAC CREATE ALE JAK ZOSTANIE TO TEZ USUWA WPIS 
  // ------------------
  req.nl.nlmsg_seq = 0;
  req.nl.nlmsg_pid = getpid(); // nadawca to nasz proces (pid) 

  req.rt.rtm_family = AF_INET; // IPv4
  req.rt.rtm_dst_len = atoi(argv[2]); // maska jest ta dlugoscia (argv2 [string] przeksztalcony do int'a)
  req.rt.rtm_table = RT_TABLE_MAIN;
  req.rt.rtm_protocol = RTPROT_STATIC;
  req.rt.rtm_scope = RT_SCOPE_UNIVERSE;
  req.rt.rtm_type = RTN_UNICAST;

  sendto(sfd, (void*) &req, req.nl.nlmsg_len, 0, (struct sockaddr*) &snl,
         sizeof(struct sockaddr_nl)); // wysylamy wypelniona strukture

  close(sfd);
  return EXIT_SUCCESS;
}
