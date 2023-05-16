/*
 * Compilation:  gcc -Wall ./zadanie_04_arping.c -o ./arping -lpcap -lnet
 * Usage:        ./arping IFNAME HOST
 * NOTE:         This program requires root privileges.
 */

#include <pcap.h>
#include <libnet.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <time.h>

clock_t time_start, time_stop;
float time_msec;

struct ethhdr *eth_header;
struct arphdr *arp_header;

pcap_t *handle;
char *errbuf_pcap;

struct arphdr
{
    u_int16_t ftype;
    u_int16_t ptype;
    u_int8_t flen;
    u_int8_t plen;
    u_int16_t opcode;
    u_int8_t sender_mac_addr[6];
    u_int8_t sender_ip_addr[4];
    u_int8_t target_mac_addr[6];
    u_int8_t target_ip_addr[4];
};

void cleanup()
{
    pcap_close(handle);
    free(errbuf_pcap);
}

void fstop(int signo)
{
    printf("\n------END------\n");
    exit(EXIT_SUCCESS);
}

void response(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{
    eth_header = (struct ethhdr *)bytes;
    arp_header = (struct arphdr *)(bytes + sizeof(struct ethhdr));
    printf("from %02x:%02x:%02x:%02x:%02x:%02x (%d.%d.%d.%d) ",
           arp_header->sender_mac_addr[0], arp_header->sender_mac_addr[1], arp_header->sender_mac_addr[2], arp_header->sender_mac_addr[3], arp_header->sender_mac_addr[4], arp_header->sender_mac_addr[5],
           arp_header->sender_ip_addr[0], arp_header->sender_ip_addr[1], arp_header->sender_ip_addr[2], arp_header->sender_ip_addr[3]);
    pcap_breakloop(handle);
}

int main(int argc, char **argv)
{
    libnet_t *ln;
    u_int32_t target_ip_addr, src_ip_addr;
    u_int8_t bcast_hw_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 
        zero_hw_addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};       
    struct libnet_ether_addr *src_hw_addr;
    char errbuf_libnet[LIBNET_ERRBUF_SIZE];

    ln = libnet_init(LIBNET_LINK, argv[1], errbuf_libnet);           // inicjalizacja biblioteki libnet
    src_ip_addr = libnet_get_ipaddr4(ln);                            // adres IP interfejsu  sieciowego (naszego)
    src_hw_addr = libnet_get_hwaddr(ln);                             // adres MAC interfejsu sieciowego
    target_ip_addr = libnet_name2addr4(ln, argv[2], LIBNET_RESOLVE); // zamiana adresu na format sieciowy | LIBNET_RESOLVE - nazwa hosta na adres IP

    atexit(cleanup);
    signal(SIGINT, fstop);
    errbuf_pcap = malloc(PCAP_ERRBUF_SIZE);

    handle = pcap_create(argv[1], errbuf_pcap);
    pcap_set_promisc(handle, 1);
    pcap_set_snaplen(handle, 65535);
    pcap_set_timeout(handle, 1);
    pcap_activate(handle);

    while (1)
    {
        libnet_autobuild_arp(                                    // automatyczne budowanie naglowka ARP
            ARPOP_REQUEST,                                       // wyslanie zapytania "Who-has"
            src_hw_addr->ether_addr_octet,                       /* sender hardware addr */
            (u_int8_t *)&src_ip_addr,                            /* sender protocol addr */ // adres IP urzadzenia wysylajacego (nasz)
            zero_hw_addr,                                        /* target hardware addr */
            (u_int8_t *)&target_ip_addr,                         /* target protocol addr */
            ln);                                                 /* libnet context       */
        libnet_autobuild_ethernet(                               // automatyczne budowanie naglowka Ethernet
            bcast_hw_addr,                                       /* ethernet destination */ // okreslenie ze wysylane jest zapytanie ARP (na adres rozgloszeniowy)
            ETHERTYPE_ARP,                                       /* ethertype            */
            ln);                                                 /* libnet context       */
        time_start = clock();
        libnet_write(ln); 
        pcap_loop(handle, -1, response, NULL);
        time_stop = clock();
        time_msec = ((float)(time_stop - time_start) / CLOCKS_PER_SEC) * 1000;
        printf("time = %.2f msec\n", time_msec);
        sleep(1);
    }
    libnet_destroy(ln);
    return EXIT_SUCCESS;
}
