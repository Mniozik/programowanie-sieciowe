/*  -- Turn off/on web interface --
 * Compilation:  gcc -Wall ./zadanie_03_switch.c -o ./switch
 * Usage:        ./switch IFNAME 0/1
 * Tutorial:     0 = Turn off, 1 = Turn on
 * Note:         This program requires root privileges.
 */

#include <arpa/inet.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    // Check input arguments
    if (argc != 3) // argv[0, 1 , 2]
    {
        printf("Incorrect input arguments.\nInput: (interface) (0 or 1)\n");
        return 1;
    }
   
    int sfd;
    struct ifreq ifr;

    sfd = socket(PF_INET, SOCK_DGRAM, 0);
    strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);
    ioctl(sfd, SIOCGIFFLAGS, &ifr); // Get flags

    if (strcmp(argv[2], "1") == 0)
    {
        printf("Turn on: %s\n", argv[1]);
        ifr.ifr_flags |= IFF_UP;
        ioctl(sfd, SIOCSIFFLAGS, &ifr); // Set flags
    }
    else if (strcmp(argv[2], "0") == 0)
    {
        printf("Turn off: %s\n", argv[1]);
        ifr.ifr_flags &= ~IFF_UP;
        ioctl(sfd, SIOCSIFFLAGS, &ifr); // Set flags
    }
    else
    {
        printf("Incorrect input arguments.\nInput: (interface) (0 or 1)\n");
        return 1;
    }
    close(sfd);
    return 0;
}
