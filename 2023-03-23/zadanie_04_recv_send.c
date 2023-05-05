/*
 * Compilation:  gcc -Wall ./zadanie_04_recv_send.c -o ./recv_send
 * Usage:        ./recv_send INTERFACE DST_HW_ADDR
 * NOTE:         This program requires root privileges.
 */

#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ETH_P_CUSTOM 0x8888

int ethsend(char *ifname, char *hwaddr, char *ethdata)
{
    int sfd, ifindex;
    char *frame;
    char *fdata;
    struct ethhdr *fhead;
    struct ifreq ifr;
    struct sockaddr_ll sall;

    sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_CUSTOM));
    frame = malloc(ETH_FRAME_LEN);
    memset(frame, 0, ETH_FRAME_LEN);
    fhead = (struct ethhdr *)frame;
    fdata = frame + ETH_HLEN;
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ioctl(sfd, SIOCGIFINDEX, &ifr);
    ifindex = ifr.ifr_ifindex;
    ioctl(sfd, SIOCGIFHWADDR, &ifr);
    memset(&sall, 0, sizeof(struct sockaddr_ll));
    sall.sll_family = AF_PACKET;
    sall.sll_protocol = htons(ETH_P_CUSTOM);
    sall.sll_ifindex = ifindex;
    sall.sll_hatype = ARPHRD_ETHER;
    sall.sll_pkttype = PACKET_OUTGOING;
    sall.sll_halen = ETH_ALEN;
    sscanf(hwaddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &sall.sll_addr[0], &sall.sll_addr[1], &sall.sll_addr[2],
           &sall.sll_addr[3], &sall.sll_addr[4], &sall.sll_addr[5]);
    memcpy(fhead->h_dest, &sall.sll_addr, ETH_ALEN);
    memcpy(fhead->h_source, &ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    fhead->h_proto = htons(ETH_P_CUSTOM);
    memcpy(fdata, ethdata, strlen(ethdata) + 1);
    sendto(sfd, frame, ETH_HLEN + strlen(ethdata) + 1, 0,
           (struct sockaddr *)&sall, sizeof(struct sockaddr_ll));
    printf("---- FRAME SENT ----\n");
    free(frame);
    close(sfd);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    int sfd;
    ssize_t len;
    char *frame;
    char *fdata;
    struct ethhdr *fhead;
    struct ifreq ifr;
    struct sockaddr_ll sall;

    sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_CUSTOM));
    strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);
    ioctl(sfd, SIOCGIFINDEX, &ifr);
    memset(&sall, 0, sizeof(struct sockaddr_ll));
    sall.sll_family = AF_PACKET;
    sall.sll_protocol = htons(ETH_P_CUSTOM);
    sall.sll_ifindex = ifr.ifr_ifindex;
    sall.sll_hatype = ARPHRD_ETHER;
    sall.sll_pkttype = PACKET_HOST;
    sall.sll_halen = ETH_ALEN;
    bind(sfd, (struct sockaddr *)&sall, sizeof(struct sockaddr_ll));
    while (1)
    {
        frame = malloc(ETH_FRAME_LEN);
        memset(frame, 0, ETH_FRAME_LEN);
        fhead = (struct ethhdr *)frame;
        fdata = frame + ETH_HLEN;
        len = recvfrom(sfd, frame, ETH_FRAME_LEN, 0, NULL, NULL);
        printf("---- FRAME RECEIVED ----\n");
        printf("[%dB] %02x:%02x:%02x:%02x:%02x:%02x -> ", (int)len,
               fhead->h_source[0], fhead->h_source[1], fhead->h_source[2],
               fhead->h_source[3], fhead->h_source[4], fhead->h_source[5]);
        printf("%02x:%02x:%02x:%02x:%02x:%02x | ",
               fhead->h_dest[0], fhead->h_dest[1], fhead->h_dest[2],
               fhead->h_dest[3], fhead->h_dest[4], fhead->h_dest[5]);
        // --- Task 3 ---
        printf("Packet type: %u | ", sall.sll_pkttype);
        printf("EtherType: %04x | ", ntohs(fhead->h_proto));
        // --------------
        printf("Data: %s\n", fdata);
        // --- Task 4 --- 
        ethsend(argv[1], argv[2], fdata);
        // --------------
        sleep(1);
        printf("\n\n");
        free(frame);
    }
    close(sfd);
    return EXIT_SUCCESS;
}
