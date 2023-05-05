/* -- Setup IP and SUBNET_MASK --
 * Compilation:  gcc -Wall ./zadanie_05_ifsetup.c -o ./ifsetup
 * Usage:        ./ifsetup IFNAME IP SUBNET_MASK
 * Note:         This program requires root privileges.
 */

#include <arpa/inet.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    // Check input arguments
    if (argc != 4) // argv[0, 1 , 2, 3]
    {
        printf("Incorrect input arguments.\nInput: (interface) (IP address) (subnet mask)\n");
        return 1;
    }

    int sfd;
    struct ifreq ifr;
    struct sockaddr_in *sin;

    sfd = socket(PF_INET, SOCK_DGRAM, 0);
    strncpy(ifr.ifr_name, argv[1], strlen(argv[1]) + 1);

    // IP address
    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    memset(sin, 0, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    // Setup IP address
    sin->sin_addr.s_addr = inet_addr(argv[2]);
    ioctl(sfd, SIOCSIFADDR, &ifr);

    // Subnet mask
    sin = (struct sockaddr_in *)&ifr.ifr_netmask;
    memset(sin, 0, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    // Setup subnet mask
    sin->sin_addr.s_addr = inet_addr(argv[3]);
    ioctl(sfd, SIOCSIFNETMASK, &ifr);

    ioctl(sfd, SIOCGIFFLAGS, &ifr);
    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
    ioctl(sfd, SIOCSIFFLAGS, &ifr);

    printf("Setup IP and subnet mask is done.\n");
    close(sfd);
    return EXIT_SUCCESS;
}
