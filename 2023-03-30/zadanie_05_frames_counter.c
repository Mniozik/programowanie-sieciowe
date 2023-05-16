/*
 * Compilation:  gcc -Wall ./zadanie_05_frames_counter.c -o ./counter -lpcap
 * Usage:        ./counter INTERFACE
 * NOTE:         This program requires root privileges.
 */

#include <pcap.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_ether.h>
#include <linux/ip.h>

int arp, ip, ip_tcp, ip_udp, other = 0;
char *errbuf;
pcap_t *handle;
struct ethhdr *eth_header;
struct iphdr *ip_header;

void cleanup()
{
  pcap_close(handle);
  free(errbuf);
}

void stop(int signo)
{
  printf("\n----------\n");
  printf("ARP: %d\nIP: %d\nIP(TCP): %d\nIP(UDP): %d\nOTHER: %d", arp, ip, ip_tcp, ip_udp, other);
  printf("\n----------\n");
  exit(EXIT_SUCCESS);
}

void trap(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{
  eth_header = (struct ethhdr *)bytes;

  if (ntohs(eth_header->h_proto) == 0x0806)
    arp++;
  else if (ntohs(eth_header->h_proto) == 0x0800)
  {
    ip++;
    ip_header = (struct iphdr *)(bytes + sizeof(struct ethhdr));
    
    if (ip_header->protocol == 6)
      ip_tcp++;
    else if (ip_header->protocol == 17)
      ip_udp++;
  }
  else
    other++;
}

int main(int argc, char **argv)
{
  atexit(cleanup);
  signal(SIGINT, stop);
  errbuf = malloc(PCAP_ERRBUF_SIZE);
  handle = pcap_create(argv[1], errbuf);
  pcap_set_promisc(handle, 1);
  pcap_set_snaplen(handle, 65535);
  pcap_set_timeout(handle, 1000);
  pcap_activate(handle);
  pcap_loop(handle, -1, trap, NULL);
}
