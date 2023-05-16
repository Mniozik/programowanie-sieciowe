/*
 * Compilation:  gcc -Wall ./zadanie_04_setup.c -o ./setup
 * Usage:        ./setup
 * NOTE:         This program requires root privileges.
 *
 */

#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/route.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ETH_P_CUSTOM 0x8888
#define IRI_T_ADDRESS 0
#define IRI_T_ROUTE 1

struct ifrtinfo
{
    int iri_type;
    char iri_iname[16];
    struct sockaddr_in iri_iaddr; /* IP address */
    struct sockaddr_in iri_rtdst; /* dst. IP address */
    struct sockaddr_in iri_rtmsk; /* dst. netmask */
    struct sockaddr_in iri_rtgip; /* gateway IP */
};

int main(int argc, char **argv)
{
    int sfd;
    ssize_t len;
    char *frame;
    struct ethhdr *fhead;
    struct ifreq ifr;
    struct sockaddr_ll sall;
    struct ifrtinfo *fdata;

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
        len = recvfrom(sfd, frame, ETH_FRAME_LEN, 0, NULL, NULL);
        fdata = (struct ifrtinfo *)(frame + ETH_HLEN);

        if (fdata->iri_type == IRI_T_ADDRESS)
        {
            printf("--- ADDRESS SETUP ---\n");
            int sfd2;
            struct ifreq ifr2;
            struct sockaddr_in *sin;

            sfd2 = socket(PF_INET, SOCK_DGRAM, 0);
            strncpy(ifr2.ifr_name, fdata->iri_iname, strlen(fdata->iri_iname) + 1);

            sin = (struct sockaddr_in *)&ifr2.ifr_addr;
            memset(sin, 0, sizeof(struct sockaddr_in));
            sin->sin_family = AF_INET;
            sin->sin_port = 0;
            sin->sin_addr.s_addr = fdata->iri_iaddr.sin_addr.s_addr;

            ioctl(sfd2, SIOCSIFADDR, &ifr2);
            ioctl(sfd2, SIOCGIFFLAGS, &ifr2);
            ifr2.ifr_flags |= IFF_UP | IFF_RUNNING;
            ioctl(sfd2, SIOCSIFFLAGS, &ifr2);
            close(sfd2);
        }
        else if (fdata->iri_type == IRI_T_ROUTE)
        {
            printf("--- ROUTE SETUP ---\n");
            int sfd2;
            struct rtentry route;
            struct sockaddr_in *addr;

            sfd2 = socket(PF_INET, SOCK_DGRAM, 0);
            memset(&route, 0, sizeof(route));

            addr = (struct sockaddr_in *)&route.rt_gateway;
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = fdata->iri_rtgip.sin_addr.s_addr;

            addr = (struct sockaddr_in *)&route.rt_dst;
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = fdata->iri_rtdst.sin_addr.s_addr;

            addr = (struct sockaddr_in *)&route.rt_genmask;
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = fdata->iri_rtmsk.sin_addr.s_addr;
            route.rt_flags = RTF_UP | RTF_GATEWAY;
            route.rt_metric = 0;
            ioctl(sfd2, SIOCADDRT, &route);
            close(sfd2);
        }
        printf("\n");
        free(frame);
    }
    close(sfd);
    return EXIT_SUCCESS;
}
