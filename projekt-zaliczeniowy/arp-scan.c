/*
Compilation:  gcc -Wall arp-scan.c -o arp-scan -lpcap -lnet
Usage:        ./arp-scan <interface> <server_addr>
NOTE:         This program requires root privileges.
*/

#include <stdio.h>
#include <pcap.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <libnet.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

int response = 0;

void process_packet(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
    struct ether_arp *arp_hdr = (struct ether_arp *)(packet + 14);

    if (ntohs(arp_hdr->arp_op) == ARPOP_REPLY)
    {
        char mac_str[18];
        char ip_str[INET_ADDRSTRLEN];

        snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                 arp_hdr->arp_sha[0], arp_hdr->arp_sha[1], arp_hdr->arp_sha[2],
                 arp_hdr->arp_sha[3], arp_hdr->arp_sha[4], arp_hdr->arp_sha[5]);

        inet_ntop(AF_INET, arp_hdr->arp_spa, ip_str, INET_ADDRSTRLEN);

        int socket_fd2 = *(int *)user;
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "MAC: %s || IP: %s\n", mac_str, ip_str);
        send(socket_fd2, buffer, sizeof(buffer), 0);
    }
}

int main(int argc, char **argv)
{
    printf("|--- ARP-SCAN (Client): <%s> ---|\n", argv[1]);
    if (argc < 2)
    {
        printf("Usage: <interface> <serwer_addr>\n");
        return EXIT_FAILURE;
    }

    // ------ LIBPCAP ------
    struct bpf_program fp;
    char filter_exp[] = "arp[6:2] = 2";
    pcap_t *handle;
    time_t clock;
    bpf_u_int32 netp, maskp;
    char libpcap_errbuf[PCAP_ERRBUF_SIZE];

    if ((handle = pcap_open_live(argv[1], 65535, 0, 2000, libpcap_errbuf)) == NULL)
    {
        fprintf(stderr, "Error - pcap_open_live: %s\n", libpcap_errbuf);
        return 1;
    }
    if ((pcap_setnonblock(handle, 1, libpcap_errbuf)) == -1)
    {
        fprintf(stderr, "Error - pcap_setnonblock: %s\n", libpcap_errbuf);
        return 1;
    }
    if (pcap_lookupnet(argv[1], &netp, &maskp, libpcap_errbuf) == -1)
    {
        fprintf(stderr, "Error - pcap_lookupnet: %s\n", libpcap_errbuf);
        return 1;
    }
    if (pcap_compile(handle, &fp, filter_exp, 0, maskp) == -1)
    {
        fprintf(stderr, "Error - pcap_compile: %s\n", pcap_geterr(handle));
        return 1;
    }
    if (pcap_setfilter(handle, &fp) == -1)
    {
        fprintf(stderr, "Error - pcap_setfilter: %s\n", pcap_geterr(handle));
        return 1;
    }
    pcap_freecode(&fp);

    // ------ SOCKET ------
    int socket_fd;
    struct sockaddr_in server_addr;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        perror("Error: socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    if (inet_pton(AF_INET, argv[2], &server_addr.sin_addr) <= 0)
    {
        perror("Error: inet_pton");
        return 1;
    }

    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error: connect");
        return 1;
    }

    // ------ LIBNET - ARPREQ ------
    libnet_t *ln;
    u_int32_t src_ip_addr;
    u_int8_t bcast_hw_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // adres rozgloszeniowy
        zero_hw_addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};       // adres MAC odbiorcy (poczatkowo same zera)

    struct libnet_ether_addr *src_hw_addr;
    char libnet_errbuf[LIBNET_ERRBUF_SIZE];

    ln = libnet_init(LIBNET_LINK, argv[1], libnet_errbuf);
    src_ip_addr = libnet_get_ipaddr4(ln);
    src_hw_addr = libnet_get_hwaddr(ln);

    for (int i = 1; i <= 254; i++)
    {
        char target_ip_str[16];
        snprintf(target_ip_str, sizeof(target_ip_str), "192.168.100.%d", i); // 192.168.100.X

        u_int32_t target_ip_addr = libnet_name2addr4(ln, target_ip_str, LIBNET_RESOLVE);
        if (target_ip_addr == src_ip_addr)
        {
            printf("Skip - Host address: %s\n", target_ip_str);
            continue;
        }
        // --- Send ---
        libnet_autobuild_arp(
            ARPOP_REQUEST,
            src_hw_addr->ether_addr_octet,
            (u_int8_t *)&src_ip_addr,
            zero_hw_addr,
            (u_int8_t *)&target_ip_addr,
            ln);

        libnet_autobuild_ethernet(
            bcast_hw_addr,
            ETHERTYPE_ARP,
            ln);

        printf("ARP request for: %s\n", target_ip_str);
        libnet_write(ln);
        // --- Catch ---
        response = 1;
        clock = time(NULL);
        while (response)
        {
            pcap_dispatch(handle, -1, process_packet, (u_char *)&socket_fd);

            if ((time(NULL) - clock) > 0.3)
            {
                response = 0;
            }
        }
        libnet_clear_packet(ln);
    }
    libnet_destroy(ln);
    pcap_close(handle);
    close(socket_fd);
    return 0;
}
