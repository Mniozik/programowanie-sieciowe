/*
 * Compilation:  gcc -Wall ./zadanie_05_sctp_server.c -o ./server -lsctp
 * Usage:        ./server
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_STREAM 0

int main(int argc, char **argv) {
  int sfd, on = 1, no, i, flags;
  socklen_t sl;
  char buf[1024];
  struct sctp_event_subscribe events;
  struct sctp_paddrparams heartbeat;
  struct sctp_rtoinfo rtoinfo;
  struct sockaddr_in *laddrs[5];
  struct sockaddr_in addr, raddr;

  sfd = socket(PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  
  // Struktura adresowa dla serwera (jak nasluchujemy)
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(1234);

  // -------------- ADD ---- Ustawianie parametrow do przesylania komunikatow SCTP
  memset(&heartbeat, 0, sizeof(heartbeat));
  heartbeat.spp_flags = SPP_HB_ENABLE;
  heartbeat.spp_hbinterval = 2000;
  heartbeat.spp_pathmaxrxt = 1;
  setsockopt(sfd, SOL_SCTP, SCTP_PEER_ADDR_PARAMS , &heartbeat,
             sizeof(heartbeat));

  memset(&rtoinfo, 0, sizeof(rtoinfo));
  rtoinfo.srto_max = 2000;
  setsockopt(sfd, SOL_SCTP, SCTP_RTOINFO , &rtoinfo, sizeof(rtoinfo));

  memset((void*) &events, 0, sizeof(events));
  events.sctp_data_io_event = 1;
  setsockopt(sfd, SOL_SCTP, SCTP_EVENTS, (void*) &events, sizeof(events));
  // --------------- ADD

  bind(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)); // polaczenie adresu z gniazdem
  listen(sfd, 5);

  // pobranie i wyswietlenie lokalnych adresow gniazda pod ktorymi jest dostepny
  no = sctp_getladdrs(sfd, 0, (struct sockaddr**) laddrs); 
  for(i = 0; i < no; i++)
    printf("ADDR %d: %s:%d\n", i + 1, inet_ntoa((*laddrs)[i].sin_addr),
           ntohs((*laddrs)[i].sin_port));
  sctp_freeladdrs((struct sockaddr*) *laddrs);
  
  while (1) {
    flags = 0;
    
    memset(&raddr, 0, sizeof(raddr));
    memset(&buf, 0, sizeof(buf));
    sl = sizeof(raddr);

    // no = recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr*) &raddr, &sl); -- BEFORE
    no = sctp_recvmsg(sfd, (void*) buf, sizeof(buf), (struct sockaddr*) &raddr, &sl, NULL, &flags); 
    printf("[%dB] from %s: %s\n", no, inet_ntoa(raddr.sin_addr), buf);
    
    // sendto(sfd, buf, no, 0, (struct sockaddr*) &raddr, sl); -- BEFORE
    sctp_sendmsg(sfd, buf, no, (struct sockaddr*) &raddr, sl, 0, 0, SERVER_STREAM, 0, 0); //Wysylam to co otrzymalem 
    // sleep(1);                    
  }
  close(sfd);
}
