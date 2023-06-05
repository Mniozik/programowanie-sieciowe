/*
 * Compilation:  gcc -Wall ./zadanie_05_sctp_client.c -o ./client -lsctp
 * Usage:        ./client SRVIP
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

#define CLIENT_STREAM 1

int main(int argc, char **argv) {
  int sfd, no, i, flags;
  socklen_t sl;
  char buf[1024];
  struct sctp_event_subscribe events;
  struct sctp_initmsg initmsg; 
  struct sctp_paddrparams heartbeat;
  struct sctp_rtoinfo rtoinfo;
  struct sockaddr_in *paddrs[5];
  struct sockaddr_in saddr, raddr;

  sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

  // Struktura adresowa dla klienta (podajemy adres serwera do ktorego bedziemy sie laczyc)
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = PF_INET;
  saddr.sin_port = htons(1234);
  saddr.sin_addr.s_addr = inet_addr(argv[1]);

  // Struktura initmsg dla parametrow inicjalizacji SCTP
  memset(&initmsg, 0, sizeof(initmsg));
  initmsg.sinit_num_ostreams = 2;       /* number of outbound streams */
  initmsg.sinit_max_instreams = 2;      /* number of inbound streams */
  initmsg.sinit_max_attempts = 1;       /* number of INIT resends */ //klient wysyla init i jak sie nie dostanie to retransmisja
  setsockopt(sfd, SOL_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg)); // zatwierdzenie zmian 
  
  // Struktura heartbeat dla parametrow heartbeat SCTP
  memset(&heartbeat, 0, sizeof(heartbeat)); // jesli nie odbieramy hertbat to nie dziala polaczenie, chyba ze nie dostaniemy potwierdzenia po wyslaniu danych (po ilu retramisjach mozna uznac ze to polczenie nie dziala)
  heartbeat.spp_flags = SPP_HB_ENABLE;  /* enable heartbeats */
  heartbeat.spp_hbinterval = 2000;      /* heartbeat interval in ms */ 
  heartbeat.spp_pathmaxrxt = 1;         /* maximum number of retransmission */
  setsockopt(sfd, SOL_SCTP, SCTP_PEER_ADDR_PARAMS , &heartbeat,
             sizeof(heartbeat));
  
  // Struktura rtoinfo dla parametrow RTO (Retransmission Timeout) SCTP
  memset(&rtoinfo, 0, sizeof(rtoinfo));
  rtoinfo.srto_max = 2000;              /* maximum retransmission timeout */
  setsockopt(sfd, SOL_SCTP, SCTP_RTOINFO , &rtoinfo, sizeof(rtoinfo));
  
  // Struktura events dla subskrypcji zdarzen SCTP
  memset((void*) &events, 0, sizeof(events));
  events.sctp_data_io_event = 1;        /* reception of SCTP_SNDRCV */ // wlaczamy obsluge strumieni
  setsockopt(sfd, SOL_SCTP, SCTP_EVENTS, (void*) &events, sizeof(events));
  
  // Wyswietlenie adresow pod ktorymi dostepny jest serwer do ktoego sie podlaczylismy
  connect(sfd, (struct sockaddr*) &saddr, sizeof(saddr)); //podlaczymy sie do serwera
  no = sctp_getpaddrs(sfd, 0, (struct sockaddr**) paddrs); //pobierzemy wszystkie adresy pod jakimi dostepny jest serwer do ktorego sie podlaczylismy
  for (i = 0; i < no; i++)
    printf("ADDR %d: %s:%d\n", i + 1, inet_ntoa((*paddrs)[i].sin_addr),
           ntohs((*paddrs)[i].sin_port));
  sctp_freepaddrs((struct sockaddr*)*paddrs); //zwolnienie pamieci zaalokowanej przez funkcje getpaddrs
  
  // Wysylanie echo message
  while (1) { 
    flags = 0;
    // write(sfd, "Echo message", 13); -- BEFORE
    sctp_sendmsg(sfd, "Echo message", 13, NULL, 0, 0, 0, CLIENT_STREAM, 0, 0);
    
    memset(&raddr, 0, sizeof(raddr));
    memset(&buf, 0, sizeof(buf));
    sl = sizeof(raddr);

    // no = recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr*) &raddr, &sl); -- BEFORE
    no = sctp_recvmsg(sfd, (void*) buf, sizeof(buf), (struct sockaddr*) &raddr, &sl, NULL, &flags); 
    buf[no] = 0;
    printf("[%dB] from %s: %s\n", no, inet_ntoa(raddr.sin_addr), buf); // RECEIVE (to co wyslalem "Echo message")

    sleep(1);
  }
  close(sfd);
}
